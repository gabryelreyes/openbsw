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

#include "adc/Adc.h"

namespace bios
{
static uint8_t const ADC_1_CH = 1U;

/**
 * ADC configuration (see adc/Adc.h)
 */
class Adc12BitConfiguration
{
public:
    enum : uint32_t
    {
        // ADC clock = module clock / 4, conversion data overwrite enabled,
        // right aligned data, no hardware averaging
        MCR = ADC_MCR_ADCLKSEL(2U) | ADC_MCR_OWREN_MASK,
        // sampling time: 128 ADC clock cycles
        CTR = ADC_CTR0_INPSAMP(128U),
        // calibration: averaging over 512 samples, 22 cycles sampling time
        CALBISTREG
        = ADC_CALBISTREG_AVG_EN_MASK | ADC_CALBISTREG_NR_SMPL(3U) | ADC_CALBISTREG_TSAMP(0U),
    };
};

typedef AdcInResolution<uint16_t, adc15BitResolutionMask, adcDataShift3> tAdc12BitConvertor;

typedef Adc<tAdc12BitConvertor, Adc12BitConfiguration> tAdc12Bit;

class AnalogInput
{
public:
    AnalogInput();

#undef BSP_ANALOGINPUT_PIN_CONFIGURATION
#include "bsp/adc/analogInputConfiguration.h"

    typedef struct
    {
        uint8_t adc;
        uint8_t phChannel;
        uint8_t pin;
        uint8_t slot;
    } tAnalogConfiguration;

    void init();
    void start();

    static bsp::BspReturnCode get(uint16_t channel, uint16_t& value);
    static bsp::BspReturnCode get(uint16_t channel, uint32_t& value);

    static char const* getName(uint16_t channel);

    /**
     * \return true if the calibration of the ADC succeeded
     */
    static bool isCalibrated();

    void stop();

    void cyclic();

    static uint16_t const AdcCountAll = static_cast<uint16_t>(_AdcChannelMax);

private:
    static tAdc12Bit fAdc1;

    static tAnalogConfiguration const* fADChannelCfg;
};

} // namespace bios
