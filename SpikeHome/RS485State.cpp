/*
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      RS485State.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#include "RS485State.h"

RS485State::RS485State()
{
    mState = STATE_UNKNOWN;
    mTimer = 0;
    mNeighbour = NEIGHBOUR_UNKNOWN;
    mLeftmostCeibling = NEIGHBOUR_UNKNOWN;
    mMaySend = false;
}

uint8_t RS485State::getReceiverAddress()
{
    if (mNeighbour == NEIGHBOUR_UNKNOWN) {
        if (mLeftmostCeibling == NEIGHBOUR_UNKNOWN) {
            return 0;
        }
        return mLeftmostCeibling;
    } else {
        return mNeighbour;
    }
}

void RS485State::storeSenderAddress(uint8_t senderAddress, uint8_t myAddress)
{
    bool rightCeibling =  (myAddress < senderAddress) && (mNeighbour > senderAddress);
    if (rightCeibling) {
        mNeighbour = senderAddress;
    }
    mLeftmostCeibling = min(mLeftmostCeibling, senderAddress);
}


value_t RS485State::changeState(value_t value, bool notForMe)
{
    value_t res = 0;
    switch (mState) {
        case STATE_UNKNOWN: res = handleUnknown(value, notForMe); break;
        case STATE_REBOOT: res = handleReboot(value, notForMe); break;
        case STATE_SINGLE: res = handleSingle(value, notForMe); break;
        case STATE_UNREGISTERED: res = handleUnregistered(value, notForMe); break;
        case STATE_REGISTERED: res = handleRegistered(value, notForMe); break;
    }
    return res;
}

value_t RS485State::changeStateNoInfo()
{
    uint16_t loopState;
    value_t res = 0;
    if (mTimer >= MAX_WAIT_TIMER) {
        res = changeState(LOOP_TIMEOUT);
    } else {
        loopState = mTimer % TIMER_LOOP;
        if (loopState == 0) {
            res = changeState(LOOP_START);
        } else if (loopState == TIMER_SMALL_PERIOD) {
            res = changeState(LOOP_SHORT_BREAK);
        } else if (loopState == TIMER_LARGE_PERIOD) {
            res = changeState(LOOP_LONG_BREAK);
        }
    }
    if (res != STATE_CHANGED) {
        mTimer += 1;
    }
    return res;
}

void RS485State::changeStateTo(state_t newState)
{
    mTimer = 0;
    mState = newState;
}

value_t RS485State::activateEnableSend()
{
    value_t res = 0;
    if (getReceiverAddress() == 0) {
        res = REGISTRATION_REQUEST;
    } else {
        res = ENABLE_SEND;
    }
    return res;
}

value_t RS485State::handleEnableSend(bool notForMe)
{
    if (notForMe) {
        changeStateTo(STATE_UNREGISTERED);
    } else {
        changeStateTo(STATE_REGISTERED);
        setMaySend(true);
    }
    return STATE_CHANGED;
}

value_t RS485State::handleUnknown(value_t value, bool notForMe)
{
    value_t res = 0;
    setMaySend(false);
    switch (value) {
    case ENABLE_SEND:
        res = handleEnableSend(notForMe);
        break;
    case REGISTRATION_INFO:
        // Only registration request will lead to a registration
        break;
    case REGISTRATION_REQUEST:
        changeStateTo(STATE_UNREGISTERED);
        res = REGISTRATION_INFO;
        break;
    case LOOP_START:
        if (mTimer == 0) {
            mNeighbour = NEIGHBOUR_UNKNOWN;
            mLeftmostCeibling = NEIGHBOUR_UNKNOWN;
        }
        delay(random(10));
        break;
    case LOOP_TIMEOUT:
        changeStateTo(STATE_REBOOT);
        res = STATE_CHANGED;
        break;
    }
    return res;
}

value_t RS485State::handleReboot(value_t value, bool notForMe)
{
    key_t res = 0;
    setMaySend(false);
    switch (value) {
    case ENABLE_SEND:
        res = handleEnableSend(notForMe);
        break;
    case REGISTRATION_INFO:
        // Only a registration request will lead to a registration
        break;
    case REGISTRATION_REQUEST:
        changeStateTo(STATE_UNREGISTERED);
        res = REGISTRATION_INFO;
        break;
    case LOOP_START:
        res = activateEnableSend();
        break;
    case LOOP_TIMEOUT:
        changeStateTo(STATE_SINGLE);
        res = STATE_CHANGED;
        break;
    }
    return res;
}

value_t RS485State::handleSingle(value_t value, bool notForMe)
{
    value_t res = 0;

    switch (value) {
    case ENABLE_SEND:
        res = handleEnableSend(notForMe);
        break;
    case REGISTRATION_INFO:
        setMaySend(false);
        changeStateTo(STATE_UNKNOWN);
        res = STATE_CHANGED;
        break;
    case REGISTRATION_REQUEST:
        setMaySend(false);
        changeStateTo(STATE_UNREGISTERED);
        res = REGISTRATION_INFO;
        break;
    case LOOP_START:
        setMaySend(false);
        res = REGISTRATION_REQUEST;
        break;
    case LOOP_SHORT_BREAK:
        setMaySend(true);
        break;
    case LOOP_TIMEOUT:
        mTimer = 0;
        break;
    }
    return res;
}

value_t RS485State::handleUnregistered(value_t value, bool notForMe)
{
    value_t res = 0;
    setMaySend(false);
    switch (value) {
    case ENABLE_SEND:
        if (!notForMe) {
            changeStateTo(STATE_REGISTERED);
            setMaySend(true);
        }
        res = STATE_CHANGED;
        break;
    case REGISTRATION_INFO:
        break;
    case REGISTRATION_REQUEST:
        res = REGISTRATION_INFO;
        break;
    case LOOP_TIMEOUT:
        changeStateTo(STATE_UNKNOWN);
        res = STATE_CHANGED;
        break;
    }
    return res;
}

value_t RS485State::registeredShortLoopBreak()
{
    value_t res = 0;
    bool mTokenLost = (mLastEnableSend + TIMEOUT_NO_ENABLE_SEND <= mTimer);
    if (mTimer == TIMER_SMALL_PERIOD || mTokenLost) {
        mLastEnableSend = mTimer;
        setMaySend(false);
        if (mNeighbour == NEIGHBOUR_UNKNOWN && !mTokenLost) {
            res = REGISTRATION_REQUEST;
        } else {
            res = ENABLE_SEND;
        }
    }
    return res;
}

value_t RS485State::handleRegistered(value_t value, bool notForMe)
{
    value_t res = 0;
    switch (value) {
    case ENABLE_SEND:
        if (!notForMe) {
            setMaySend(true);
            mTimer = 0;
        } else {
            setMaySend(false);
            mLastEnableSend = mTimer;
        }
    case REGISTRATION_INFO: break;
    case REGISTRATION_REQUEST: break;
    case LOOP_SHORT_BREAK:
        res = registeredShortLoopBreak();
        break;
    case LOOP_LONG_BREAK:
        if (mTimer == TIMER_LARGE_PERIOD && mNeighbour == NEIGHBOUR_UNKNOWN && mLeftmostCeibling != NEIGHBOUR_UNKNOWN) {
            res = ENABLE_SEND;
        }
        break;
    case LOOP_TIMEOUT:
        changeStateTo(STATE_UNREGISTERED);
        res = STATE_CHANGED;
        break;
    }
    return res;
}
