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

#include "can/transceiver/canflex2/CanFlex2Transceiver.h"
#include "lifecycle/SingleContextLifecycleComponent.h"

#include <etl/singleton_base.h>
#include <systems/ICanSystem.h>

namespace bios
{
class IEcuPowerStateController;
}

extern "C"
{
/**
 * This is a C-style function that handles the CAN receive interrupt service routine (ISR).
 *
 * It enters the ISR group for CAN, locks the async task and calls the receiveInterrupt method on
 * CanFlex2Transceiver. If any frames are received, it dispatches the RX task. Finally, it leaves
 * the ISR group for CAN.
 */
extern void call_can_isr_RX();

/**
 * This is a C-style function that handles the CAN transmit interrupt service routine (ISR).
 *
 * It enters the ISR group for CAN, calls the transmitInterrupt method on CanFlex2Transceiver and
 * then leaves the ISR group for CAN. On the S32K344 the receive and transmit message buffers
 * share one interrupt line, so the transmit ISR is only run if a transmit interrupt is actually
 * pending.
 */
extern void call_can_isr_TX();
}

class StaticBsp;

namespace systems
{

class CanSystem
: public ::can::ICanSystem
, public ::lifecycle::SingleContextLifecycleComponent
, public ::etl::singleton_base<CanSystem>
{
public:
    /**
     * \param context The context in which the CanSystem will run which is unit8_t.
     * \param staticBsp The StaticBsp object which allows us to create the instance of
     *        CanFlex2Transceiver which is responsible for establishing the CAN connection.
     */
    CanSystem(::async::ContextType const context, StaticBsp& staticBsp);
    CanSystem(CanSystem const&)            = delete;
    CanSystem& operator=(CanSystem const&) = delete;

    /**
     * Initializes the CanSystem.
     * Invoke the transitionDone method which will update lifecycle manager that the component has
     * completed its transition.
     */
    void init() override;

    /**
     * Runs the CanSystem.
     * Enables CanRxRunnable, initialize and open CanFlex2Transceiver which setup and open CAN
     * connection, then invoke transitionDone to update lifecycle manager that the component has
     * completed its transition.
     */
    void run() override;

    /**
     * Shutdowns the CanSystem.
     * Close and shutdowns CanFlex2Transceiver which will cancel the CAN connection, disables
     * CanRxRunnable and then invoke transitionDone to update lifecycle manager that the component
     * has completed its transition.
     */
    void shutdown() override;

    /**
     * Checks if the given busId is equal to CAN_0. If it is returns a reference else nullptr.
     *
     * \param busId BusId.
     * \return A non-const reference to the CanFlex2Transceiver object.
     */
    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    /**
     * Executes the CanRxRunnable in the given context.
     */
    void dispatchRxTask();

    class CanRxRunnable : public ::async::RunnableType
    {
    public:
        /**
         * \param parent The CanSystem object that owns the CanRxRunnable.
         */
        CanRxRunnable(CanSystem& parent);

        /**
         * This method will be called when CAN frames are received.
         */
        void execute() override;

        /**
         * Sets the enabled state of the CanRxRunnable.
         *
         * \param enabled The state to set.
         */
        void setEnabled(bool enabled) { _enabled = enabled; }

    private:
        CanSystem& _parent;
        bool _enabled;
    };

private:
    ::async::ContextType _context;
    bios::CanFlex2Transceiver _transceiver0;
    CanRxRunnable _canRxRunnable;
};

} // namespace systems
