// https://wiki.osdev.org/Creating_a_C_Library#Program_Initialization
//
#include "lykosapi.h"

extern int main(int argc, char **argv);

void _start(void) {
  // TODO:need kernel to push argc/argv stuff
  //
  main(0, (char **)0);
  lykos_exit();
}
