..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _s32k344_gdb_via_usb:

Flash, debug and test the S32K3X4EVB-T172 board
===============================================

The `S32K3X4EVB-T172 board <https://www.nxp.com/design/design-center/development-boards-and-designs/S32K3X4EVB-T172>`_
carries an on-board P&E Micro debug probe (Embedded Multilink).
A single USB cable between the board and your computer provides both
the debugger connection for flashing and debugging and the OpenSDA virtual COM port used as the console.
As for the S32K148, P&E Micro's gdb server must be used to talk to this probe.

This page assumes you have built the image as described in :doc:`setup_s32k344_build`.
Pin mapping, memory map and the current feature scope are described in :ref:`s32k344evb_overview`.

.. note::

   Please refer to :ref:`known_issues_with_s32k344_board`.

Get P&E Micro's gdb server and USB driver
-----------------------------------------

The gdb server (``pegdbserver_console``) and the OpenSDA USB driver are the same as for the S32K148.
Follow the steps in :doc:`setup_s32k148_gdbserver` to get them,
either from an S32 Design Studio installation or from the free
"GDB Server Plug-In for Eclipse-based ARM IDEs" on the P&E Micro website.
Use a recent version of the plug-in, since older versions do not know the S32K3 devices.

The unpacked plug-in contains executables for Linux (``lin/``), Windows (``win32/``) and OS X (``osx/``).
Each one needs its sibling folders (e.g. ``gdi/`` and ``supportFiles_ARM``),
so always run it from inside the unpacked tree rather than copying the executable elsewhere.

Running gdb server on a Linux host
----------------------------------

With the board connected via USB, start the gdb server with the S32K344 device name:

.. code-block:: bash

   ./pegdbserver_console -startserver -device=NXP_S32K3xx_S32K344 -serverport=7224

Depending on how the USB driver was installed you may need to run this with ``sudo``.
When it reports ``All Servers Running`` it is listening on ``127.0.0.1:7224``,
which is the address used by the gdb scripts in ``test/pyTest``.

Some points to note:

* Only one gdb client can be connected at a time.
  If a gdb client is killed while connected, the server may stop responding and has to be restarted.
* Do not run ``pegdbserver_console -h`` while a server is running: it grabs the probe and the running server loses its connection.
* If the server reports ``Failure to enter debug mode`` or ``Can not enter background mode``,
  unplug and replug the board's USB cable (this power-cycles the MCU and the probe) and start the server again.

Running gdb server on Windows with WSL
--------------------------------------

.. warning::

   Do **not** forward the P&E probe into WSL with ``usbipd`` (see :ref:`setup_wsl_usb`) to run the Linux gdb server there.
   USB transfers are silently dropped on that path: the server reports success,
   but parts of the image remain erased in flash.
   The board then crashes at varying places with a hard fault (typically an undefined instruction in an erased region).
   Small test images often survive, which makes the problem easy to miss.

Instead, keep the probe attached to Windows (``usbipd detach`` and ``usbipd unbind`` it if it was shared before),
and run the Windows executable ``win32/pegdbserver_console.exe`` natively,
binding it to the Windows address that is reachable from WSL.
With the default NAT networking of WSL this is the default gateway of the WSL instance,
which you can find with ``ip route show default`` in WSL.
In PowerShell (quote the ``-serverip`` argument):

.. code-block:: powershell

   .\pegdbserver_console.exe -startserver -device=NXP_S32K3xx_S32K344 -serverport=7224 "-serverip=<windows-ip>"

Allow the server through the Windows firewall when asked.
From WSL, gdb then connects over TCP to ``<windows-ip>:7224`` instead of ``localhost:7224``,
so use local copies of ``test/pyTest/flash.gdb`` and ``test/pyTest/reset.gdb`` with the address changed accordingly.

In this setup the OpenSDA virtual COM port also stays on Windows and appears there as a COM port.

Running gdb
-----------

``arm-none-eabi-gdb`` from the GCC for ARM toolchain is the gdb client
(it is also available in the development container).
If you use ``gdb-multiarch`` instead, run ``set architecture armv7e-m`` before connecting to the server.

To start an interactive debug session, run from the base directory:

.. code-block:: bash

   arm-none-eabi-gdb -x tools/gdb/pegdbserver.gdb build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf

Note that attaching gdb to a running core resets it,
so registers and memory only show meaningful values once the core has been started again
(e.g. after ``continue`` stopped at a breakpoint).

Flash and reset the board on command-line
-----------------------------------------

With the gdb server running, flash the board in a single command:

.. code-block:: bash

   arm-none-eabi-gdb -batch -x test/pyTest/flash.gdb build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf

and reset it, running the application from the start:

.. code-block:: bash

   arm-none-eabi-gdb -batch -x test/pyTest/reset.gdb build/s32k344-freertos/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf

If you suspect a corrupted flash image, dump the flash with gdb
(``dump binary memory``, starting at ``0x00400000``)
and compare it with the output of ``arm-none-eabi-objcopy -O binary --gap-fill=0xff`` for the ELF file.

Opening the console
-------------------

The console uses LPUART6, routed to the OpenSDA virtual COM port, at 115200 baud, 8N1.
On Linux the port typically appears as ``/dev/ttyACM0``; on Windows it is a COM port.
Open it with a terminal program of your choice, e.g.:

.. code-block:: bash

   picocom -b 115200 /dev/ttyACM0

After a reset the boot log runs from ``DEBUG: Run level 1`` to ``INFO: Run level 9 done``.
Type ``help`` to list the available console commands.

Connecting CAN
--------------

The reference application runs FlexCAN0 at 500 kbit/s through the board's on-board CAN transceiver.
Connect a SocketCAN compatible adapter, such as a PEAK PCAN-USB, to the board's CAN connector
and bring the interface up at the same bitrate:

.. code-block:: bash

   sudo ip link set can0 up type can bitrate 500000

A PCAN-USB adapter works in WSL when forwarded with ``usbipd`` as described in
:ref:`setup_wsl_usb` and :ref:`setup_wsl_socketcan` (the limitation above only affects the P&E probe).
Once the board has booted you can check the bus with ``candump can0``.

Running the pytests
-------------------

The tests in ``test/pyTest`` are run against the board with ``target_s32k344.toml``.
Set up the python environment as described in ``test/pyTest/README.md``, then,
with the gdb server running, the board connected and ``can0`` up, run in ``test/pyTest``:

.. code-block:: bash

   pytest --target=s32k344

``target_s32k344.toml`` flashes the image once with ``flash.gdb`` and resets the board with ``reset.gdb`` before each test.
It expects the gdb server on ``localhost:7224``, ``arm-none-eabi-gdb`` on the ``PATH``,
the console on ``/dev/ttyACM0`` and the CAN interface ``can0``.
For the Windows/WSL setup above, copy it to a new ``target_<name>.toml``,
point the commands at your edited gdb scripts and select it with ``--target=<name>``.
Since the console is then a Windows COM port, it can be made available to the tests
by forwarding it over TCP from a small serial bridge on Windows to a pseudo terminal in WSL,
and setting ``port`` in the ``[serial]`` section to that pseudo terminal.

Current state of the tests on this board:

* The ``can``, ``docan`` and ``uds`` suites are expected to pass.
* The console tests pass. ``test_storage.py`` is skipped, because persistent storage
  is not yet supported on this platform (``storage = false`` in the ``[features]`` table of
  ``target_s32k344.toml``).
* The ``enet`` and ``middleware`` suites are skipped the same way, since Ethernet and the middleware
  demo are not enabled for the S32K344. The ADC test needs the hardware tester board and is skipped.

Run the suites that apply, e.g.:

.. code-block:: bash

   pytest --target=s32k344 console can docan uds
