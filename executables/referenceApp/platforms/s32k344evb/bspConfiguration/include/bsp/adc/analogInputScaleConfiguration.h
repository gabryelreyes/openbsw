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

#include "bsp/adc/AnalogInputScale.h"

/**
 * Scaling of the 12 bit ADC values to mV, assuming VREFH = 5 V (VDD_HV_A of
 * the S32K3X4EVB-T172 in its 5 V setting). Adjust the full scale value if the
 * board supplies VDD_HV_A with 3.3 V.
 */
uint32_t const precisionFactor                                = 1000U;
uint32_t const _5_0Volt12BitResolution                        = 5000U * precisionFactor / 4095U;
AnalogInputScale::scale const analogInputScaleConfiguration[] = {
    // clang-format off
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, // AiEVAL_POTI_ADC
    // clang-format on
};
