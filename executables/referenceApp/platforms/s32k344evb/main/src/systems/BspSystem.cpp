/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/BspSystem.h"

namespace systems
{

BspSystem::BspSystem(::async::ContextType const context, StaticBsp& staticBsp)
: ::lifecycle::SingleContextLifecycleComponent(context)
, _context(context)
, _timeout()
, _staticBsp(staticBsp)
, _analogTester()
, _digitalInputTester()
, _outputTester()
, _asyncAnalogTester(_analogTester, context)
, _asyncDigitalInputTester(_digitalInputTester, context)
, _asyncOutputTester(_outputTester, context)
{}

void BspSystem::init() { transitionDone(); }

void BspSystem::run()
{
    ::async::scheduleAtFixedRate(_context, *this, _timeout, 10U, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void BspSystem::shutdown()
{
    _timeout.cancel();
    transitionDone();
}

void BspSystem::execute() { _staticBsp.cyclic(); }

} // namespace systems
