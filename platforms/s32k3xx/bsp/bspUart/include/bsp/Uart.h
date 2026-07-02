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

#include <bsp/uart/UartConcept.h>

namespace bsp
{
/**
 * Polling UART driver on the S32K3xx LPUART peripheral. The mapping of the
 * terminal UART to an LPUART instance and its pins is provided by the board
 * configuration (bsp/uart/UartConfig.h).
 */
class Uart
{
public:
    enum class Id : size_t;

    /**
     * Sends out a number of bytes, blocking until all bytes are written.
     * \param data - span of data to be sent
     * \return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads the bytes currently available in the receiver, non blocking.
     * \param data - span of data to be read
     * \return the number of bytes read
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures the pins and starts the UART.
     * This method must be called before using the read/write methods.
     */
    void init();

    /**
     * Deinitializes the UART.
     */
    void deinit();

    /**
     * Returns if this Uart instance is initialized or not.
     */
    bool isInitialized() const;

    /**
     * Waits until the transmitter is idle.
     * \return true if ready, false if the UART is not initialized
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
     * \param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

    Uart(Id id);

private:
    Id _id;
    bool _initialized = false;
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp

#include "bsp/uart/UartConfig.h"
