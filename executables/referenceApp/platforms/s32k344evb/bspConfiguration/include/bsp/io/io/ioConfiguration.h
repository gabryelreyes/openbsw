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

#if defined(BSP_IO_PIN_CONFIGURATION) && (BSP_IO_PIN_CONFIGURATION == 1)

/**
 * Pad configuration of the IOs of the S32K3X4EVB-T172, same order as the IO
 * IDs below.
 *
 *      <port>, <pin>, <direction>, <IMCR source select>, <IMCR>, <MSCR settings>
 */
Io::PinConfiguration const Io::fPinConfiguration[Io::NUMBER_OF_IOS] = {
    // clang-format off
    /* 0 */ {_PORTA_, PIN29, _OUT, 0U, NO_IMCR, GPIO},
    /* 1 */ {_PORTA_, PIN30, _OUT, 0U, NO_IMCR, GPIO},
    /* 2 */ {_PORTA_, PIN31, _OUT, 0U, NO_IMCR, GPIO},
    /* 3 */ {_PORTB_, PIN26, _IN, 0U, NO_IMCR, GPIO},
    /* 4 */ {_PORTB_, PIN19, _IN, 0U, NO_IMCR, GPIO},
    /* 5 */ {_PORTA_, PIN11, _DISABLED, 0U, NO_IMCR, PINDISABLE},
    // clang-format on
};

#else

/**
 * IO IDs. Only the pads used through the IO driver are listed. The pads of
 * the terminal UART (PTA15/PTA16), CAN0 (PTA6/PTA7) and the CAN transceiver
 * control (PTC20/PTC21) are still configured directly by their drivers.
 */
enum PinId
{
    /* 0 */ EVAL_LED_RED,   // PTA29, RGB LED red, active high
    /* 1 */ EVAL_LED_GREEN, // PTA30, RGB LED green, active high
    /* 2 */ EVAL_LED_BLUE,  // PTA31, RGB LED blue, active high
    /* 3 */ EVAL_SW5,       // PTB26, user button SW5
    /* 4 */ EVAL_SW6,       // PTB19, user button SW6
    /* 5 */ EVAL_POTI_ADC,  // PTA11 = ADC1_S10, potentiometer (analog, digital buffers off)

    /* xx */ NUMBER_OF_INPUTS_AND_OUTPUTS,
    /* xx */ PORT_UNAVAILABLE = NUMBER_OF_INPUTS_AND_OUTPUTS,
};

#endif /* BSP_IO_PIN_CONFIGURATION == 1 */
