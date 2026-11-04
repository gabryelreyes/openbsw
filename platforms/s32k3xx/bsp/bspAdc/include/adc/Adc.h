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

#include "adc/adcPhysicalName.h"
#include "adc/adcResolution.h"
#include "bsp/Bsp.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

#include <etl/uncopyable.h>

#include <cstdint>

namespace bios
{
/**
 * Driver for one instance of the S32K3xx SAR ADC.
 *
 * The driver calibrates the ADC once at init() and then converts single
 * channels on request: getValueSync() runs a software triggered one-shot
 * normal conversion of exactly one channel and polls for its end.
 *
 * \tparam AdcResolution    conversion of the raw CDATA value (see adcResolution.h)
 * \tparam AdcConfiguration board configuration providing
 *                          - MCR:        static MCR settings (ADCLKSEL, OWREN, AVGEN, ...);
 *                                        PWDN, MODE (one-shot) and NSTART are set by the driver
 *                          - CTR:        sampling time (INPSAMP) for all input groups
 *                          - CALBISTREG: calibration settings (AVG_EN, NR_SMPL, TSAMP)
 *
 * The ADC module clock has to be enabled (MC_ME) before init() is called.
 */
template<typename AdcResolution, typename AdcConfiguration>
class Adc
: private AdcConfiguration
, public ::etl::uncopyable
{
public:
    explicit Adc(ADC_Type& adc)
    : fAdcInResolution(), fAdc(adc), fIsStarted(false), fIsInitialized(false), fIsCalibrated(false)
    {}

    bsp::BspReturnCode init();
    bsp::BspReturnCode start();
    bsp::BspReturnCode stop();
    bsp::BspReturnCode getValueSync(uint8_t channel, uint16_t& value) const;

    /**
     * \return true if the calibration at init() succeeded
     */
    bool isCalibrated() const { return fIsCalibrated; }

    /**
     * \return true if \p channel is a precision or standard input of the ADC
     */
    static bool isValidChannel(uint8_t channel);

private:
    // Polling loop limits: power up, calibration (init only) and conversion
    static uint32_t const TIMEOUT             = 1000000U;
    static uint32_t const CALIBRATION_TIMEOUT = 10000000U;
    static uint32_t const CONVERSION_TIMEOUT  = 10000U;

    // MSR[ADCSTATUS] values
    static uint32_t const ADCSTATUS_IDLE       = 0U;
    static uint32_t const ADCSTATUS_POWER_DOWN = 1U;

    static uint32_t const GROUP_PRECISION = 0U;
    static uint32_t const GROUP_STANDARD  = 1U;

    bool waitForStatus(uint32_t status) const;
    bool calibrate();

    uint32_t volatile& ceocfr(uint32_t group) const
    {
        return (group == GROUP_PRECISION) ? fAdc.CEOCFR0 : fAdc.CEOCFR1;
    }

    uint32_t volatile& ncmr(uint32_t group) const
    {
        return (group == GROUP_PRECISION) ? fAdc.NCMR0 : fAdc.NCMR1;
    }

    uint32_t data(uint32_t group, uint32_t bit) const
    {
        return (group == GROUP_PRECISION) ? fAdc.PCDR[bit] : fAdc.ICDR[bit];
    }

    AdcResolution fAdcInResolution;
    ADC_Type& fAdc;
    bool fIsStarted;
    bool fIsInitialized;
    bool fIsCalibrated;
};

template<typename AdcResolution, typename AdcConfiguration>
bool Adc<AdcResolution, AdcConfiguration>::isValidChannel(uint8_t const channel)
{
    uint32_t const group = static_cast<uint32_t>(channel) >> 5U;
    uint32_t const bit   = static_cast<uint32_t>(channel) & 0x1FU;
    if (group == GROUP_PRECISION)
    {
        return bit < ADC_CDR1_COUNT;
    }
    if (group == GROUP_STANDARD)
    {
        return bit < ADC_CDR2_COUNT;
    }
    return false;
}

template<typename AdcResolution, typename AdcConfiguration>
bool Adc<AdcResolution, AdcConfiguration>::waitForStatus(uint32_t const status) const
{
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((fAdc.MSR & ADC_MSR_ADCSTATUS_MASK) == ADC_MSR_ADCSTATUS(status))
        {
            return true;
        }
    }
    return false;
}

/**
 * Runs the built-in calibration (see the S32K3xx Reference Manual, ADC
 * calibration): the calibration settings are written to CALBISTREG,
 * CALBISTREG[TEST_EN] starts the calibration, CALBISTREG[C_T_BUSY] signals
 * that it is running and MSR[CALIBRTD] / CALBISTREG[TEST_FAIL] report the
 * result.
 */
template<typename AdcResolution, typename AdcConfiguration>
bool Adc<AdcResolution, AdcConfiguration>::calibrate()
{
    uint32_t calbistreg = fAdc.CALBISTREG;
    calbistreg &= ~(
        ADC_CALBISTREG_AVG_EN_MASK | ADC_CALBISTREG_NR_SMPL_MASK | ADC_CALBISTREG_TSAMP_MASK
        | ADC_CALBISTREG_TEST_EN_MASK);
    calbistreg |= static_cast<uint32_t>(AdcConfiguration::CALBISTREG);
    fAdc.CALBISTREG = calbistreg | ADC_CALBISTREG_TEST_EN_MASK;

    for (uint32_t timeout = CALIBRATION_TIMEOUT; timeout > 0U; --timeout)
    {
        uint32_t const status = fAdc.CALBISTREG;
        if ((status & ADC_CALBISTREG_C_T_BUSY_MASK) == 0U)
        {
            if ((status & ADC_CALBISTREG_TEST_FAIL_MASK) != 0U)
            {
                return false;
            }
            if ((fAdc.MSR & ADC_MSR_CALIBRTD_MASK) != 0U)
            {
                return true;
            }
        }
    }
    return false;
}

template<typename AdcResolution, typename AdcConfiguration>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration>::init()
{
    fIsStarted     = false;
    fIsInitialized = false;

    // The ADC clock prescaler (MCR[ADCLKSEL]) may only be changed in power down.
    uint32_t const mcr = static_cast<uint32_t>(AdcConfiguration::MCR)
                         & ~(ADC_MCR_PWDN_MASK | ADC_MCR_MODE_MASK | ADC_MCR_NSTART_MASK);
    fAdc.MCR = mcr | ADC_MCR_PWDN_MASK;
    (void)waitForStatus(ADCSTATUS_POWER_DOWN);
    fAdc.MCR = mcr;
    if (!waitForStatus(ADCSTATUS_IDLE))
    {
        return bsp::BSP_ERROR;
    }

    fIsCalibrated = calibrate();

    // No interrupts, no DMA, no presampling, no channel enabled.
    fAdc.IMR   = 0U;
    fAdc.CIMR0 = 0U;
    fAdc.CIMR1 = 0U;
    fAdc.CIMR2 = 0U;
    fAdc.DMAE  = 0U;
    fAdc.PSR0  = 0U;
    fAdc.PSR1  = 0U;
    fAdc.PSR2  = 0U;
    fAdc.NCMR0 = 0U;
    fAdc.NCMR1 = 0U;
    fAdc.NCMR2 = 0U;
    fAdc.JCMR0 = 0U;
    fAdc.JCMR1 = 0U;
    fAdc.JCMR2 = 0U;

    fAdc.CTR0 = static_cast<uint32_t>(AdcConfiguration::CTR);
    fAdc.CTR1 = static_cast<uint32_t>(AdcConfiguration::CTR);
    fAdc.CTR2 = static_cast<uint32_t>(AdcConfiguration::CTR);

    // Clear all pending flags (write 1 to clear).
    fAdc.ISR = ADC_ISR_ECH_MASK | ADC_ISR_EOC_MASK | ADC_ISR_JECH_MASK | ADC_ISR_JEOC_MASK
               | ADC_ISR_EOBCTU_MASK;
    fAdc.CEOCFR0 = 0xFFFFFFFFU;
    fAdc.CEOCFR1 = 0xFFFFFFFFU;
    fAdc.CEOCFR2 = 0xFFFFFFFFU;

    fIsInitialized = true;
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration>::start()
{
    if ((false == fIsStarted) && (true == fIsInitialized))
    {
        fIsStarted = true;
    }
    return fIsStarted ? bsp::BSP_OK : bsp::BSP_ERROR;
}

template<typename AdcResolution, typename AdcConfiguration>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration>::stop()
{
    fIsStarted     = false;
    fIsInitialized = false;
    fAdc.NCMR0     = 0U;
    fAdc.NCMR1     = 0U;
    fAdc.NCMR2     = 0U;
    fAdc.MCR       = fAdc.MCR | ADC_MCR_PWDN_MASK;
    return bsp::BSP_OK;
}

/**
 * Converts \p channel (software triggered one-shot normal conversion) and
 * waits for the result. Interrupts are suspended for the few microseconds of
 * the conversion so that callers from different tasks cannot interleave.
 */
template<typename AdcResolution, typename AdcConfiguration>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration>::getValueSync(uint8_t const channel, uint16_t& value) const
{
    if ((false == fIsInitialized) || (false == fIsStarted) || (!isValidChannel(channel)))
    {
        return bsp::BSP_ERROR;
    }
    uint32_t const group = static_cast<uint32_t>(channel) >> 5U;
    uint32_t const bit   = static_cast<uint32_t>(channel) & 0x1FU;
    uint32_t const mask  = 1UL << bit;

    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;

    if ((fAdc.MSR & (ADC_MSR_NSTART_MASK | ADC_MSR_JSTART_MASK)) != 0U)
    {
        return bsp::BSP_ERROR;
    }

    fAdc.NCMR0    = 0U;
    fAdc.NCMR1    = 0U;
    fAdc.NCMR2    = 0U;
    ncmr(group)   = mask;
    fAdc.ISR      = ADC_ISR_ECH_MASK | ADC_ISR_EOC_MASK;
    ceocfr(group) = mask;
    // one-shot mode (MCR[MODE] = 0), start the normal conversion chain
    fAdc.MCR      = (fAdc.MCR & ~ADC_MCR_MODE_MASK) | ADC_MCR_NSTART_MASK;

    bool converted = false;
    for (uint32_t timeout = CONVERSION_TIMEOUT; timeout > 0U; --timeout)
    {
        if (((ceocfr(group) & mask) != 0U) && ((fAdc.ISR & ADC_ISR_ECH_MASK) != 0U))
        {
            converted = true;
            break;
        }
    }
    uint32_t const raw = data(group, bit);
    ceocfr(group)      = mask;
    fAdc.ISR           = ADC_ISR_ECH_MASK | ADC_ISR_EOC_MASK;
    ncmr(group)        = 0U;

    if ((!converted) || ((raw & ADC_PCDR_VALID_MASK) == 0U))
    {
        return bsp::BSP_ERROR;
    }
    value = fAdcInResolution(raw & ADC_PCDR_CDATA_MASK);
    return bsp::BSP_OK;
}

} // namespace bios
