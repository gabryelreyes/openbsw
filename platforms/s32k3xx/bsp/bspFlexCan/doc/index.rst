..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspFlexCan (S32K3xx)
====================

Overview
--------

The module ``bspFlexCan`` implements the low level FlexCAN device driver for
the S32K3xx family, ported from the S32K1xx driver. S32K3 specifics: the
message buffer RAM is ECC protected and is therefore initialized in freeze
mode, and the protocol engine clock always comes from the MC_CGM ``MUX_3``
clock (no ``CTRL1.CLKSRC`` bit).
