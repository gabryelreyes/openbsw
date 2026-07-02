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
 * Placeholder UART for the S32K3xx platform which discards all output and
 * never receives data.
 * TODO (Phase 2, doc/dev/s32k344_integration_plan.md): implement the driver
 * on the LPUART peripheral.
 */
class Uart
{
public:
    enum class Id : size_t;

    /**
     * Sends out a number of bytes.
     * \param data - span of data to be sent
     * \return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads a number of bytes.
     * \param data - span of data to be read
     * \return the number of bytes read
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures and starts the UART.
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
     * Waits until the UART is ready to transmit data.
     * \return true if ready within timeout, false otherwise
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
     * \param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

    Uart(Id id);

private:
    bool _initialized = false;
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp

#include "bsp/uart/UartConfig.h"
