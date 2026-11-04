..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspClock (S32K3xx)
==================

Overview
--------

The module ``bspClock`` configures the S32K344 clock tree: FXOSC (16 MHz
crystal) feeds the PLL, resulting in a 160 MHz core clock, 80 MHz
``AIPS_PLAT_CLK``, 40 MHz ``AIPS_SLOW_CLK`` and a 40 MHz FlexCAN protocol
engine clock (MC_CGM ``MUX_3``). It also ungates the peripheral clocks used
by the BSP through the MC_ME partition interface.
