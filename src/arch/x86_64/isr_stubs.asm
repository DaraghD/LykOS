extern isr_handler_table ; function ptrs

%macro ISR_STUB 1
global isr_stub_%1
isr_stub_%1:
    push qword 0            ; dummy error code (for vectors that don't push one)
    push qword %1           ; push the vector number
    jmp  isr_common
%endmacro

; For vectors where the CPU pushes an error code already (e.g. GPF #13, page fault #14)
%macro ISR_STUB_ERR 1
global isr_stub_%1
isr_stub_%1:
    ; CPU already pushed error code
    push qword %1           ; push vector number
    jmp  isr_common
%endmacro

isr_common:
    ; Save all general-purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; First arg (rdi) = ptr to this stack frame
    mov  rdi, rsp

    ; Second arg (rsi) = vector number (sitting in the stack)
    mov  rsi, [rsp + 120]   ; 15 regs * 8 bytes = 120 offset to vector number

    ; Look up handler in C table and call it
    lea  rax, [rel isr_handler_table]
    mov  rax, [rax + rsi * 8]
    test rax, rax
    jz   .no_handler
    call rax

.no_handler:
    ; Restore all registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16             ; pop vector number + error code
    iretq

ISR_STUB_ERR 13 ; GPF (CPU pushes error code)
ISR_STUB_ERR 14 ; page fault
ISR_STUB     0  ; divide error
ISR_STUB     32 ; PIT timer (0x20)
ISR_STUB     33 ; keyboard (0x21)
ISR_STUB     60 ; your test syscall
ISR_STUB     128    ; syscall 0x80
