..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _s32k344evb_overview:

S32K3X4EVB-T172 Evaluation Board
================================

Overview
--------

- The S32K3X4EVB-T172 is an evaluation board by NXP Semiconductors for the
  S32K344 general purpose automotive microcontroller (Arm Cortex-M7, 4 MB
  flash, 512 KB SRAM including TCMs).
- Eclipse OpenBSW provides a reference application for this platform
  (``s32k344-freertos-gcc`` preset) with an interactive console on the
  OpenSDA VCOM port and CAN/UDS communication.
- The image contains a boot header (IVT) for the Boot Assist Firmware, so it
  starts from a power cycle once flashed.

Reference Links
---------------

1. **S32K3X4EVB-T172 Product Page**:
    - `S32K3X4EVB-T172 Evaluation Board <https://www.nxp.com/design/design-center/development-boards-and-designs/S32K3X4EVB-T172>`_

2. **S32K344 MCU Documentation**:
    - `Reference Manual: S32K3xx Reference Manual (login required) <https://www.nxp.com/webapp/Download?colCode=S32K3XXRM>`_
    - `Data Sheet: S32K3xx Data Sheet <https://www.nxp.com/docs/en/data-sheet/S32K3xx_DS.pdf>`_

3. **Software and Tools**:
    - `S32 Design Studio for S32 Platform <https://www.nxp.com/design/design-center/software/development-software/s32-design-studio-ide/s32-design-studio-for-s32-platform:S32DS-S32PLATFORM>`_
    - `PE Micro OpenSDA <http://www.pemicro.com/opensda/>`_

4. **Community and Support**:
    - `NXP Community Forum <https://community.nxp.com/>`_

Build environment
-----------------
For instructions on building see :ref:`learning_setup`. The platform is built
with the ``s32k344-freertos-gcc`` preset. Flashing and debugging use the PE
Micro GDB server (``pegdbserver_console -device=NXP_S32K3xx_S32K344
-startserver -serverport=7224``) together with ``test/pyTest/flash.gdb`` /
``reset.gdb``; SEGGER J-Link and the NXP S32 Debug Probe work as
alternatives.

Notes
-----

- The on-board CAN transceivers are secure TJA1153 devices. Their EN/STB pins
  are driven to normal mode by the BSP, but a factory-fresh transceiver may
  require a one-time configuration sequence before it forwards traffic.
- The C40 flash read wait states are left at the conservative reset default.

Connections
-----------

.. csv-table:: Pin Configuration
   :header: "Name", "State and Function", "Pin ID", "Usage"
   :widths: 10, 20, 15, 20

   "PTA15", "IN, IMCR 705 = 2", "LPUART6_RX", "Console (OpenSDA VCOM)"
   "PTA16", "OUT, ALT 5", "LPUART6_TX", "Console (OpenSDA VCOM)"
   "PTA6", "IN, IMCR 512 = 2", "CAN0_RX", "FlexCAN 0"
   "PTA7", "OUT, ALT 4", "CAN0_TX", "FlexCAN 0"
   "PTC20", "OUT, GPIO", "CAN0_STB", "TJA1153 standby"
   "PTC21", "OUT, GPIO", "CAN0_EN", "TJA1153 enable"
