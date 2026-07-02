# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM s32k3xx)

set(S32K3_CHIP
    "S32K344"
    CACHE STRING "S32K3 chip on the board")

# Platform bring-up is done in phases (see doc/dev/s32k344_integration_plan.md).
# Features are switched on as the corresponding BSP drivers become available.
set(PLATFORM_SUPPORT_IO
    OFF
    CACHE BOOL "Turn IO support on or off" FORCE)
set(PLATFORM_SUPPORT_CAN
    OFF
    CACHE BOOL "Turn CAN support on or off" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    OFF
    CACHE BOOL "Turn ethernet support on or off" FORCE)
set(PLATFORM_SUPPORT_TRANSPORT
    OFF
    CACHE BOOL "Turn TRANSPORT support on or off" FORCE)
set(PLATFORM_SUPPORT_UDS
    OFF
    CACHE BOOL "Turn UDS support on or off" FORCE)
set(PLATFORM_SUPPORT_WATCHDOG
    OFF
    CACHE BOOL "Turn OFF Watchdog support" FORCE)
set(PLATFORM_SUPPORT_MPU
    OFF
    CACHE BOOL "Turn OFF MPU support" FORCE)
set(PLATFORM_SUPPORT_STORAGE
    OFF
    CACHE BOOL "Turn persistent storage on or off" FORCE)
set(PLATFORM_SUPPORT_ROM_CHECK
    OFF
    CACHE BOOL "Turn ROM check support off" FORCE)
