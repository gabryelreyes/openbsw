..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspUart (S32K3xx)
=================

Overview
--------

The module ``bspUart`` implements a polling UART driver on the S32K3xx LPUART
peripheral, used for the interactive console. The mapping of the terminal
UART to an LPUART instance, its clock and its pins is provided by the board
configuration (``bsp/uart/UartConfig.h``).
