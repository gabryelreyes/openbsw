/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSP_ANALOGINPUT_PIN_CONFIGURATION) && (BSP_ANALOGINPUT_PIN_CONFIGURATION == 1)
/**
 * Configuration of analog signals
 *
 * Order must be the same as the order of the signal IDs defined below.
 *
 *      <ADC device>, <ADC channel>, <pin>, <slot (unused on the S32K3xx)>
 */
AnalogInput::tAnalogConfiguration const ADChannelCfg[AnalogInput::_AdcChannelMax] = {
    {ADC_1_CH, _ADC_S10, Io::EVAL_POTI_ADC, 0U},
};

#else
/**
 * Analog input signal IDs
 */
enum AnalogInputId
{
    AiEVAL_POTI_ADC,

    // end marker
    _AdcChannelMax
};

#endif
