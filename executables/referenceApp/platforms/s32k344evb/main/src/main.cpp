/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/StaticBsp.h"

#include "clock/clockConfig.h"
#include "mcu/mcu.h"

#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>

extern void app_main();

extern "C"
{
/**
 * Called from the startup code before the C/C++ runtime is initialized:
 * no globals may be touched here.
 */
void boardInit()
{
    /* Disable the SWT0 watchdog, which is enabled out of reset when booting
     * through the debugger. A safety watchdog concept follows with the
     * safety extension (see doc/dev/s32k344_integration_plan.md). */
    if ((IP_SWT_0->CR & SWT_CR_SLK_MASK) != 0U)
    {
        IP_SWT_0->SR = 0xC520U;
        IP_SWT_0->SR = 0xD928U;
    }
    IP_SWT_0->CR = IP_SWT_0->CR & ~SWT_CR_WEN_MASK;

    configurePll();

    SCB_EnableICache();
    SCB_EnableDCache();
}

/**
 * Callout from the FreeRTOS port when the scheduler starts to set up
 * application interrupts. Nothing to set up yet - CAN interrupts are
 * configured here from Phase 3 on.
 */
void setupApplicationsIsr(void) {}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

/**
 * Callout from main application to give platform the chance to add a
 * ::lifecycle::ILifecycleComponent to the \p lifecycleManager at a given \p level.
 */
void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    (void)lifecycleManager;
    (void)level;
}
} // namespace platform

int main()
{
    ::safety::safeSupervisorConstructor.construct();
    ::platform::staticBsp.init();
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
