/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      State.cpp
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "State.h"

State::State(device_t deviceNo, key_t notify)
    : NotifyTarget(deviceNo), mNotifyKey(notify), mLastValue(0)
{
    mLoopsOnLastStateSend = 0;
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_NORMAL);
}

value_t State::analogReadState(pin_t pin, bool inverted)
{
    value_t state = analogRead(pin);
    if (inverted) {
        state = MAX_ANALOG_READ_VALUE - state;
    }
    return state;
}

value_t State::digitalReadState(pin_t pin, bool invert)
{
    value_t state;
    bool isPinAnalog = pin >= A0;
    if (isPinAnalog) {
        state = analogRead(pin);
        state = (state <= 100) ? LOW : HIGH;
    } else {
        state = digitalRead(pin);
    }
    if (invert) {
        state = state == LOW ? HIGH : LOW;
    }
    return state;
}

void State::setPullup(pin_t pin)
{
    bool isPinAnalog = pin >= A0;
    if (isPinAnalog) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        pinMode(pin, INPUT);
    } else {
        pinMode(pin, INPUT_PULLUP);
    }
}

void State::checkState(time_t scheduleLoops)
{
    StateValue curValue = getValue();
    bool hasChangedFlag = hasChanged(curValue, mLastValue);

    if (hasChangedFlag) {
        notifyChange(curValue);
        mLastValue = curValue;
        mNotifyServer = true;
    }
    if (mNotifyServer && maySend(scheduleLoops)) {
        if (notifyServer(curValue)) {
            mLoopsOnLastStateSend = scheduleLoops;
            mNotifyServer = false;
        }
    }
}

bool State::maySend(time_t scheduleLoops)
{
    time_t loopsSinceLastStateSend = scheduleLoops - mLoopsOnLastStateSend;
    bool enoughTime = loopsSinceLastStateSend >= MIN_LOOPS_BETWEEN_SEND_IN_SECONDS;
    return enoughTime;
}


void State::notifyChange(StateValue value)
{
    if (mNotifyKey != 0) {
        notify(mNotifyKey, value);
    }
}

bool State::notifyServer(StateValue value)
{
    bool result = true;
    if (mNotifyKey != 0) {
        result = sendToServer(mNotifyKey, value);
    }
    return result;
}

bool State::notifyServer(uint16_t loopCount)
{
    StateValue value = getValue();
    if (value.toInt() != mLastValue.toInt()) {
        // Ensure that server and internal devices have the same information
        // Server might get the samve value multiple time. Internally it is important to send every change only
        // once.
        notify(mNotifyKey, value);
        mLastValue = value;
    }
    return notifyServer(mLastValue);
};
