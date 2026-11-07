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

/**
 * PWM outputs. There is no PWM driver (eMIOS) for the S32K344 yet: the LED
 * PWM channels used by the generic DemoSystem are dynamic channels without a
 * client, so setting their duty cycle has no effect (BSP_ERROR). The LEDs
 * are plain digital outputs (see outputConfiguration.h).
 */
enum outputPwm
{
    _pwmStaticStart = 0,
    dummyPwm        = _pwmStaticStart,
    _pwmStaticEnd,
    _pwmDynamicStart = _pwmStaticEnd

    ,
    EVAL_LED_GREEN_PWM = _pwmDynamicStart,
    EVAL_LED_BLUE_PWM

    ,
    _pwmDynamicMark,
    _pwmChannelMax   = _pwmDynamicMark,
    PORT_UNAVAILABLE = _pwmChannelMax
};
