/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "clock/clockConfig.h"

#include "mcu/mcu.h"

namespace
{
uint32_t const MC_ME_KEY          = 0x5AF0U;
uint32_t const MC_ME_INVERTED_KEY = 0xA50FU;

// MC_ME PRTN1 COFB block assignments (see S32K3xx Reference Manual, MC_ME)
uint32_t const PRTN1_COFB0_REQ24_MSCM    = (1U << 24);
uint32_t const PRTN1_COFB1_REQ53_FXOSC   = (1U << 21);
uint32_t const PRTN1_COFB1_REQ56_PLL     = (1U << 24);
uint32_t const PRTN1_COFB2_REQ80_LPUART6 = (1U << 16);

// PLL configuration: FXOSC 16 MHz / RDIV 4 * MFI 240 = 960 MHz VCO,
// / ODIV2 2 = 480 MHz PLL_CLK, / PHI divider 3 = 160 MHz PLL_PHI0.
uint32_t const PLL_RDIV     = 4U;
uint32_t const PLL_MFI      = 240U;
uint32_t const PLL_ODIV2    = 2U;
uint32_t const PLL_PHI0_DIV = 3U;
uint32_t const PLL_PHI1_DIV = 3U;

// FXOSC settings for the 16 MHz crystal on the S32K3X4EVB
uint32_t const FXOSC_EOCV   = 157U;
uint32_t const FXOSC_GM_SEL = 12U; // 0.7016x

uint32_t const TIMEOUT = 100000U;

/**
 * Commits a PRTN1 COFB clock enable change and waits until the hardware
 * process has completed.
 */
void mcMeCommitPartition1(uint32_t volatile const& statusRegister, uint32_t const mask)
{
    IP_MC_ME->PRTN1_PCONF = MC_ME_PRTN1_PCONF_PCE_MASK;
    IP_MC_ME->PRTN1_PUPD  = MC_ME_PRTN1_PUPD_PCUD_MASK;
    IP_MC_ME->CTL_KEY     = MC_ME_KEY;
    IP_MC_ME->CTL_KEY     = MC_ME_INVERTED_KEY;

    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((statusRegister & mask) != 0U)
        {
            break;
        }
    }
}

/**
 * Triggers a MUX_0 divider update and clock switch and waits for completion.
 */
void mux0Transition()
{
    IP_MC_CGM->MUX_0_DIV_TRIG = 1U;
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((IP_MC_CGM->MUX_0_DIV_UPD_STAT & MC_CGM_MUX_0_DIV_UPD_STAT_DIV_STAT_MASK) == 0U)
        {
            break;
        }
    }
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((IP_MC_CGM->MUX_0_CSS & MC_CGM_MUX_0_CSS_SWIP_MASK) == 0U)
        {
            break;
        }
    }
    IP_MC_CGM->MUX_0_CSC = IP_MC_CGM->MUX_0_CSC | MC_CGM_MUX_0_CSC_CLK_SW_MASK;
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((IP_MC_CGM->MUX_0_CSS & MC_CGM_MUX_0_CSS_CLK_SW_MASK) != 0U)
        {
            break;
        }
    }
}
} // namespace

extern "C" void configurePll(void)
{
    // Enable the MSCM and PLL peripheral interfaces.
    IP_MC_ME->PRTN1_COFB0_CLKEN = IP_MC_ME->PRTN1_COFB0_CLKEN | PRTN1_COFB0_REQ24_MSCM;
    IP_MC_ME->PRTN1_COFB1_CLKEN
        = IP_MC_ME->PRTN1_COFB1_CLKEN | PRTN1_COFB1_REQ53_FXOSC | PRTN1_COFB1_REQ56_PLL;
    // Peripheral clocks used by the BSP: LPUART6 (terminal).
    IP_MC_ME->PRTN1_COFB2_CLKEN = IP_MC_ME->PRTN1_COFB2_CLKEN | PRTN1_COFB2_REQ80_LPUART6;
    mcMeCommitPartition1(IP_MC_ME->PRTN1_COFB1_STAT, PRTN1_COFB1_REQ53_FXOSC);

    // Run MUX_0 from FIRC with safe dividers while the PLL is (re)configured.
    IP_MC_CGM->MUX_0_DIV_TRIG_CTRL
        = MC_CGM_MUX_0_DIV_TRIG_CTRL_TCTL_MASK | MC_CGM_MUX_0_DIV_TRIG_CTRL_HHEN_MASK;
    IP_MC_CGM->MUX_0_DC_0 = MC_CGM_MUX_0_DC_0_DE_MASK | MC_CGM_MUX_0_DC_0_DIV(0U); // CORE
    IP_MC_CGM->MUX_0_DC_1 = MC_CGM_MUX_0_DC_1_DE_MASK | MC_CGM_MUX_0_DC_1_DIV(0U); // AIPS_PLAT
    IP_MC_CGM->MUX_0_DC_2 = MC_CGM_MUX_0_DC_2_DE_MASK | MC_CGM_MUX_0_DC_2_DIV(2U); // AIPS_SLOW
    IP_MC_CGM->MUX_0_DC_3 = MC_CGM_MUX_0_DC_3_DE_MASK | MC_CGM_MUX_0_DC_3_DIV(0U); // HSE
    IP_MC_CGM->MUX_0_DC_4 = MC_CGM_MUX_0_DC_4_DE_MASK | MC_CGM_MUX_0_DC_4_DIV(0U); // DCM
    IP_MC_CGM->MUX_0_DC_5 = MC_CGM_MUX_0_DC_5_DE_MASK | MC_CGM_MUX_0_DC_5_DIV(0U); // LBIST
    IP_MC_CGM->MUX_0_CSC  = MC_CGM_MUX_0_CSC_SELCTL(0U) | MC_CGM_MUX_0_CSC_CLK_SW_MASK; // FIRC
    mux0Transition();

    // Enable the fast external oscillator (16 MHz crystal).
    uint32_t fxoscCtrl = IP_FXOSC->CTRL;
    fxoscCtrl &= ~FXOSC_CTRL_OSC_BYP_MASK;
    fxoscCtrl |= FXOSC_CTRL_COMP_EN_MASK | FXOSC_CTRL_EOCV(FXOSC_EOCV)
                 | FXOSC_CTRL_GM_SEL(FXOSC_GM_SEL) | FXOSC_CTRL_OSCON_MASK;
    IP_FXOSC->CTRL = fxoscCtrl;
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((IP_FXOSC->STAT & FXOSC_STAT_OSC_STAT_MASK) != 0U)
        {
            break;
        }
    }

    // Configure and lock the PLL.
    IP_PLL->PLLDV
        = PLL_PLLDV_RDIV(PLL_RDIV) | PLL_PLLDV_MFI(PLL_MFI) | PLL_PLLDV_ODIV2(PLL_ODIV2);
    IP_PLL->PLLFD = IP_PLL->PLLFD & ~(PLL_PLLFD_SDMEN_MASK | PLL_PLLFD_SDM3_MASK);
    uint32_t pllFm = IP_PLL->PLLFM;
    pllFm &= ~(PLL_PLLFM_STEPSIZE_MASK | PLL_PLLFM_STEPNO_MASK);
    pllFm |= PLL_PLLFM_SSCGBYP_MASK | PLL_PLLFM_SPREADCTL_MASK;
    IP_PLL->PLLFM    = pllFm;
    IP_PLL->PLLODIV[0]
        = PLL_PLLODIV_DIV(PLL_PHI0_DIV - 1U) | PLL_PLLODIV_DE_MASK;
    IP_PLL->PLLODIV[1]
        = PLL_PLLODIV_DIV(PLL_PHI1_DIV - 1U) | PLL_PLLODIV_DE_MASK;
    IP_PLL->PLLCR = 0U; // power up
    for (uint32_t timeout = TIMEOUT; timeout > 0U; --timeout)
    {
        if ((IP_PLL->PLLSR & PLL_PLLSR_LOCK_MASK) != 0U)
        {
            break;
        }
    }

    // Switch MUX_0 to PLL_PHI0 with the final dividers.
    //
    // The C40 flash read wait states (FLASH.CTL[RWSC]) are left at their
    // conservative reset default, which is safe at 160 MHz. Tuning RWSC (4
    // wait states at 160 MHz) requires executing the change from RAM and is
    // deferred (see doc/dev/s32k344_integration_plan.md).
    IP_MC_CGM->MUX_0_DC_0 = MC_CGM_MUX_0_DC_0_DE_MASK | MC_CGM_MUX_0_DC_0_DIV(0U); // 160 MHz
    IP_MC_CGM->MUX_0_DC_1 = MC_CGM_MUX_0_DC_1_DE_MASK | MC_CGM_MUX_0_DC_1_DIV(1U); //  80 MHz
    IP_MC_CGM->MUX_0_DC_2 = MC_CGM_MUX_0_DC_2_DE_MASK | MC_CGM_MUX_0_DC_2_DIV(3U); //  40 MHz
    IP_MC_CGM->MUX_0_DC_3 = MC_CGM_MUX_0_DC_3_DE_MASK | MC_CGM_MUX_0_DC_3_DIV(1U); //  80 MHz
    IP_MC_CGM->MUX_0_DC_4 = MC_CGM_MUX_0_DC_4_DE_MASK | MC_CGM_MUX_0_DC_4_DIV(0U); // DCM
    IP_MC_CGM->MUX_0_DC_5 = MC_CGM_MUX_0_DC_5_DE_MASK | MC_CGM_MUX_0_DC_5_DIV(0U); // LBIST
    IP_MC_CGM->MUX_0_CSC
        = MC_CGM_MUX_0_CSC_SELCTL(8U) | MC_CGM_MUX_0_CSC_CLK_SW_MASK; // PLL_PHI0
    mux0Transition();
}
