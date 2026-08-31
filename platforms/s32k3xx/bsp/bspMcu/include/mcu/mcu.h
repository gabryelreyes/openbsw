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

#include "3rdparty/nxp/S32K344.h"

// Headers that are not pulled in by S32K344.h but are needed by the BSP.
#include "3rdparty/nxp/S32K344_MC_ME.h"
#include "3rdparty/nxp/S32K344_MC_RGM.h"
#include "3rdparty/nxp/S32K344_PMC.h"
#include "3rdparty/nxp/S32K344_SIUL2.h"
#include "3rdparty/nxp/S32K344_STM.h"
#include "3rdparty/nxp/S32K344_SWT.h"
#include "3rdparty/nxp/S32K344_WKPU.h"

// Cortex-M7 r1p2 (see S32K3xx Reference Manual, chapter "Cores"). The NXP
// device header does not define the core revision required by CMSIS.
#ifndef __CM7_REV
#define __CM7_REV 0x0102U
#endif

// The NXP header provides its own core peripheral definitions with an S32_
// prefix (S32_NVIC, S32_SCB, ...), so they do not conflict with CMSIS. The
// __MPU_PRESENT define refers to the ARMv7-M core MPU, for which the CMSIS
// implementation is used.
#include "core_cm7.h"

// interrupt locking used by the platform BSP and the FreeRTOS port
#define ENABLE_INTERRUPTS()  __enable_irq()
#define DISABLE_INTERRUPTS() __disable_irq()

#define FEATURE_NVIC_PRIO_BITS (4U)
