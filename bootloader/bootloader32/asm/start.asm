[bits 16]

SECTION .intro
    EXTERN bootloader32_main    ; C code entry point
    EXTERN make_memory_map

    intro:
        ; Setup bootloader_parameters
        mov [boot_info__boot_drive_id], dl
        mov [boot_info__boot_partition], dh

        ; Setup video mode
        mov ah, 0x00
        mov al, 0x03
        int 0x10

        ; Setup protected mode
        lgdt [gdt_description]

        ; Disable interrupts and NMI
        cli
        in al, 0x70
        or al, 0x80
        out 0x70, al

        ; Enter protected mode
        mov eax, cr0
        or eax, 1
        mov cr0, eax

        jmp 0x08:.protected_mode

        .protected_mode:
            [bits 32]
            mov ax, 0x10
            mov ds, ax
            mov ss, ax
            mov es, ax
            mov fs, ax
            mov gs, ax

            mov esp, 0x00080000
            mov ebp, esp

            ; Jump to C code
            jmp bootloader32_main

SECTION .real.data
    ; Macro for GDT entries
    %macro gdt_entry 4
        ; %1 - base address
        ; %2 - limit
        ; %3 - access
        ; %4 - flags

        dw (%2 & 0xFFFF)
        dw (%1 & 0xFFFF)
        db ((%1 >> 16) & 0xFF)
        db (%3 & 0xFF)
        db ((%2 >> 16) & 0x0F | (%4 << 4))
        db ((%1 >> 24) & 0xFF)
    %endmacro

    gdt_description:
        dw (gdt_end - gdt_start - 1)                               ; size
        dd gdt_start                                               ; offset

    gdt_start:
        gdt_entry 0x00000000, 0x00000000, 0b00000000, 0b0000       ; null segment
        gdt_entry 0x00000000, 0x0FFFFFFF, 0b10011010, 0b1100       ; 32-bit code segment
        gdt_entry 0x00000000, 0x0FFFFFFF, 0b10010010, 0b1100       ; 32-bit data segment
        gdt_entry 0x00000000, 0x0FFFFFFF, 0b10011010, 0b1000       ; 16-bit code segment
        gdt_entry 0x00000000, 0x0FFFFFFF, 0b10010010, 0b1000       ; 16-bit data segment
    gdt_end:

    ; Parameters to be passed to the bootloader
    GLOBAL g_bootInfo
    g_bootInfo:
        boot_info__boot_drive_id:        db 0            ; Boot drive ID
        boot_info__boot_partition:       db 0            ; Boot partition
