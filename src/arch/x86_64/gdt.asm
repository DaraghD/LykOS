global load_gdt
load_gdt:
    lgdt [rdi]          ; rdi = &gdt_ptr

    ; reload data segments
    mov ax, 0x10        ; GDT_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; reload code segment with far jump
    push 0x08           ; GDT_CODE_SELECTOR
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ret
