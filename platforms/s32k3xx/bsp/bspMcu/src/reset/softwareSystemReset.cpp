/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include "mcu/mcu.h"

namespace
{
uint32_t const MC_ME_KEY          = 0x5AF0U;
uint32_t const MC_ME_INVERTED_KEY = 0xA50FU;

/**
 * Requests a reset through the mode entry module. On the S32K3xx the
 * Cortex-M7 SYSRESETREQ (NVIC_SystemReset) is not a reset source of the
 * MC_RGM, so software resets have to go through MC_ME.
 */
[[noreturn]] void mcMeReset(uint32_t const modeConf)
{
    __DSB();
    IP_MC_ME->MODE_CONF = modeConf;
    IP_MC_ME->MODE_UPD  = MC_ME_MODE_UPD_MODE_UPD_MASK;
    IP_MC_ME->CTL_KEY   = MC_ME_KEY;
    IP_MC_ME->CTL_KEY   = MC_ME_INVERTED_KEY;
    for (;;)
    {
        __WFI();
    }
}
} // namespace

extern "C"
{
[[noreturn]] void softwareSystemReset(void) { mcMeReset(MC_ME_MODE_CONF_FUNC_RST_MASK); }

void softwareDestructiveReset(void) { mcMeReset(MC_ME_MODE_CONF_DEST_RST_MASK); }
}
