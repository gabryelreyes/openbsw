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

namespace bios
{

/**
 * Channel numbers of the S32K3xx SAR ADC (see the S32K3xx Reference Manual,
 * ADC, and the S32K344 IO signal description table for the pad mapping):
 *  - 0 .. 7:   precision inputs P0 .. P7 (CTR0, NCMR0, CEOCFR0, PCDR)
 *  - 32 .. 55: standard inputs S0 .. S23 (CTR1, NCMR1, CEOCFR1, ICDR)
 *  - 64 .. 95: external (multiplexed) inputs, not supported by the driver yet
 */
enum
{
    _ADC_P0 = 0,
    _ADC_P1,
    _ADC_P2,
    _ADC_P3,
    _ADC_P4,
    _ADC_P5,
    _ADC_P6,
    _ADC_P7,
    _ADC_S0 = 32,
    _ADC_S1,
    _ADC_S2,
    _ADC_S3,
    _ADC_S4,
    _ADC_S5,
    _ADC_S6,
    _ADC_S7,
    _ADC_S8,
    _ADC_S9,
    _ADC_S10,
    _ADC_S11,
    _ADC_S12,
    _ADC_S13,
    _ADC_S14,
    _ADC_S15,
    _ADC_S16,
    _ADC_S17,
    _ADC_S18,
    _ADC_S19,
    _ADC_S20,
    _ADC_S21,
    _ADC_S22,
    _ADC_S23
};

} // namespace bios
