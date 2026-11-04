/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/StaticBsp.h"

#include <bsp/Uart.h>
#include <bsp/timer/SystemTimer.h>
#include <outputPwm/OutputPwm.h>

#include <cstdio>

void StaticBsp::init()
{
    ::bsp::Uart::getInstance(::bsp::Uart::Id::TERMINAL).init();
    initSystemTimer();

    _output.init(0U);
    _digitalInput.init(0U);
    ::bios::OutputPwm::init();

    _adc.init();
    _adc.start();
    if (!::bios::AnalogInput::isCalibrated())
    {
        (void)printf("ADC1 calibration failed\r\n");
    }
}

void StaticBsp::cyclic()
{
    _digitalInput.cyclic();
    _adc.cyclic();
}
