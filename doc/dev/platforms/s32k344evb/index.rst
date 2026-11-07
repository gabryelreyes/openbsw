..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _s32k344evb_overview:

S32K344 Evaluation Board
========================

Overview
--------

- The S32K3X4EVB-T172 is an evaluation board by NXP Semiconductors for the S32K344 general purpose
  automotive microcontroller (Arm Cortex-M7 with FPU, 4 MB program flash, 512 KB SRAM including
  the tightly coupled memories).
- Eclipse OpenBSW provides a reference application for this platform which is built with the
  ``s32k344-freertos`` preset. It offers an interactive console on the OpenSDA VCOM port and
  CAN/UDS communication. The platform code lives in ``platforms/s32k3xx`` (SoC level) and
  ``executables/referenceApp/platforms/s32k344evb`` (board level).

Scope
-----

The platform provides a Cortex-M7 image with no regression to the existing platforms:

- startup code with vector table and ECC initialization of SRAM and DTCM,
- a boot header (IVT) for the Boot Assist Firmware, so the image starts from a power cycle once
  flashed,
- a linker script placing the image in program flash, the main stack in DTCM and data, BSS and
  heap in the system SRAM,
- the clock tree configured to a 160 MHz PLL core clock,
- the FreeRTOS Cortex-M7 port driven by the SysTick timer,
- the interrupt manager and the system timer based on the DWT cycle counter,
- the console on LPUART6 (OpenSDA VCOM port),
- FlexCAN0 at 500 kbit/s with the docan transport and UDS stack,
- lifecycle management with the safety supervisor,
- digital IO (RGB LED outputs, user buttons SW5 and SW6) and the ADC (on-board potentiometer on
  PTA11, ADC1 channel S10) with the ``in``, ``out`` and ``adc`` console commands.

Not yet available: PWM, Ethernet, the middleware demo, the watchdog, the MPU and the persistent
storage. The corresponding ``PLATFORM_SUPPORT_*`` options are therefore switched off in
``executables/referenceApp/platforms/s32k344evb/Options.cmake`` and get enabled as the drivers
become available. Open items are marked with ``TODO`` comments in the sources.

Reference Links
---------------

1. **S32K3X4EVB-T172 Evaluation Board Product Page**:
    - `S32K3X4EVB-T172 Evaluation Board <https://www.nxp.com/design/design-center/development-boards-and-designs/S32K3X4EVB-T172>`_

2. **S32K344 MCU Documentation**:
    - `Reference Manual: S32K3xx Reference Manual (login required) <https://www.nxp.com/webapp/Download?colCode=S32K3XXRM>`_
    - `Data Sheet: S32K3xx Data Sheet <https://www.nxp.com/docs/en/data-sheet/S32K3xx_DS.pdf>`_

3. **S32K344 Software and Tools**:
    - `S32 Design Studio for S32 Platform <https://www.nxp.com/design/design-center/software/development-software/s32-design-studio-ide/s32-design-studio-for-s32-platform:S32DS-S32PLATFORM>`_
    - `PE Micro OpenSDA <http://www.pemicro.com/opensda/>`_

4. **Community and Support**:
    - `NXP Community Forum <https://community.nxp.com/>`_
    - `NXP Technical Support <https://www.nxp.com/products/processors-and-microcontrollers/s32-automotive-platform/s32k-auto-general-purpose-mcus/s32k3-microcontrollers-for-general-purpose:S32K3#support>`_

Build environment
-----------------

For step-by-step instructions on building see :ref:`setup_s32k344_build`. The platform uses the same ARM toolchain
as the S32K148EVB; the preset selects the Cortex-M7 core and the ``fpv5-d16`` FPU via the
``OPENBSW_ARM_CPU`` and ``OPENBSW_ARM_FPU`` cache variables:

.. code-block:: bash

    cmake --preset s32k344-freertos
    cmake --build --preset s32k344-freertos

The ELF file is written to
``build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``.

Flashing and debugging use the PE Micro GDB server together with ``test/pyTest/flash.gdb`` and
``test/pyTest/reset.gdb``; SEGGER J-Link and the NXP S32 Debug Probe work as alternatives:

.. code-block:: bash

    pegdbserver_console -device=NXP_S32K3xx_S32K344 -startserver -serverport=7224

See :ref:`s32k344_gdb_via_usb` for setting up the GDB server (including the Windows/WSL setup),
the console, CAN and running the pytests on the board.

Running the tests
-----------------

The pytests in ``test/pyTest`` run against the board with ``pytest --target=s32k344``, using
``target_s32k344.toml``. See :ref:`s32k344_gdb_via_usb` for the required setup (gdb server, console
and CAN) and for the test suites that currently apply to this platform.

Memory map
----------

.. csv-table:: Memory regions used by the linker script
   :header: "Region", "Start", "End", "Usage"
   :widths: 15, 15, 15, 40

   "Program flash", "0x00400000", "0x007F3FFF", "Boot header, vector table and application (last 48 KB reserved by the BAF)"
   "DTCM", "0x20000000", "0x2001FFFF", "Main stack"
   "System SRAM", "0x20400000", "0x2044FFFF", "Data, BSS and heap"

Connections
-----------

.. csv-table:: Pin Configuration
   :header: "Name", "State and Function", "Pin ID", "Usage"
   :widths: 10, 20, 15, 20

   "PTA15", "IN, IMCR 705 = 2", "LPUART6_RX", "Console (OpenSDA VCOM)"
   "PTA16", "OUT, ALT 5", "LPUART6_TX", "Console (OpenSDA VCOM)"
   "PTA6", "IN, IMCR 512 = 2", "CAN0_RX", "FlexCAN 0"
   "PTA7", "OUT, ALT 4", "CAN0_TX", "FlexCAN 0"

Notes
-----

- The on-board CAN transceiver of board revision B2 is a TJA1443 CAN FD device. Its EN/STB pins
  are driven to normal mode by the BSP; no further configuration is required.
- The C40 flash read wait states are left at the conservative reset default.
- The last 48 KB of the program flash (``0x007F4000`` to ``0x007FFFFF``) are reserved for the
  secure boot assist firmware and are not part of the linker's flash region. If the HSE firmware
  were installed, the reservation would grow to 176 KB of program flash and 40 KB of data flash.
  The evaluation board used here has no HSE firmware installed and this port does not use the HSE.
