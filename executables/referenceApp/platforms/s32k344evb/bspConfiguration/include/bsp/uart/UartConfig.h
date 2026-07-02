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

/* Terminal UART of the S32K3X4EVB-T172: LPUART6 on the OpenSDA VCOM port
 * (PTA15 = LPUART6_RX, PTA16 = LPUART6_TX), 115200 baud.
 * LPUART0..7 are clocked from AIPS_PLAT_CLK (80 MHz, see bspClock). */
#define UART_TERMINAL_INSTANCE IP_LPUART_6
#define UART_TERMINAL_CLOCK_HZ 80000000U
#define UART_TERMINAL_BAUD 115200U

namespace bsp
{

enum class Uart::Id : size_t
{
    TERMINAL,
    INVALID,
};

static constexpr size_t NUMBER_OF_UARTS = static_cast<size_t>(Uart::Id::INVALID);

/**
 * Configures the SIUL2 pin multiplexing for the terminal UART.
 * Implemented by the board configuration.
 */
void uartConfigureTerminalPins();

} // namespace bsp
