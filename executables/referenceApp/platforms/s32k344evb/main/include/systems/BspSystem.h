/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "console/AsyncCommandWrapper.h"
#include "inputManager/DigitalInputTester.h"
#include "lifecycle/SingleContextLifecycleComponent.h"
#include "lifecycle/StaticBsp.h"
#include "outputManager/OutputTester.h"
#include "tester/AnalogTester.h"

namespace systems
{

/**
 * Runs the cyclic part of the static BSP and provides the console commands
 * for the digital inputs ("in"), the digital outputs ("out") and the analog
 * inputs ("adc").
 */
class BspSystem
: public ::lifecycle::SingleContextLifecycleComponent
, private ::async::RunnableType
{
public:
    BspSystem(::async::ContextType context, StaticBsp& staticBsp);

    void init() override;
    void run() override;
    void shutdown() override;

    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
    StaticBsp& _staticBsp;
    bios::AnalogTester _analogTester;
    bios::DigitalInputTester _digitalInputTester;
    bios::OutputTester _outputTester;
    ::console::AsyncCommandWrapper _asyncAnalogTester;
    ::console::AsyncCommandWrapper _asyncDigitalInputTester;
    ::console::AsyncCommandWrapper _asyncOutputTester;
};

} // namespace systems
