/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <bsp/Uart.h>

using bsp::Uart;

Uart::Uart(Id id) { (void)id; }

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    // TODO (Phase 2): transmit via LPUART. The placeholder discards the data.
    return data.size();
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    (void)data;
    return 0U;
}

void Uart::init() { _initialized = true; }

void Uart::deinit() { _initialized = false; }

Uart& Uart::getInstance(Id id)
{
    (void)id;
    static Uart instance{Uart::Id::TERMINAL};
    return instance;
}

bool Uart::isInitialized() const { return _initialized; }

bool Uart::waitForTxReady() { return true; }
