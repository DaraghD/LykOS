#include "task.h"
#include "drivers/pit.h"
#include "drivers/serial.h"
#include "klib/kstring.h"
#include "klib/time.h"
#include "mem/kalloc.h"
#include "terminal.h"
#include "user/elf.h"

Task tasks[MAX_TASKS];
int current_task = -1;
int task_count = 0;
volatile int need_resched = 0;

extern void switch_task(Registers *from, Registers *to); // in switch_task.asm
__attribute__((noreturn)) void task_exit(void);

static int find_free_task_slot(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == TASK_UNUSED || tasks[i].state == TASK_DEAD) {
      return i;
    }
  }
  return -1;
}

int task_create(const char *name, void (*entry)(void)) {
  int i = find_free_task_slot();
  if (i < 0) {
    serial_write_fstring("[TASK] No free task slots\n");
    return -1;
  }

  Task *t = &tasks[i];

  t->stack = kalloc(TASK_STACK_SIZE);
  if (!t->stack) {
    serial_write_fstring("[TASK] Failed to allocate stack\n");
    return -1;
  }

  u64 stack_top = ((u64)t->stack + TASK_STACK_SIZE) & ~0xFULL;

  // fake return address so if entry() returns, it goes to task_exit
  stack_top -= 8;
  *(u64 *)stack_top = (u64)task_exit;

  t->regs.r15 = 0;
  t->regs.r14 = 0;
  t->regs.r13 = 0;
  t->regs.r12 = 0;
  t->regs.rbx = 0;
  t->regs.rbp = 0;
  t->regs.rip = (u64)entry;
  t->regs.rsp = stack_top;

  t->state = TASK_READY;
  t->slice = TASK_SLICE_DEFAULT;
  t->name = name;
  t->start_tick = pit_get_ticks(); // used to calculate uptime

  if (i >= task_count)
    task_count = i + 1;

  serial_write_fstring("[TASK] Created: {str}\n", name);
  return i;
}

int task_create_elf(const char *name, void *file) {
  int id = find_free_task_slot();
  if (id < 0)
    return -1;

  loaded_elf elf;
  if (!load_elf(file, &elf))
    return -1;

  Task *t = &tasks[id];

  t->stack = kalloc(TASK_STACK_SIZE);
  if (!t->stack)
    return -1;

  u64 sp = ((u64)t->stack + TASK_STACK_SIZE) & ~0xFULL;

  // fake return address -> task_exit
  sp -= 8;
  *(u64 *)sp = (u64)task_exit;

  memset(&t->regs, 0, sizeof(t->regs));
  t->regs.rip = elf.entry_addr;
  t->regs.rsp = sp;

  t->state = TASK_READY;
  t->slice = TASK_SLICE_DEFAULT;
  t->name = name;
  t->wakeup_tick = 0;
  t->start_tick = pit_get_ticks();

  t->segment_count = elf.segment_count;
  for (u64 i = 0; i < elf.segment_count; i++) {
    t->segments[i] = elf.segments[i];
  }
  t->is_elf = true;

  if (id >= task_count)
    task_count = id + 1;

  serial_write_fstring("[TASK] created ELF task {str} entry=0x{hex}\n", name,
                       elf.entry_addr);
  terminal_fstring("Created ELF Task\n");

  return id;
}

static int find_next_ready_task(void) {
  if (task_count == 0)
    return -1;

  for (int offset = 1; offset <= task_count; offset++) {
    int i = (current_task + offset) % task_count;
    if (tasks[i].state == TASK_READY) {
      return i;
    }
  }

  return -1;
}

void yield(void) {
  asm volatile("cli"); // we dont wanna get pre-empted during a yield

  if (current_task < 0) {
    goto end;
  }

  int next = find_next_ready_task();
  if (next < 0 || next == current_task) {
    goto end;
  }

  int prev = current_task;

  if (tasks[prev].state == TASK_RUNNING)
    tasks[prev].state = TASK_READY;

  tasks[next].state = TASK_RUNNING;
  tasks[next].slice = TASK_SLICE_DEFAULT;
  current_task = next;

  serial_write_fstring("Switching to {str} \n", tasks[next].name);
  switch_task(&tasks[prev].regs, &tasks[next].regs);

end:
  asm volatile("sti");
}

__attribute__((noreturn)) void task_exit(void) {
  asm volatile("cli");

  Task *t = &tasks[current_task];
  serial_write_fstring("[TASK] Exiting: {str}\n", t->name);
  if (current_task >= 0) {
    t->state = TASK_DEAD;
    if (t->is_elf) {
      for (u64 i = 0; i < t->segment_count; i++) {
        kfree(t->segments[i]);
      }
    }
  }
  kfree(t->stack);

  asm volatile("sti");

  yield();

  for (;;)
    asm volatile("hlt");
}

void tasking_init(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    tasks[i].state = TASK_UNUSED;
    tasks[i].stack = 0;
    tasks[i].name = 0;
  }

  // bootstrap current kernel execution as task 0
  tasks[0].state = TASK_RUNNING;
  tasks[0].slice = TASK_SLICE_DEFAULT;
  tasks[0].name = "kernel-main";
  current_task = 0;
  task_count = 1;

  serial_write_fstring("[TASK] Tasking initialised\n");
}

void preempt_check(void) {
  if (current_task < 0)
    return;

  if (tasks[current_task].slice == 0) {
    yield();
  } else {
    tasks[current_task].slice--;
  }
}

void debug_tasks(void) {
  // add in a header of the info like name : id : etc
  // need to figure out justifications
  terminal_fstring("===KERNEL TASKS===\n");
  terminal_fstring("CURRENT TICK : {uint}\n", pit_get_ticks());

  int longest_task_name = 0;
  for (int i = 0; i < task_count; i++) {
    Task *t = &tasks[i];
    if (longest_task_name < strlen(t->name))
      longest_task_name = strlen(t->name);
  }

  for (int task_id = 0; task_id < MAX_TASKS; task_id++) {
    Task *t = &tasks[task_id];
    if (t->state == TASK_UNUSED)
      continue;

    u16 name_diff = longest_task_name - strlen(t->name);
    terminal_fstring("{uint} : ", task_id);
    terminal_fstring("{str} ", t->name);
    for (int i = 0; i < name_diff; i++)
      terminal_fstring(" ");
    terminal_fstring(": ");

    char buf[9];
    ticks_to_time(t->start_tick, buf);
    terminal_fstring("{str} : ", buf);

    terminal_fstring("{str} : SLICE : {uint}\n", task_state_to_str(t->state),
                     t->slice);
  }
}

void kill_task(u16 task_id) {
  Task *t = &tasks[task_id];
  serial_write_fstring("[Tasks] Killing task {uint} called {str}", task_id,
                       t->name);
  t->state = TASK_DEAD;
}

const char *task_state_names[] = {
#define X(name) #name,
    TASK_STATE_LIST
#undef X
};
const char *task_state_to_str(enum task_state s) { return task_state_names[s]; }

void task_sleep(u64 ms) {
  asm volatile("cli");

  Task *t = &tasks[current_task];

  t->wakeup_tick = pit_get_ticks() + ms_to_ticks(ms);
  t->state = TASK_BLOCKED;

  asm volatile("sti");

  yield();
}

void wake_sleeping_tasks(void) {
  for (int task_id = 0; task_id < task_count; task_id++) {
    Task *t = &tasks[task_id];
    if (t->state == TASK_BLOCKED) {
      if (pit_get_ticks() >= t->wakeup_tick) {
        t->state = TASK_READY;
      }
    }
  }
}
