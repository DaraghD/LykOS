#include "arch/x86_64/io.h"

static inline void qemu_shutdown(){
    outw(0x604, 0x2000);
}
