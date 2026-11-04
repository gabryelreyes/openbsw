..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _setup_s32k344_build:

Set up build for S32K344 platform
=================================

The S32K344 platform targets the
`S32K3X4EVB-T172 evaluation board <https://www.nxp.com/design/design-center/development-boards-and-designs/S32K3X4EVB-T172>`_.
Its reference application is built with the same GCC for ARM toolchain as the S32K148 platform,
so if you can already build for the S32K148 nothing needs to be installed.
The board itself is described in :ref:`s32k344evb_overview`.

Required tools:

* gcc for ARM (tested with version :prop:`tool:gcc-arm-none-eabi`)
* cmake >= :prop:`tool:cmake_version`
* make

Using a local toolchain
-----------------------

Set up the GCC for ARM toolchain and the ``CC``/``CXX`` environment variables exactly as described in
:doc:`setup_s32k148_ubuntu_build` (on Windows, do this in an Ubuntu :prop:`tool:ubuntu_version` shell running in WSL).
Then, in the base directory run:

.. code-block:: bash

    cmake --preset s32k344-freertos
    cmake --build --preset s32k344-freertos

The preset selects the Cortex-M7 core and the ``fpv5-d16`` FPU,
so the compiler options differ from the S32K148 (Cortex-M4F) build even though the toolchain is the same.

Using the development container
-------------------------------

The repository also provides a docker based development environment which already contains
the GCC for ARM toolchain in ``/opt/arm-gnu-toolchain``.
The toolchain is not on the ``PATH`` of non-interactive shells,
so add it before running cmake. From the base directory you can build without installing anything locally:

.. code-block:: bash

    docker compose run --rm development bash -c \
        'export PATH=/opt/arm-gnu-toolchain/bin:$PATH && cmake --preset s32k344-freertos && cmake --build --preset s32k344-freertos'

The build directory is shared with the host, so the ELF file can be flashed from outside the container.

Build output
------------

The build files are written to a new subdirectory named ``build/s32k344-freertos``
and the built executable is found at
``build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``
which you can flash on the S32K3X4EVB-T172 board.

Only a FreeRTOS based preset exists for this platform so far.
The application starts with the console and CAN/UDS enabled;
IO, the watchdog, the MPU and the persistent storage are not yet available
(see :ref:`s32k344evb_overview` for the current scope).

The image contains a boot header for the Boot Assist Firmware,
so once it has been flashed it also starts after a power cycle or a press of the reset button.

Next :doc:`setup_s32k344_gdbserver`
