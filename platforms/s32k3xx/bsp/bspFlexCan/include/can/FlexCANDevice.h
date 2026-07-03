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

#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "bsp/timer/SystemTimer.h"
#include "can/canframes/CANFrame.h"
#include "can/transceiver/ICanTransceiver.h"
#include "mcu/mcu.h"

#include <etl/delegate.h>
#include <etl/queue.h>
#include <etl/uncopyable.h>

#include <platform/estdint.h>

// The NXP S32K3 device headers do not provide the volatile shorthands the
// S32K1xx headers had.
typedef uint8_t volatile vuint8_t;
typedef uint16_t volatile vuint16_t;
typedef uint32_t volatile vuint32_t;

namespace bios
{

class FlexCANDevice : public etl::uncopyable
{
public:
    enum CTRLTimingValues
    {
        // Protocol engine clock: 40 MHz CAN0_CLK from MC_CGM MUX_3 (see
        // bspClock). The S32K344 FlexCAN has no CTRL1.CLKSRC bit, the PE
        // clock is always the CANn_CLK.
        // PRESDIV = 24 -> fTQ = fCLK / 25 = 1.6 MHz
        // RJW = 0, PSEG1 = 4, PSEG2 = 2, PROPSEG = 6 -> Bittime = 16 TQ
        CTRL_TIMING_LOWSPEED  = (24U << 24) | (0U << 22) | (4U << 19) | (2U << 16) | (6U << 0),
        // PRESDIV = 4 -> fTQ = fCLK / 5 = 8 MHz
        // RJW = 0, PSEG1 = 4, PSEG2 = 2, PROPSEG = 6 -> Bittime = 16 TQ
        CTRL_TIMING_HIGHSPEED = (4U << 24) | (0U << 22) | (4U << 19) | (2U << 16) | (6U << 0),
        // PRESDIV = 1 -> fTQ = fCLK / 2 = 20 MHz
        // RJW = 1, PSEG1 = 6, PSEG2 = 3, PROPSEG = 7 -> Bittime = 20 TQ
        CTRL_TIMING_1MBAUD    = (1U << 24) | (1U << 22) | (6U << 19) | (3U << 16) | (7U << 0)
    };

    struct Config
    {
        uint32_t baseAddress;
        uint32_t baudrate;
        uint32_t clockSetupRegister;
        // SIUL2 pin multiplexing of the CAN pads (replaces the S32K1xx Io
        // port configuration): MSCR index + source signal select of the TX
        // pad, MSCR index of the RX pad and the IMCR index + source select
        // routing the RX pad to the CAN module.
        uint16_t txPadMscr;
        uint16_t txPadSss;
        uint16_t rxPadMscr;
        uint16_t rxImcr;
        uint16_t rxImcrSss;
        uint8_t numRxBufsStd;
        uint8_t numRxBufsExt;
        uint8_t numTxBufsApp;
        uint32_t BusId;
        uint32_t wakeUp;
    };

    enum BusState
    {
        BUS_OFF = 0,
        BUS_ON
    };

    enum
    {
        e_TRANSMIT_BUFFER_TP_SIZE = 1,
        e_TRANSMIT_BUFFER_MAX     = 32,
        e_TRANSMIT_BUFFER_START   = 16,
        e_TRANSMIT_BUFFER_MAX_APP = e_TRANSMIT_BUFFER_MAX - e_TRANSMIT_BUFFER_TP_SIZE
    };

    static uint8_t const TRANSMIT_BUFFER_UNAVAILABLE = 255;
    static uint8_t const FIRST_TRANSMIT_BUFFER       = e_TRANSMIT_BUFFER_START;
    static uint8_t const CALLBACK_TRANSMIT_BUFFER    = 31;

    FlexCANDevice(
        Config const& config,
        CanPhy& CanPhy,
        ::etl::delegate<void()> frameSentCallback,
        IEcuPowerStateController& powerManager);
    FlexCANDevice(Config const& config, CanPhy& CanPhy, IEcuPowerStateController& powerManager);

    /**
     * initialises the CAN transceiver.
     * \return the result of the transceiver initialisation.
     *             - CAN_ERR_OK if initialisation was successful
     *             - CAN_ERR_NOT_OK error
     */
    can::ICanTransceiver::ErrorCode init();

    /**
     * starts the transceiver to be able send and receive data.
     * \return the result of the transceiver open transaction.
     *             - CAN_ERR_OK if transceiver was successfully started (enabled)
     *             - CAN_ERR_NOT_OK if transceiver is NOT successfully started
     */
    can::ICanTransceiver::ErrorCode start();

    /**
     * Disables the transceiver
     */
    void stop();

    /**
     * Sets the transceiver into receive only mode.
     */
    void mute();

    /**
     * Removes the receive only mode and restores both transmit and receive
     */
    void unmute() {}

    /**
     * Called by interrupt if a frame has been successfully received.
     * It copies all the received frames into the queue
     * \param map Pointer to can id map. if map = NULL, there is no
     *               listener for this id and all frames will be enqueued.
     * \return Number of frames copied to the queue
     */
    uint8_t receiveISR(uint8_t const* filterMap = nullptr);

    void transmitISR();

    /**
     * Clears the Tp Tx buffer interrupt status flag which would have been set
     * when the Tx buffer became empty. This function is only called when
     * a Tx buffer interrupt has occurred so that the last message sent can
     * be removed from the Tx Queue
     */
    void enableTransmitInterrupt()
    {
        fpDevice->IFLAG1 = fTxInterruptMask0;
        fpDevice->IMASK1 = fpDevice->IMASK1 | fTxInterruptMask0;
    }

    void disableTransmitInterrupt()
    {
        fpDevice->IMASK1 = fpDevice->IMASK1 & ~fTxInterruptMask0;
        fpDevice->IFLAG1 = fTxInterruptMask0;
    }

    /**
     * \return true if an enabled transmit interrupt is pending.
     *
     * On the S32K344 the receive (MB 0-15) and transmit (MB 16-31) message
     * buffers share a single interrupt line, so the shared handler has to
     * check whether the transmit part has actually fired.
     */
    bool isTransmitInterruptPending() const
    {
        return (fpDevice->IFLAG1 & fpDevice->IMASK1 & fTxInterruptMask0) != 0U;
    }

    /**
     * Searches for a free CAN Buffer for a given CAN frame id.
     * if there is already a frame with the same id in one of the buffers,
     * a free buffer with a buffer number greater than the one already
     * in the hardware queue is returned else the lowest empty buffer is
     * returned.
     * \param frame    CAN frame to be queued (id and isExtended is used only).
     * \param callbackRequested    If true, CALLBACK_TRANSMIT_BUFFER (if free) is returned.
     * \return Free CAN buffer number for the id
     *             - TRANSMIT_BUFFER_UNAVAILABLE if there is no buffer available
     *             - buffer number if a free buffer is available
     */
    uint8_t getTransmitBuffer(can::CANFrame const& frame, bool callbackRequested);

    /**
     * Write a frame to a hardware buffer.
     * \param frame CAN frame to be sent
     * \param bufIdx CAN buffer index
     * \return - CAN_ERR_OK if the transaction was successful
     *             - CAN_ERR_TX_FAIL if the transaction was not successful
     */
    can::ICanTransceiver::ErrorCode
    transmit(can::CANFrame const& frame, uint8_t bufIdx, bool txInterruptNeeded = true);

    can::ICanTransceiver::ErrorCode
    transmitStream(uint8_t* Txframe, bool txInterruptNeeded = false);

    /**
     * \return the number of Transmit errors
     */
    uint8_t getTxErrorCounter() { return FLEXCAN_ECR_TXERRCNT(fpDevice->ECR); }

    /**
     * \return the number of Receive errors
     */
    uint8_t getRxErrorCounter() { return FLEXCAN_ECR_RXERRCNT(fpDevice->ECR); }

    /**
     * \return the off status of the bus
     *             - BUS_OFF if bus is in off state
     *             - BUS_ON if bus is not in bus off state
     */
    BusState getBusOffState()
    {
        // FLTCONF: 00 error active
        //          01 error passive
        //          1X bus off
        if ((FLEXCAN_ESR1_FLTCONF(fpDevice->ESR1) > 1) || fBusOff)
        {
            fBusOff = true;
            return BUS_OFF;
        }
        else
        {
            return BUS_ON;
        }
    }

    CanPhy& getPhy() { return fPhy; }

    uint8_t getIndex() const { return fIndex; }

    uint32_t getBaudrate() const { return fConfig.baudrate; }

    void dequeueRxFrame() { fRxQueue.pop(); }

    unsigned char dequeueRxFrameStream(unsigned char* data);

    can::CANFrame& getRxFrameQueueFront() { return fRxQueue.front(); }

    bool isRxQueueEmpty() const { return fRxQueue.empty(); }

    /*
     * Get Rx Counter
     */
    uint32_t getRxAlive() const { return fFramesReceived; }

    /*
     * Clear Rx Counter
     */
    void clearRxAlive() { fFramesReceived = 0; }

    uint32_t getFirstCanId() const { return fFirstRxId; }

    void resetFirstFrame() { fFirstRxId = 0; }

    /*
     * Api for
     */
    bool wokenUp();

private:
    enum
    {
        e_CANRX                 = 0x04000000,
        e_CANTX                 = 0x08000000,
        e_CAN_SRR               = 0x00400000,
        e_CAN_EXT_ID            = 0x00200000,
        e_MCR_DEV               = 0x1083003f,
        e_STANDART_ID_11BIT     = 0x7ff,
        e_STANDART_ID_29BIT     = 0x1fffffff,
        e_EXTANDET_BUFFER_START = 22,
        e_EXTANDET_BUFFER_END   = 31,
        e_BITRATE_TQ            = 16,
        e_BITRATE_PSEG1         = 3,
        e_BITRATE_PSEG2         = 2,
        e_BITRATE_PROPSEG       = 7,
        e_BITRATE_RJW           = 0x2,
        e_BITRATE_PSEG1_1000    = 0x1,
        e_BITRATE_PSEG2_1000    = 0x1,
        e_BITRATE_PROPSEG_1000  = 0x2,
        e_BITRATE_RJW_1000      = 0x1,
        e_BITRATE_SMP           = 0,
        e_CAN_INVALID           = 0xffffffff
    };

    /**
     * Struct representing an 8 byte CAN frame message buffer.
     */
    struct MessageBuffer8Byte
    {
        union
        { // FLAGS Register
            vuint32_t R;

            struct
            {
                // Free running timestamp.
                vuint32_t TIMESTAMP : 16;
                // Length of data.
                vuint32_t DLC       : 4;
                // Remote Transmission Request.
                vuint32_t RTR       : 1;
                // ID Extended Bit. 1 extended, 0 standard.
                vuint32_t IDE       : 1;
                // Substitute Remote Request
                vuint32_t SRR       : 1;
                vuint32_t unnamed0  : 1;
                vuint32_t CODE      : 4;
                vuint32_t unnamed1  : 1;
                // Error State Indicator. Indicates if tx node is in error.
                vuint32_t ESI       : 1;
                // Bit Rate Switch. Defines bit rate switch for CAN FD.
                vuint32_t BRS       : 1;
                // Extended Data Length. Distinguishes between CAN and CAN FD.
                vuint32_t EDL       : 1;
            } B;
        } FLAGS;

        union
        { // ID Register
            vuint32_t R;

            struct
            {
                vuint32_t ID_EXT : 18;
                vuint32_t ID_STD : 11;
                vuint32_t PRIO   : 3;
            } B;
        } ID;

        union
        { // Frame payload
            vuint8_t B[8];
            vuint16_t H[4];
            vuint32_t W[2];
        } DATA;
    };

    // The S32K344 device header does not expose the message buffer RAM as a
    // struct member (the S32K148 header had a RAMn array); it starts at
    // offset 0x80 and holds 96 message buffers of 16 bytes each.
    static uint32_t const MESSAGE_BUFFER_RAM_OFFSET = 0x80U;
    static uint32_t const MESSAGE_BUFFER_RAM_WORDS  = (96U * 16U) / sizeof(uint32_t);

    MessageBuffer8Byte volatile& messageBuffer(uint8_t const bufIdx)
    {
        MessageBuffer8Byte volatile* msgBuffers = reinterpret_cast<MessageBuffer8Byte volatile*>(
            fConfig.baseAddress + MESSAGE_BUFFER_RAM_OFFSET);
        return msgBuffers[bufIdx];
    }

    static uint32_t const INIT_DELAY_TIMEOUT_US = 300;
    static uint8_t const RX_QUEUE_SIZE          = 32;

    Config const& fConfig;
    CanPhy& fPhy;
    FLEXCAN_Type* const fpDevice;
    uint32_t fLastMessageBuffer;
    uint32_t fTxInterruptMask0;
    uint32_t fRxInterruptMask;
    ::etl::queue<can::CANFrame, RX_QUEUE_SIZE> fRxQueue;
    uint32_t fFirstRxId;
    uint32_t fFramesReceived;
    uint32_t fFramesReceivedTotal;
    ::etl::delegate<void()> fFrameSentCallback;
    IEcuPowerStateController& fPowerManager;
    uint8_t fIndex;
    bool fBusOff;

    void setupMessageBuffer(uint8_t bufIdx, uint8_t code, bool extended)
    {
        messageBuffer(bufIdx).FLAGS.R      = 0;
        messageBuffer(bufIdx).FLAGS.B.IDE  = extended ? 1 : 0;
        messageBuffer(bufIdx).FLAGS.B.CODE = code;
        messageBuffer(bufIdx).ID.R         = 0;
        messageBuffer(bufIdx).DATA.W[0]    = 0;
        messageBuffer(bufIdx).DATA.W[1]    = 0;
        fpDevice->RXIMR[bufIdx]            = 0;
    }

    uint8_t enqueueRxFrame(
        uint32_t id, uint8_t length, vuint32_t payload[], bool extended, uint8_t const* map);
};

namespace CANRxBuffer
{
enum Code
{
    CODE_INACTIVE = 0,
    CODE_FULL     = 2,
    CODE_EMPTY    = 4,
    CODE_OVERRUN  = 5
};

static uint32_t const FLAG_BUSY = (1UL << 24);
} // namespace CANRxBuffer

namespace CANTxBuffer
{
enum Code
{
    CODE_INACTIVE = 8,
    CODE_TRANSMIT = 12
};
} // namespace CANTxBuffer

} // namespace bios
