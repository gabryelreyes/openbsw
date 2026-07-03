
/*****************************************************************************/
/* Startup code for the S32K344 (Cortex-M7)                                  */
/* Version: GCC for ARM Embedded Processors                                  */
/*****************************************************************************/
    .syntax unified
    .arch armv7e-m

/* Vector table: 16 system exceptions + 213 external interrupts (see
 * NUMBER_OF_INT_VECTORS in S32K344_COMMON.h).
 * TODO (Phase 2, doc/dev/s32k344_integration_plan.md): name the peripheral
 * interrupt handlers as they get implemented (LPUART, ...), and route
 * the fault exceptions to the hardFaultHandler module. */
    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                                      /* Top of Stack */
    .long   Reset_Handler                                   /* Reset Handler */
    .long   DefaultISR                                      /* NMI Handler */
    .long   DefaultISR                                      /* Hard Fault Handler */
    .long   DefaultISR                                      /* MPU Fault Handler */
    .long   DefaultISR                                      /* Bus Fault Handler */
    .long   DefaultISR                                      /* Usage Fault Handler */
    .long   0                                               /* Reserved */
    .long   0                                               /* Reserved */
    .long   0                                               /* Reserved */
    .long   0                                               /* Reserved */
    .long   SVC_Handler                                     /* SVCall Handler */
    .long   DefaultISR                                      /* Debug Monitor Handler */
    .long   0                                               /* Reserved */
    .long   PendSV_Handler                                  /* PendSV Handler */
    .long   SysTick_Handler                                 /* SysTick Handler */

                                                            /* External Interrupts */
    .rept   109                                             /* IRQ 0..108 */
    .long   DefaultISR
    .endr
    .long   CAN0_Bus_Off_Error_IRQHandler                   /* IRQ 109: FlexCAN0 bus off / errors */
    .long   CAN0_ORed_0_31_MB_IRQHandler                    /* IRQ 110: FlexCAN0 message buffers 0-31 */
    .long   CAN0_ORed_32_63_MB_IRQHandler                   /* IRQ 111: FlexCAN0 message buffers 32-63 */
    .long   CAN0_ORed_64_95_MB_IRQHandler                   /* IRQ 112: FlexCAN0 message buffers 64-95 */
    .rept   100                                             /* IRQ 113..212 */
    .long   DefaultISR
    .endr

    .size    __isr_vector, . - __isr_vector

    .section .text
    .equ _NVIC_ICER,  0xE000E180
    .equ _NVIC_ICPR,  0xE000E280
    .equ _NVIC_VTOR,  0xE000ED08
    .equ _CPACR,      0xE000ED88
    .text
    .thumb

/* Reset Handler */

    .thumb_func
    .align 2
    .globl   Reset_Handler
    .type    Reset_Handler, %function

Reset_Handler:

    cpsid   i               /* Mask interrupts */

    /* Disable interrupts and clear pending flags (8 registers for up to
     * 256 interrupts) */
    ldr r0, =_NVIC_ICER
    ldr r1, =_NVIC_ICPR
    ldr r2, =0xFFFFFFFF
    movs r3, #8
.LC_NVIC:
    str r2, [r0], #4        /* NVIC_ICER - clear enable IRQ register */
    str r2, [r1], #4        /* NVIC_ICPR - clear pending IRQ register */
    subs r3, #1
    bne .LC_NVIC

    /* Enable FPU (CP10/CP11 full access) */
    ldr r0, =_CPACR
    ldr r1, [r0]
    orr r1, r1, #0x00F00000
    str r1, [r0]
    dsb
    isb

    /* Init ECC RAM.
     *
     * The S32K344 SRAM and TCMs are ECC protected and must be initialized
     * with aligned 64-bit writes before the first read access - including
     * the stack, so this loop must run before any stack usage.
     * TODO (Phase 2, doc/dev/s32k344_integration_plan.md): cover standby RAM
     * and ITCM when they get used. */
    movs r2, #0
    movs r3, #0

    ldr r0, =__ECC_DTCM_START
    ldr r1, =__ECC_DTCM_END
.LC_ECC_DTCM:
    strd r2, r3, [r0], #8
    cmp r0, r1
    blt .LC_ECC_DTCM

    ldr r0, =__ECC_RAM_START
    ldr r1, =__ECC_RAM_END
.LC_ECC_RAM:
    strd r2, r3, [r0], #8
    cmp r0, r1
    blt .LC_ECC_RAM

    /* Set the vector table base address. The application does not start at
     * the default VTOR location. */
    ldr r0, =_NVIC_VTOR
    ldr r1, =__isr_vector
    str r1, [r0]
    dsb

    /* Init the rest of the registers */
    ldr     r1,=0
    ldr     r2,=0
    ldr     r3,=0
    ldr     r4,=0
    ldr     r5,=0
    ldr     r6,=0
    ldr     r7,=0
    mov     r8,r7
    mov     r9,r7
    mov     r10,r7
    mov     r11,r7
    mov     r12,r7

    /* Initialize the stack pointer */
    ldr     r0,=__StackTop
    mov     r13,r0

    bl boardInit

/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs    r3, r2
    ble     .LC0

.LC1:
    subs    r3, 4
    ldr    r0, [r1,r3]
    str    r0, [r2,r3]
    bgt    .LC1
.LC0:

/*     Loop to zero out BSS section, which uses following symbols
 *     in linker script:
 *      __bss_start__: start of BSS section. Must align to 4
 *      __bss_end__: end of BSS section. Must align to 4
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    subs    r2, r1
    ble .LC3

    movs    r0, 0
.LC2:
    str r0, [r1, r2]
    subs    r2, 4
    bge .LC2
.LC3:

    cpsie   i               /* Unmask interrupts */

    bl    __libc_init_array

    bl    main
    .pool
    .size Reset_Handler, . - Reset_Handler


    .align  1
    .thumb_func
    .weak DefaultISR
    .type DefaultISR, %function
DefaultISR:
    b       DefaultISR
    .size DefaultISR, . - DefaultISR

.end
