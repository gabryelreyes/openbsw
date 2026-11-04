/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "safeIo/SafeState.h"

#include <io/Io.h>

namespace safety
{

namespace
{
// For demonstration purposes, the safe state switches off the output buffer
// of the red LED.
constexpr auto SAFETY_PIN_ID = ::bios::Io::EVAL_LED_RED;
} // namespace

void SafeState::enterSafeState()
{
    ::bios::Io::PinConfiguration pinConfig;
    (void)::bios::Io::getConfiguration(SAFETY_PIN_ID, pinConfig);
    pinConfig.dir = ::bios::Io::_DISABLED;
    (void)::bios::Io::setConfiguration(SAFETY_PIN_ID, pinConfig);
}

void SafeState::leaveSafeState()
{
    ::bios::Io::PinConfiguration pinConfig;
    (void)::bios::Io::getConfiguration(SAFETY_PIN_ID, pinConfig);
    pinConfig.dir = ::bios::Io::_OUT;
    (void)::bios::Io::setConfiguration(SAFETY_PIN_ID, pinConfig);
}

} // namespace safety
