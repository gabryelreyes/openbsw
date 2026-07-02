# FreeRTOS Custom Port Modification

This repository contains a custom implementation of FreeRTOS for the S32K3xx platform (Cortex-M7). It is derived from the S32K1xx ARM CM4F-based port in `platforms/s32k1xx/3rdparty/freertos_cm4_sysTick`; the ARMv7E-M port code is architecture-compatible with the Cortex-M7 (r1p2) of the S32K344, which is not affected by the r0p0/r0p1 erratum 837070 that the upstream ARM_CM7 port works around. One of the customizations involves the addition of an application-specific interrupt service routine (ISR) setup function, `setupApplicationsIsr`.

## Custom Function: `setupApplicationsIsr`

Please refer to ``ManualChanges.diff`` for the making manual modification.
Always remember to reapply this modification whenever you update the FreeRTOS source code in your project.