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
  ``s32k344-freertos`` preset. The platform code lives in ``platforms/s32k3xx`` (SoC level) and
  ``executables/referenceApp/platforms/s32k344evb`` (board level).

Scope
-----

The platform currently provides a verified Cortex-M7 image with no regression to the existing
platforms:

- startup code with vector table and ECC initialization of SRAM and DTCM,
- a linker script placing the image in program flash, the main stack in DTCM and data, BSS and
  heap in the system SRAM,
- the FreeRTOS Cortex-M7 port driven by the SysTick timer,
- the interrupt manager and the system timer based on the DWT cycle counter,
- lifecycle management with the safety supervisor.

Not yet available: the console (``stdIo`` discards all output, ``bspUart`` is a placeholder),
the clock tree configuration (the core runs on the default FIRC clock), CAN, IO and the
persistent storage. The corresponding ``PLATFORM_SUPPORT_*`` options are therefore switched off
in ``executables/referenceApp/platforms/s32k344evb/Options.cmake`` and get enabled as the drivers
become available. Open items are marked with ``TODO`` comments in the sources.

The image has no boot header (IVT) for the Boot Assist Firmware yet, so it is started via
debugger instead of from a power cycle.

Reference Links
---------------

1. **S32K3X4EVB-T172 Evaluation Board Product Page**:
    - `S32K3X4EVB-T172 Evaluation Board <https://www.nxp.com/design/design-center/development-boards-and-designs/S32K3X4EVB-T172>`_

2. **S32K344 MCU Documentation**:
    - `Reference Manual: S32K3xx Reference Manual (login required) <https://www.nxp.com/webapp/Download?colCode=S32K3XXRM>`_
    - `Data Sheet: S32K3xx Data Sheet <https://www.nxp.com/docs/en/data-sheet/S32K3xx_DS.pdf>`_

3. **S32K344 Software and Tools**:
    - `S32 Design Studio for S32 Platform <https://www.nxp.com/design/design-center/software/development-software/s32-design-studio-ide/s32-design-studio-for-s32-platform:S32DS-S32PLATFORM>`_

4. **Community and Support**:
    - `NXP Community Forum <https://community.nxp.com/>`_
    - `NXP Technical Support <https://www.nxp.com/products/processors-and-microcontrollers/s32-automotive-platform/s32k-auto-general-purpose-mcus/s32k3-microcontrollers-for-general-purpose:S32K3#support>`_

Build environment
-----------------

For instructions on building see :ref:`learning_setup`. The platform uses the same ARM toolchain
as the S32K148EVB; the preset selects the Cortex-M7 core and the ``fpv5-d16`` FPU via the
``OPENBSW_ARM_CPU`` and ``OPENBSW_ARM_FPU`` cache variables:

.. code-block:: bash

    cmake --preset s32k344-freertos
    cmake --build --preset s32k344-freertos

The ELF file is written to
``build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``.

Memory map
----------

.. csv-table:: Memory regions used by the linker script
   :header: "Region", "Start", "End", "Usage"
   :widths: 15, 15, 15, 40

   "Program flash", "0x00400000", "0x007F3FFF", "Vector table and application (last 48 KB reserved by the BAF)"
   "DTCM", "0x20000000", "0x2001FFFF", "Main stack"
   "System SRAM", "0x20400000", "0x2044FFFF", "Data, BSS and heap"
