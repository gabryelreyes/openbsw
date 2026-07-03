/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

extern "C"
{
extern void call_can_isr_RX();
extern void call_can_isr_TX();

/* FlexCAN0_0_IRQn (109): bus off / errors. Not enabled - the bus off state is
 * polled by the transceiver's cyclic task. */
void CAN0_Bus_Off_Error_IRQHandler() {}

/* FlexCAN0_1_IRQn (110): message buffers 0-31. Unlike on the S32K148, the
 * receive (MB 0-15) and transmit (MB 16-31) buffers share this single
 * interrupt line; call_can_isr_TX only acts on a pending transmit interrupt. */
void CAN0_ORed_0_31_MB_IRQHandler()
{
    call_can_isr_TX();
    call_can_isr_RX();
}

/* FlexCAN0_2_IRQn (111) / FlexCAN0_3_IRQn (112): message buffers 32-63 and
 * 64-95. Not used by the driver, never enabled. */
void CAN0_ORed_32_63_MB_IRQHandler() {}

void CAN0_ORed_64_95_MB_IRQHandler() {}

} /* extern "C" */
