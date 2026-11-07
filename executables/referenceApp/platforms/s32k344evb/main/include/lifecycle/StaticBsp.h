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

#include "bsp/adc/AnalogInput.h"
#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "inputManager/DigitalInput.h"
#include "mcu/mcu.h"
#include "outputManager/Output.h"

namespace bios
{

/**
 * CAN phy of the S32K3X4EVB-T172 (board revision B2): a TJA1443 high-speed
 * CAN FD transceiver on CAN0, controlled via two GPIOs (PTC20 = CAN0_STB,
 * PTC21 = CAN0_EN). Driving both pins high puts the transceiver into normal
 * mode, driving both pins low into standby mode. The TJA1443 needs no
 * further configuration.
 */
class CanPhyTja1443 : public CanPhy
{
public:
    CanPhyTja1443() = default;

    void init(uint32_t /* id */) override
    {
        // PTC20 (MSCR[84]) and PTC21 (MSCR[85]) as GPIO outputs
        IP_SIUL2->MSCR[84] = SIUL2_MSCR_OBE_MASK;
        IP_SIUL2->MSCR[85] = SIUL2_MSCR_OBE_MASK;
        (void)setMode(CAN_PHY_MODE_STANDBY, 0U);
    }

    bool setMode(Mode const mode, uint32_t /* id */) override
    {
        switch (mode)
        {
            case CAN_PHY_MODE_STANDBY:
            {
                IP_SIUL2->GPDO84 = 0U; // CAN0_STB low
                IP_SIUL2->GPDO85 = 0U; // CAN0_EN low
                break;
            }
            case CAN_PHY_MODE_ACTIVE:
            {
                IP_SIUL2->GPDO84 = 1U; // CAN0_STB high
                IP_SIUL2->GPDO85 = 1U; // CAN0_EN high
                break;
            }
            default:
            {
                return false;
            }
        }
        fMode = mode;
        return true;
    }

    ErrorCode getPhyErrorStatus(uint32_t /* id */) override { return CAN_PHY_ERROR_NONE; }
};

} // namespace bios

class StaticBsp
{
public:
    StaticBsp() {}

    void init();

    void cyclic();

    bios::CanPhy& getCanPhy() { return _canPhy; }

    bios::AnalogInput& getAdc() { return _adc; }

    bios::IEcuPowerStateController& getPowerStateController() { return _powerStateController; }

private:
    class DummyEcuPowerStateController : public bios::IEcuPowerStateController
    {
    public:
        void startPreSleep() override {}

        uint32_t powerDown(uint8_t /* mode */, tCheckWakeupDelegate /* delegate */) override
        {
            return 0;
        }

        uint32_t powerDown(uint8_t /* mode */) override { return 0; }

        void fullPowerUp() override {}

        void setWakeupSourceMonitoring(
            uint32_t /* source */, bool /* active */ = true, bool /* fallingEdge */ = true) override
        {}

        void clearWakeupSourceMonitoring(uint32_t /* source */) override {}

        bool setWakeupDelegate(tCheckWakeupDelegate& /* delegate */) override { return true; }

        uint32_t getWakeupSource(void) override { return 0; }
    };

    bios::CanPhyTja1443 _canPhy;
    DummyEcuPowerStateController _powerStateController;
    bios::Output _output;
    bios::DigitalInput _digitalInput;
    bios::AnalogInput _adc;
};
