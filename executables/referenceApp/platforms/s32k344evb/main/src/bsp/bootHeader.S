
/*****************************************************************************/
/* Image Vector Table (IVT) for the S32K344 Boot Assist Firmware (BAF)       */
/*                                                                           */
/* The BAF searches for this 256 byte structure at the program flash base.   */
/* It ungates the Cortex-M7_0 core and starts it at the given address, which */
/* points to the application vector table (initial SP and Reset_Handler).    */
/* See S32K3xx Reference Manual, chapter "Boot".                             */
/*****************************************************************************/
    .section .bootHeader, "a"
    .align 2
    .globl __bootHeader
__bootHeader:
    .long   0x5AA55AA5          /* IVT marker */
    .long   0x00000001          /* boot configuration: non-secure boot,
                                 * SWT0 disabled, ungate Cortex-M7_0 */
    .long   0                   /* reserved */
    .long   __isr_vector        /* Cortex-M7_0 start address */
    .long   0                   /* reserved */
    .long   0                   /* Cortex-M7_1 start address (unused) */
    .long   0                   /* reserved */
    .long   0                   /* Cortex-M7_2 start address (unused) */
    .long   0                   /* reserved */
    .long   0                   /* lifecycle configuration pointer (unused) */
    .space  216, 0x00           /* reserved */

    .size __bootHeader, . - __bootHeader
.end
