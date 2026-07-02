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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Configures the S32K344 clock tree for full speed operation:
 * FXOSC (16 MHz crystal) -> PLL -> PHI0 160 MHz
 *   CORE_CLK      160 MHz
 *   AIPS_PLAT_CLK  80 MHz
 *   AIPS_SLOW_CLK  40 MHz
 *   HSE_CLK        80 MHz
 * Also ungates the peripheral clocks used by the BSP (LPUART6).
 *
 * Must be callable before the C/C++ runtime is initialized (no globals).
 */
void configurePll(void);

#ifdef __cplusplus
} // extern "C"
#endif
