/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/Io.h"

#include "mcu/mcu.h"

using namespace bsp;

namespace
{
// Byte wide GPIO pad data registers (see the S32K3xx Reference Manual,
// SIUL2). GPDO/GPDI are grouped in 32 bit words of four pads with the lowest
// pad in the most significant byte, so pad n is at byte (n & ~3) + (3 - n % 4).
uintptr_t const GPDO_OFFSET = 0x1300U;
uintptr_t const GPDI_OFFSET = 0x1500U;

uintptr_t padDataByteOffset(uint32_t const mscr) { return (mscr & ~3U) + (3U - (mscr & 3U)); }

uint8_t volatile* gpdo(uint32_t const mscr)
{
    return reinterpret_cast<uint8_t volatile*>(
        reinterpret_cast<uintptr_t>(IP_SIUL2) + GPDO_OFFSET + padDataByteOffset(mscr));
}

uint8_t const volatile* gpdi(uint32_t const mscr)
{
    return reinterpret_cast<uint8_t const volatile*>(
        reinterpret_cast<uintptr_t>(IP_SIUL2) + GPDI_OFFSET + padDataByteOffset(mscr));
}
} // namespace

namespace bios
{

#undef BSP_IO_PIN_CONFIGURATION
#define BSP_IO_PIN_CONFIGURATION 1
#include "bsp/io/io/ioConfiguration.h"

uint32_t Io::mscrIndex(PinConfiguration const& cfg)
{
    return (static_cast<uint32_t>(cfg.port) * static_cast<uint32_t>(PINS_PER_PORT))
           + static_cast<uint32_t>(cfg.pinNumber);
}

BspReturnCode Io::setDefaultConfiguration(uint16_t const io)
{
    if (io < NUMBER_OF_IOS)
    {
        return setConfiguration(io, fPinConfiguration[io]);
    }
    return BSP_NOT_SUPPORTED;
}

BspReturnCode Io::setConfiguration(uint16_t const io, PinConfiguration const& cfg)
{
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    if ((cfg.pinNumber != fPinConfiguration[io].pinNumber)
        || (cfg.port != fPinConfiguration[io].port))
    {
        // Mapping of IOs must be the same as default
        return BSP_ERROR;
    }
    uint32_t const mscr = mscrIndex(cfg);
    if (mscr >= SIUL2_MSCR_COUNT)
    {
        return BSP_NOT_SUPPORTED;
    }

    uint32_t value = cfg.pinCfg & ~static_cast<uint32_t>(BUFFER_MASK);
    if (_IN == cfg.dir)
    {
        value |= SIUL2_MSCR_IBE_MASK;
    }
    else if (_OUT == cfg.dir)
    {
        value |= SIUL2_MSCR_OBE_MASK;
    }
    else
    {
        // disabled: neither input nor output buffer
    }
    IP_SIUL2->MSCR[mscr] = value;

    if ((cfg.imcr != NO_IMCR) && (cfg.imcr < SIUL2_IMCR_COUNT))
    {
        IP_SIUL2->IMCR[cfg.imcr] = static_cast<uint32_t>(cfg.imcrSss) & SIUL2_IMCR_SSS_MASK;
    }
    return BSP_OK;
}

BspReturnCode Io::getConfiguration(uint16_t const io, PinConfiguration& cfg)
{
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    cfg.port            = fPinConfiguration[io].port;
    cfg.pinNumber       = fPinConfiguration[io].pinNumber;
    cfg.imcr            = fPinConfiguration[io].imcr;
    cfg.imcrSss         = 0U;
    uint32_t const mscr = mscrIndex(cfg);
    if (mscr >= SIUL2_MSCR_COUNT)
    {
        return BSP_NOT_SUPPORTED;
    }

    uint32_t const value = IP_SIUL2->MSCR[mscr];
    cfg.pinCfg           = value & ~static_cast<uint32_t>(BUFFER_MASK);
    if ((value & SIUL2_MSCR_OBE_MASK) != 0U)
    {
        cfg.dir = _OUT;
    }
    else if ((value & SIUL2_MSCR_IBE_MASK) != 0U)
    {
        cfg.dir = _IN;
    }
    else
    {
        cfg.dir = _DISABLED;
    }
    if ((cfg.imcr != NO_IMCR) && (cfg.imcr < SIUL2_IMCR_COUNT))
    {
        cfg.imcrSss = static_cast<uint8_t>(IP_SIUL2->IMCR[cfg.imcr] & SIUL2_IMCR_SSS_MASK);
    }
    return BSP_OK;
}

uint32_t Io::getPinNumber(uint16_t const io)
{
    if (io >= NUMBER_OF_IOS)
    {
        return SIUL2_MSCR_COUNT;
    }
    return mscrIndex(fPinConfiguration[io]);
}

/**
 * Reads the level of an IO: the pad level (GPDI) if the input buffer is
 * enabled, otherwise the output latch (GPDO) of an output.
 */
bool Io::getPin(uint16_t const io)
{
    if (io >= NUMBER_OF_IOS)
    {
        return false;
    }
    uint32_t const mscr = mscrIndex(fPinConfiguration[io]);
    if (mscr >= SIUL2_MSCR_COUNT)
    {
        return false;
    }
    uint32_t const value = IP_SIUL2->MSCR[mscr];
    if ((value & SIUL2_MSCR_IBE_MASK) != 0U)
    {
        return ((*gpdi(mscr)) & 1U) != 0U;
    }
    if ((value & SIUL2_MSCR_OBE_MASK) != 0U)
    {
        return ((*gpdo(mscr)) & 1U) != 0U;
    }
    return false;
}

/**
 * Writes the output latch of an IO configured as output. The latch can be
 * written before the output buffer is enabled, which allows setting the
 * default level of an output glitch free before its pad is configured.
 */
BspReturnCode Io::setPin(uint16_t const io, bool const level)
{
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    uint32_t const mscr = mscrIndex(fPinConfiguration[io]);
    if ((mscr >= SIUL2_MSCR_COUNT) || (_OUT != fPinConfiguration[io].dir))
    {
        return BSP_ERROR;
    }
    *gpdo(mscr) = level ? 1U : 0U;
    return BSP_OK;
}

BspReturnCode Io::resetConfig(uint16_t const io)
{
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    uint32_t const mscr = mscrIndex(fPinConfiguration[io]);
    if (mscr >= SIUL2_MSCR_COUNT)
    {
        return BSP_NOT_SUPPORTED;
    }
    IP_SIUL2->MSCR[mscr] = 0U;
    uint16_t const imcr  = fPinConfiguration[io].imcr;
    if ((imcr != NO_IMCR) && (imcr < SIUL2_IMCR_COUNT))
    {
        IP_SIUL2->IMCR[imcr] = 0U;
    }
    return BSP_OK;
}

} /* namespace bios */
