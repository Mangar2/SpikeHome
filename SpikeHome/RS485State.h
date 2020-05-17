/*
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485State.h
 * Purpose:   Implements a state machine for RS485 Token Ring communication. Available States:
 *            STATE_UNKNOWN, initial state. Listens only until timeout to detect status
 *            STATE_REBOOT,  If no communication detected. Reboot of all devices expected.
 *            STATE_SINGLE,  No other device detected. Regularily sends data and checks for new devices
 *            STATE_UNREGISTERED, other devices detected, tries to register
 *            STATE_REGISTERED, Token Ring communication established
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __RS485STATE_H
#define __RS485STATE_H

#include "StdInclude.h"


class RS485State {

public:
    typedef uint16_t value_t;
    typedef uint8_t state_t;

    static const char TOKEN                            = '!';
    static const value_t PASS_SEND_TOKEN_TO_NEXT_DEVICE = 1;
    static const value_t REGISTRATION_INFO              = 2;
    static const value_t REGISTRATION_REQUEST           = 3;
    static const value_t STATE_CHANGED                  = 4;
    static const value_t LOOP_TIMEOUT                   = 10;
    static const value_t LOOP_START                     = 11;
    static const value_t LOOP_SHORT_BREAK               = 12;
    static const value_t LOOP_LONG_BREAK                = 13;


    RS485State();

    uint8_t getReceiverAddress();

    /**
     * true, if the device may send messages
     */
    bool maySend()
    {
        return mMaySend;
    }

    /**
     * Stores the address of a right ceibling registration message if key
     * matches REGISTRATION_INFO
     * @param senderAddress Adress of element sending the registration info
     * @param myAddress first Address of current device
     */
    void storeSenderAddress(uint8_t senderAddress, uint8_t myAddress);

    /**
     * Conditionally change state using received masseage
     * @param value value of the message
     * @param notForMe true, iff the message is not send to this device
     * @return value to send to other device
     */
    value_t changeState(value_t value = 0, bool notForMe = true);

    /**
     * Conditionally change state not receiving a token message
     * @return value to send to other device
     */
    value_t changeStateNoInfo();

    /**
     * Gets current state of the state machine
     * @return current state
     */
    state_t getState()
    {
        return mState;
    }

    /**
     * Decides, if commands should be ignored due to registration processes
     * @return true, if commands should be ignored
     */
    bool ignoreCommands()
    {
        return mState != STATE_STABLE;
    }

private:


    /**
     * Sets the may send flag
     * @param maySend true if we have the token, false if not
     */
    void setMaySend(bool maySend)
    {
        mMaySend = maySend;
    }


    /**
     * Sets a new state
     * @param newState current new state
     */
    void changeStateTo(state_t newState);

    /**
     * Calulates the next token ring request number if enable send should be communicated
     * @return type of next request
     */
    value_t activateEnableSend();

    /**
     * Calculates the next token ring request number if enable send has been received
     * @param notForMe true, if the enable send request was for another device
     * @return type of next request
     */
    value_t handleEnableSend(bool notForMe);

    /**
     * Handles a STATE_UNKNOWN state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleUnknown(value_t value, bool notForMe);

    /**
     * Handles a REBOOT state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleReboot(value_t value, bool notForMe);

    /**
     * Handles a SINGLE state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleSingle(value_t value, bool notForMe);

    /**
     * Handles an UNREGISTERED state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleUnregistered(value_t value, bool notForMe);

    /**
     * Handles an UNREGISTERED state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t registeredShortLoopBreak();

    /**
     * Handles a REGISTERED state
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleRegistered(value_t value, bool notForMe);

    /**
     * Handles a STABLE state. The state is stable, if registered and with few errors detected
     * @param value value of notification received
     * @param notForMe true, if notification was for another device
     * @return type of next request
     */
    value_t handleStable(value_t value, bool notForMe);



    static const state_t STATE_UNKNOWN            = 0;
    static const state_t NEIGHBOUR_UNKNOWN        = 255;
    static const state_t STATE_REBOOT             = 1;
    static const state_t STATE_SINGLE             = 2;
    static const state_t STATE_UNREGISTERED       = 3;
    static const state_t STATE_REGISTERED         = 4;
    static const state_t STATE_STABLE             = 5;

    static const uint16_t MAX_WAIT_TIMER           = 1000;
    static const uint16_t TIMER_SMALL_PERIOD       = 30; // 15
    static const uint16_t TIMER_LARGE_PERIOD       = 70; // 30
    static const uint16_t TIMER_LOOP               = TIMER_SMALL_PERIOD + TIMER_LARGE_PERIOD;
    static const uint16_t TIMEOUT_NO_ENABLE_SEND   = 4 * TIMER_LOOP;
    static const uint8_t LOOPS_TO_WAIT_AFTER_REGISTRATION = 3;


    state_t     mState;
    uint16_t    mTimer;
    uint16_t    mLastEnableSend;
    uint8_t     mWaitAfterRegistrationTimer;
    address_t   mNeighbour;
    address_t   mLeftmostCeibling;

    bool        mMaySend;
};

#endif // __RS485STATE_H
