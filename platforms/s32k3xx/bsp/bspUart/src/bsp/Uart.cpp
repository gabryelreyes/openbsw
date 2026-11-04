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

#include "mcu/mcu.h"

using bsp::Uart;

namespace
{
uint32_t const OVERSAMPLING_RATIO = 16U;

LPUART_Type* instanceRegisters(Uart::Id const id)
{
    (void)id;
    return UART_TERMINAL_INSTANCE; // only the terminal UART is configured
}
} // namespace

Uart::Uart(Id id) : _id(id) {}

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    if (!_initialized)
    {
        return 0U;
    }
    LPUART_Type* const lpuart = instanceRegisters(_id);
    for (size_t i = 0U; i < data.size(); ++i)
    {
        while ((lpuart->STAT & LPUART_STAT_TDRE_MASK) == 0U) {}
        lpuart->DATA = data[i];
    }
    return data.size();
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    if ((!_initialized) || (data.size() == 0U))
    {
        return 0U;
    }
    LPUART_Type* const lpuart = instanceRegisters(_id);
    // Clear a pending overrun, it blocks further reception.
    if ((lpuart->STAT & LPUART_STAT_OR_MASK) != 0U)
    {
        lpuart->STAT = LPUART_STAT_OR_MASK;
    }
    size_t bytesRead = 0U;
    while ((bytesRead < data.size()) && ((lpuart->STAT & LPUART_STAT_RDRF_MASK) != 0U))
    {
        data[bytesRead] = static_cast<uint8_t>(lpuart->DATA);
        ++bytesRead;
    }
    return bytesRead;
}

void Uart::init()
{
    if (_initialized)
    {
        return;
    }
    uartConfigureTerminalPins();

    LPUART_Type* const lpuart = instanceRegisters(_id);
    lpuart->GLOBAL            = LPUART_GLOBAL_RST_MASK;
    lpuart->GLOBAL            = 0U;
    lpuart->BAUD
        = LPUART_BAUD_OSR(OVERSAMPLING_RATIO - 1U)
          | LPUART_BAUD_SBR(UART_TERMINAL_CLOCK_HZ / (OVERSAMPLING_RATIO * UART_TERMINAL_BAUD));
    lpuart->CTRL = LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK;

    _initialized = true;
}

void Uart::deinit()
{
    if (_initialized)
    {
        LPUART_Type* const lpuart = instanceRegisters(_id);
        lpuart->CTRL              = 0U;
        _initialized              = false;
    }
}

Uart& Uart::getInstance(Id id)
{
    (void)id;
    static Uart instance{Uart::Id::TERMINAL};
    return instance;
}

bool Uart::isInitialized() const { return _initialized; }

bool Uart::waitForTxReady()
{
    if (!_initialized)
    {
        return false;
    }
    LPUART_Type* const lpuart = instanceRegisters(_id);
    while ((lpuart->STAT & LPUART_STAT_TC_MASK) == 0U) {}
    return true;
}
