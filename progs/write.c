#include "lykosapi.h"

void main(void) {
  char *my_str = "hello syscall world!2222222\n";
  write(my_str);
  write("Sleeping for 3 seconds Zzzz...\n");
  sleep(3000);
  write("Im awake! bye...\n");
}
