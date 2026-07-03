/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "can/FlexCANDevice.h"

/* CAN0 of the S32K3X4EVB-T172:
 * - PTA7 = CAN0_TX: MSCR[7], source signal select 4
 * - PTA6 = CAN0_RX: MSCR[6] (input), routed to FLEXCAN0 via IMCR[0]
 *   (IMCR #512), source select 2
 * The pads are wired to a TJA1153 secure CAN transceiver, whose EN/STB
 * control pins are driven by the board CAN phy (see StaticBsp).
 *
 * Bit timing for the 40 MHz FLEXCAN0 protocol engine clock (MC_CGM MUX_3,
 * see bspClock), 500 kbit/s:
 * PRESDIV = 4 -> fTQ = 40 MHz / 5 = 8 MHz
 * RJW = 0, PSEG1 = 4, PSEG2 = 2, PROPSEG = 6
 * -> Bittime = 16 TQ, sample point 81.25 % (same bus settings as on the
 *    S32K148EVB) */
bios::FlexCANDevice::Config const Can0Config
    = {IP_CAN_0_BASE,
       can::ICanTransceiver::BAUDRATE_HIGHSPEED,
       // PRESDIV                PSEG1        PSEG2        PROPSEG
       (4U << 24) | (0U << 22) | (4U << 19) | (2U << 16) | (6U << 0),
       7,  // txPadMscr (PTA7)
       4,  // txPadSss
       6,  // rxPadMscr (PTA6)
       0,  // rxImcr
       2,  // rxImcrSss
       14, // numRxBufsStd
       2,  // numRxBufsExt
       15, // numTxBufsApp
       0,  // BusId
       0}; // wakeUp
