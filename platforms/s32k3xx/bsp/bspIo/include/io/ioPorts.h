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

/**
 * Ports and pin numbers of the S32K3xx SIUL2.
 *
 * The SIUL2 numbers its pads linearly: the MSCR (and GPDO/GPDI) index of a pad
 * is port * 32 + pin, e.g. PTA29 = 29, PTB26 = 58, PTC20 = 84.
 */
enum IOPort
{
    _PORTA_     = 0,
    _PORTB_     = 1,
    _PORTC_     = 2,
    _PORTD_     = 3,
    _PORTE_     = 4,
    _PORTF_     = 5,
    _PORTG_     = 6,
    _PORTS_MAX_ = 7,
};

enum IOPin
{
    PIN0 = 0,
    PIN1,
    PIN2,
    PIN3,
    PIN4,
    PIN5,
    PIN6,
    PIN7,
    PIN8,
    PIN9,
    PIN10,
    PIN11,
    PIN12,
    PIN13,
    PIN14,
    PIN15,
    PIN16,
    PIN17,
    PIN18,
    PIN19,
    PIN20,
    PIN21,
    PIN22,
    PIN23,
    PIN24,
    PIN25,
    PIN26,
    PIN27,
    PIN28,
    PIN29,
    PIN30,
    PIN31,
    PINS_PER_PORT = 32,
};
