/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/adc/AnalogInput.h"

#include "bsp/adc/AnalogInputScale.h"
#include "io/Io.h"

#ifdef BSP_ANALOGINPUT_PIN_CONFIGURATION
#undef BSP_ANALOGINPUT_PIN_CONFIGURATION
#endif
#define BSP_ANALOGINPUT_PIN_CONFIGURATION 1

namespace bios
{

#include "bsp/adc/analogInputConfiguration.h"
#include "bsp/adc/analogInputConfigurationStrings.h"
#include "bsp/adc/analogInputScaleConfiguration.h"

tAdc12Bit AnalogInput::fAdc1(*IP_ADC_1);

AnalogInput::tAnalogConfiguration const* AnalogInput::fADChannelCfg = nullptr;

AnalogInput::AnalogInput() { fADChannelCfg = &ADChannelCfg[0]; }

void AnalogInput::init()
{
    AnalogInputScale::init(
        static_cast<uint8_t>(
            sizeof(analogInputScaleConfiguration) / sizeof(AnalogInputScale::scale)),
        &analogInputScaleConfiguration[0]);

    (void)fAdc1.init();

    for (uint16_t i = 0; i < static_cast<uint16_t>(_AdcChannelMax); ++i)
    {
        // analog pads: input and output buffers off
        (void)Io::setDefaultConfiguration(fADChannelCfg[i].pin);
    }
}

void AnalogInput::start() { (void)fAdc1.start(); }

bsp::BspReturnCode AnalogInput::get(uint16_t const channel, uint16_t& value)
{
    bsp::BspReturnCode ret = bsp::BSP_ERROR;

    if ((nullptr != fADChannelCfg) && (channel < AdcCountAll)
        && (fADChannelCfg[channel].adc == ADC_1_CH))
    {
        ret = fAdc1.getValueSync(fADChannelCfg[channel].phChannel, value);
    }

    return ret;
}

bsp::BspReturnCode AnalogInput::get(uint16_t const channel, uint32_t& value)
{
    uint16_t value16             = 0U;
    bsp::BspReturnCode const ret = get(channel, value16);
    value                        = static_cast<uint32_t>(value16);
    return ret;
}

bool AnalogInput::isCalibrated() { return fAdc1.isCalibrated(); }

void AnalogInput::cyclic() {}

void AnalogInput::stop() { (void)fAdc1.stop(); }

char const* AnalogInput::getName(uint16_t const channel)
{
    if (channel < sizeof(adcChannelsNamingStrings) / (sizeof(char*)))
    {
        return adcChannelsNamingStrings[channel];
    }
    else
    {
        return ("??? ->");
    }
}

} // namespace bios
