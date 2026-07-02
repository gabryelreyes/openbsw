/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * System timer based on the Cortex-M7 DWT cycle counter, running on the
 * 160 MHz core clock configured by bspClock.
 * \file
 * \ingroup bspConfig
 */
#include "bsp/timer/SystemTimer.h"

#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

namespace
{
uint32_t const DWT_FREQ_MHZ_RUN = 160U; // PLL core clock (see bspClock)
uint32_t const TICK_FREQ_MHZ    = 16U; // Common divider of all DWT frequencies

struct
{
    uint64_t ticks; // Main ticks counter, never overflows
    uint32_t lastDwt;
    uint32_t dwtTicksRatio;
    uint32_t dwtFreqMhz;
} state = {0, 0, DWT_FREQ_MHZ_RUN / TICK_FREQ_MHZ, DWT_FREQ_MHZ_RUN};

// DWT registers (refer to ARMv7-M architecture reference manual)
uint32_t volatile& DWT_CTRL   = *reinterpret_cast<uint32_t volatile*>(0xE0001000U);
uint32_t volatile& DWT_CYCCNT = *reinterpret_cast<uint32_t volatile*>(0xE0001004U);
uint32_t volatile& DWT_LAR    = *reinterpret_cast<uint32_t volatile*>(0xE0001FB0U);
uint32_t volatile& DEMCR      = *reinterpret_cast<uint32_t volatile*>(0xE000EDFCU);

// Must be called more than once per DWT period (~26 sec for 160 MHz frequency). Called implicitly
// through all getSomething() functions
uint64_t updateTicks()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
    uint32_t const curDwt = DWT_CYCCNT;
    state.ticks += static_cast<uint32_t>((curDwt - state.lastDwt) / state.dwtTicksRatio);
    state.lastDwt = curDwt;
    return state.ticks;
}

} // namespace

extern "C"
{
void initSystemTimer()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;

    DEMCR = DEMCR | 0x01000000U;
    // The Cortex-M7 DWT is protected by a lock access register.
    DWT_LAR    = 0xC5ACCE55U;
    DWT_CYCCNT = 0;
    DWT_CTRL   = DWT_CTRL | 0x00000001U;

    state.ticks         = 0; // General ticks counter, never overflows
    state.lastDwt       = 0;
    state.dwtTicksRatio = DWT_FREQ_MHZ_RUN / TICK_FREQ_MHZ;
    state.dwtFreqMhz    = DWT_FREQ_MHZ_RUN;
}

uint64_t getSystemTicks(void) { return updateTicks(); }

uint32_t getSystemTicks32Bit(void) { return static_cast<uint32_t>(updateTicks()); }

uint64_t getSystemTimeNs(void) { return updateTicks() * 1000U / TICK_FREQ_MHZ; }

uint64_t getSystemTimeUs(void) { return updateTicks() / TICK_FREQ_MHZ; }

uint32_t getSystemTimeUs32Bit(void) { return static_cast<uint32_t>(updateTicks() / TICK_FREQ_MHZ); }

uint64_t getSystemTimeMs(void) { return updateTicks() / TICK_FREQ_MHZ / 1000U; }

uint32_t getSystemTimeMs32Bit(void)
{
    return static_cast<uint32_t>(updateTicks() / TICK_FREQ_MHZ / 1000U);
}

uint64_t systemTicksToTimeUs(uint64_t const ticks) { return ticks / TICK_FREQ_MHZ; }

uint64_t systemTicksToTimeNs(uint64_t const ticks) { return ticks * 1000U / TICK_FREQ_MHZ; }

uint32_t getFastTicks(void) { return DWT_CYCCNT; }

uint32_t getFastTicksPerSecond(void)
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
    return state.dwtFreqMhz * 1000000;
}

void sysDelayUs(uint32_t const delay)
{
    uint64_t const start = getSystemTimeUs();
    while (getSystemTimeUs() < start + delay) {}
}

} // extern "C"
