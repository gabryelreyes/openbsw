..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspAdc (S32K3xx)
================

Overview
--------

The module ``bspAdc`` implements the driver ``bios::Adc`` for one instance of
the S32K3xx SAR ADC. It also provides the console command ``adc``
(``bios::AnalogTester``) and the scaling helpers used by the board's
``AnalogInputScale``. The board configuration (``bsp/adc``) maps the analog
input IDs to an ADC instance and channel, and provides the scaling functions.

Channels
--------

The channel numbers follow the S32K3xx SAR ADC numbering
(``adc/adcPhysicalName.h``):

- 0 to 7: precision inputs P0 to P7
- 32 to 55: standard inputs S0 to S23

External (multiplexed) inputs are not supported yet.

Operation
---------

``init()`` expects the ADC module clock to be enabled in the MC_ME. It then
does the following:

#. Sets the ADC clock prescaler (MCR[ADCLKSEL]) while the ADC is powered down.
#. Powers up the ADC.
#. Runs the built-in calibration (CALBISTREG). ``isCalibrated()`` reports
   whether the calibration succeeded.
#. Disables all interrupts, DMA requests and channels.
#. Sets the sampling time (CTR0 to CTR2).

``getValueSync()`` converts one channel on request. It enables only that
channel in NCMR, starts a software-triggered one-shot normal conversion
(MCR[NSTART]) and polls the channel's end-of-conversion flag. It then reads
the conversion data register (PCDR or ICDR) and checks its VALID bit. The ADC
returns 15-bit results (CDATA[14:0]), and the board's ``AdcInResolution``
shifts them to 12 bit. Interrupts are suspended for the few microseconds of
the conversion, so that tasks calling the driver at the same time cannot
interleave.
