#include "exec.h"
#include "drivers/serial.h"
#include "fs/vfs.h"
#include "klib/kstring.h"
#include "proc/task.h"

i64 exec(char *file_name) {
  serial_fstring("Exec9 called with {str} \n", file_name);
  u64 size = 0;
  kstring ks_file = KSTRING(file_name, strlen(file_name));
  ks_file.len = strlen(file_name);

  void *file = read_file(&ks_file, &size);
  if (file == NULL) {
    serial_writeln("Can't find file to exec");
    return -1;
  }

  int ret = task_create_elf(file_name, file);
  if (ret < 0) {
    serial_fstring("Could not exec file\n");
    return -2;
  }

  return ret;
}
