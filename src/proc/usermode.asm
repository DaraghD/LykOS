global enter_usermode
; rdi = rip, rsi = rsp, rdx = cr3
enter_usermode:
    mov cr3, rdx
    push 0x23          ; SS
    push rsi           ; RSP
    push 0x202         ; RFLAGS (interrupts enabled)
    push 0x1B          ; CS
    push rdi           ; RIP
    iretq
