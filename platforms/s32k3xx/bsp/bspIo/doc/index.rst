..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspIo (S32K3xx)
===============

Overview
--------

The module ``bspIo`` implements the digital IO driver ``bios::Io`` on the
S32K3xx SIUL2. It provides the ``io/Io.h`` interface used by the generic input
and output managers (``bspInputManager``, ``bspOutputManager``).

The board configuration (``bsp/io/io/ioConfiguration.h``) defines the IO IDs
(``Io::PinId``) and a ``PinConfiguration`` per IO:

- ``port`` and ``pinNumber`` select the pad. Its SIUL2 MSCR (and GPDO/GPDI)
  index is ``port * 32 + pinNumber``.
- ``dir`` (``_DISABLED``, ``_IN``, ``_OUT``) sets the input buffer enable
  (MSCR[IBE]) or the output buffer enable (MSCR[OBE]).
- ``pinCfg`` holds the other MSCR settings: source signal select (``GPIO``,
  ``ALT1`` .. ``ALT7``), pull up or down, drive strength, slew rate, input
  filter and inversion.
- ``imcr`` and ``imcrSss`` route the pad to a peripheral input through an
  IMCR register (``NO_IMCR`` for GPIOs).

``setPin()`` writes the byte-wide GPDO register of the pad. It works only for
IOs that the configuration defines as outputs. You can write the output latch
before the pad is configured, so outputs start at their default level without
a glitch. ``getPin()`` reads GPDI if the input buffer is enabled and GPDO
otherwise, so reading an output returns the level it was set to.

The S32K3xx SIUL2 has no per-pad lock bit like the S32K1xx PORT_PCR[LK], so
``PinConfiguration`` has no lock option.
