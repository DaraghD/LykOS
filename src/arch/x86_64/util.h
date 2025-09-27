#pragma once
#include "arch/x86_64/io.h"

static inline void qemu_shutdown(void){
    outw(0x604, 0x2000);
}
