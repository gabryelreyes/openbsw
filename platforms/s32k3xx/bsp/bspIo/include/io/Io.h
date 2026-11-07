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

#include "bsp/Bsp.h"
#include "io/ioPorts.h"
#include "mcu/mcu.h"

#include <cstdint>

namespace bios
{

/**
 * Digital IO driver for the S32K3xx SIUL2.
 *
 * Each IO of the board configuration (bsp/io/io/ioConfiguration.h) maps to
 * one SIUL2 pad. The pad is configured through its MSCR register (source
 * signal select, input/output buffer enable, pull, drive strength, ...) and,
 * for peripheral inputs, through an IMCR register that routes the pad to the
 * peripheral. GPIO levels are written and read through the byte wide GPDO and
 * GPDI registers.
 */
class Io
{
public:
    Io() = delete;

    enum Level
    {
        LOW,
        HIGH
    };

    enum Direction
    {
        _DISABLED = 0,
        _IN       = (1 << 0),
        _OUT      = (1 << 1),
    };

    using tDirection = Direction;

    /**
     * MSCR source signal select (SSS): GPIO or alternate function 1..7.
     */
    enum
    {
        PINDISABLE = 0,
        GPIO       = 0,
        ALT1       = 1,
        ALT2       = 2,
        ALT3       = 3,
        ALT4       = 4,
        ALT5       = 5,
        ALT6       = 6,
        ALT7       = 7,
        SSS_MASK   = 7
    };

    /**
     * Further MSCR pad settings. The buffer enable bits (OBE, IBE) are not
     * part of the configured pad settings, they follow the IO direction.
     */
    enum : uint32_t
    {
        INPUT_FILTER = SIUL2_MSCR_IFE_MASK,
        STRENGTH_ON  = SIUL2_MSCR_DSE_MASK,
        PULLDOWN     = SIUL2_MSCR_PUE_MASK,
        PULLUP       = SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_PUS_MASK,
        SLEW_SLOW    = SIUL2_MSCR_SRC_MASK,
        PULL_KEEPER  = SIUL2_MSCR_PKE_MASK,
        INVERT       = SIUL2_MSCR_INV_MASK,
        BUFFER_MASK  = SIUL2_MSCR_IBE_MASK | SIUL2_MSCR_OBE_MASK
    };

    /** Marker for IOs that do not route their pad to a peripheral input. */
    static uint16_t const NO_IMCR = 0xFFFFU;

    static bool const LOW_ACTIVE  = true;
    static bool const HIGH_ACTIVE = false;

#undef BSP_IO_PIN_CONFIGURATION
#include "bsp/io/io/ioConfiguration.h"

    /**
     * Pad configuration of one IO.
     */
    struct PinConfiguration
    {
        uint8_t port;      ///< _PORTA_ .. _PORTG_
        uint8_t pinNumber; ///< 0 .. 31
        uint8_t dir;       ///< _DISABLED, _IN or _OUT
        uint8_t imcrSss;   ///< IMCR source signal select if imcr != NO_IMCR
        uint16_t imcr;     ///< IMCR index (signal number - 512) or NO_IMCR
        uint32_t pinCfg;   ///< MSCR value without OBE/IBE (SSS, pull, DSE, ...)
    };

    static bsp::BspReturnCode setDefaultConfiguration(uint16_t io);
    static bsp::BspReturnCode setConfiguration(uint16_t io, PinConfiguration const& cfg);
    static bsp::BspReturnCode getConfiguration(uint16_t io, PinConfiguration& cfg);
    static bool getPin(uint16_t io);
    static bsp::BspReturnCode setPin(uint16_t io, bool vol);

    /**
     * \return the SIUL2 MSCR index of the pad of \p io
     */
    static uint32_t getPinNumber(uint16_t io);
    static uint16_t const NUMBER_OF_IOS = NUMBER_OF_INPUTS_AND_OUTPUTS;
    static bsp::BspReturnCode resetConfig(uint16_t io);

private:
    static uint32_t mscrIndex(PinConfiguration const& cfg);

    static PinConfiguration const fPinConfiguration[NUMBER_OF_IOS];
};

} /* namespace bios */
