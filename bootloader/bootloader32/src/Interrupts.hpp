#ifndef FUNNYOS_BOOTLOADER_BOOTLOADER32_SRC_INTERRUPTS_HPP
#define FUNNYOS_BOOTLOADER_BOOTLOADER32_SRC_INTERRUPTS_HPP

namespace FunnyOS::Bootloader32 {

    /**
     * Setups all the interrupt handlers, loads IDT, reprograms PIC and enables all interrupts.
     */
    void SetupInterrupts();

}

#endif  // FUNNYOS_BOOTLOADER_BOOTLOADER32_SRC_INTERRUPTS_HPP
