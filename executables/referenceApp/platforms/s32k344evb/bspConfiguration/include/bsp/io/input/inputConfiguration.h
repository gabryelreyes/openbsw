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

#if defined(BSP_INPUT_PIN_CONFIGURATION) && (BSP_INPUT_PIN_CONFIGURATION == 1)
DigitalInput::InputConfiguration const
    DigitalInput::sfDigitalInputConfigurations[][DigitalInput::NUMBER_OF_INTERNAL_DIGITAL_INPUTS]
    = {
        {
            /*  0 */ {Io::EVAL_SW5, Io::HIGH_ACTIVE, 1},
            /*  1 */ {Io::EVAL_SW6, Io::HIGH_ACTIVE, 1},
        },
};

DigitalInput::InputConfiguration const*
DigitalInput::getConfiguration(uint8_t /* hardwareVersion */)
{
    return &sfDigitalInputConfigurations[0][0];
}

#else

enum DigitalInputId
{
    /*  0 */ EVAL_SW5,
    /*  1 */ EVAL_SW6,
    // other internal inputs go here
    //       update LAST_INTERNAL_DIGITAL_INPUT when adding a new internal input
    LAST_INTERNAL_DIGITAL_INPUT = EVAL_SW6,
    // dynamic inputs go here
    //       update LAST_DYNAMIC_DIGITAL_INPUT when adding a new external input
    LAST_DYNAMIC_DIGITAL_INPUT  = LAST_INTERNAL_DIGITAL_INPUT,
    PORT_UNAVAILABLE,
    // Name of the user button used by the generic DemoSystem (SW3 on the S32K148EVB)
    EVAL_SW3 = EVAL_SW5
};
#endif /* #if (BSP_INPUT_PIN_CONFIGURATION == 1) */
