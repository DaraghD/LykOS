#include "lykosapi.h"

void main(void) {
  exec("write.elf");
  // exits before its "child" is complete
  // should maybe have a join() / wait() syscall
}
