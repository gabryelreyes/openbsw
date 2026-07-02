/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <bsp/Uart.h>

#include "mcu/mcu.h"

namespace bsp
{
void uartConfigureTerminalPins()
{
    // PTA16 = LPUART6_TX: source signal select ALT5, output buffer enable
    IP_SIUL2->MSCR[16] = SIUL2_MSCR_OBE_MASK | 5U;
    // PTA15 = LPUART6_RX: input buffer enable ...
    IP_SIUL2->MSCR[15] = SIUL2_MSCR_IBE_MASK;
    // ... and route pad PTA15 to the LPUART6 receiver (IMCR #705 = index 193,
    // source select 2; see the S32K344 IO signal description table)
    IP_SIUL2->IMCR[193] = 2U;
}
} // namespace bsp
