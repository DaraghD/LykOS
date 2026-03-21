void main(void) {
  asm volatile(
      "cli"); // illegal instruction in usermode, tests ring 3 protection
}
