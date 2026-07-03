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

#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"

#include <etl/alignment.h>

#include "async/Config.h"
#include "interrupt_manager.h"
#endif

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
 * application interrupts.
 */
void setupApplicationsIsr(void)
{
#ifdef PLATFORM_SUPPORT_CAN
    /* FlexCAN0 message buffers 0-31 (RX and TX share this line on the
     * S32K344). The bus off line (FlexCAN0_0_IRQn) stays disabled, bus off
     * is polled; message buffers 32-95 are not used. */
    SYS_SetPriority(FlexCAN0_1_IRQn, 8);

    SYS_EnableIRQ(FlexCAN0_1_IRQn);

    ENABLE_INTERRUPTS();
#endif
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

/**
 * Callout from main application to give platform the chance to add a
 * ::lifecycle::ILifecycleComponent to the \p lifecycleManager at a given \p level.
 */
void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 2U)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN, staticBsp), level);
#endif // PLATFORM_SUPPORT_CAN
    }
    (void)lifecycleManager;
    (void)level;
}
} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

int main()
{
    ::safety::safeSupervisorConstructor.construct();
    ::platform::staticBsp.init();
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
