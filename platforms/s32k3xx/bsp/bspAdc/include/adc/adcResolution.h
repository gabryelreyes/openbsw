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

#include <cstdint>

namespace bios
{
/**
 * Conversion of the raw conversion data register content into an ADC value.
 *
 * The S32K3xx SAR ADC is a 12 bit converter, but it presents its results as
 * 15 bit values in CDATA[14:0] (right aligned, MCR[WLSIDE] = 0). Shifting by
 * 3 gives the 12 bit result.
 */
enum tAdcDataResolution
{
    adcDataShift0,
    adcDataShift3 = 3
};

enum tAdcDataMask
{
    adc12BitResolutionMask = 0xfff,
    adc15BitResolutionMask = 0x7fff
};

template<typename RT, tAdcDataMask resolutionMask, tAdcDataResolution resolution>
class AdcInResolution
{
public:
    using retType = RT;

    enum convertorResolution
    {
        convertResolutionMask = resolutionMask,
        convertShiftParam     = resolution
    };

    retType operator()(uint32_t rawValue) const
    {
        retType ret = static_cast<retType>((rawValue & (convertResolutionMask)) >> resolution);
        return ret;
    }
};

} // namespace bios
