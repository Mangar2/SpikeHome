/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:    Activity.cpp
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

//#define DEBUG
#include "Activity.h"

Activity::Activity(device_t deviceNo)
: State(deviceNo, LIGHT_ON_NOTIFICATION)
{
    mActivityTimeInMilliseconds = 0;
    mLightSwichtdOnByCommand = false;
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_NORMAL);
    initConfig();
}

void Activity::initConfig()
{
    mTimeFirstSignalInSeconds = addConfigValue(INIT_LIGHT_TIME_KEY, 20);
    mIncreaseTimeNextSignalInSeconds = addConfigValue(INC_LIGHT_TIME_KEY, 30);
    mMaxTimeInSeconds = addConfigValue(MAX_LIGHT_TIME_KEY, 60);
}

void Activity::checkState(time_t scheduleLoops)
{
    if (!mState.isMoveActive()) {
        if (mActivityTimeInMilliseconds > ACTIVITY_INTERVAL) {
            mActivityTimeInMilliseconds -= ACTIVITY_INTERVAL;
        } else {
            mActivityTimeInMilliseconds = 0;
            mLightSwichtdOnByCommand = false;
            mState.lightTimeOut();
        }
    }
    mState.decreaseIgnoreMoveTimer();
    State::checkState(scheduleLoops);
}

void Activity::setTime(time_t activityTimeInSeconds)
{
    printVariableIfDebug(activityTimeInSeconds);
    time_t newTimeInMilliseconds = activityTimeInSeconds * MILLISECONDS_IN_A_SECOND;
    if (!mLightSwichtdOnByCommand || mActivityTimeInMilliseconds < newTimeInMilliseconds) {
        mActivityTimeInMilliseconds = newTimeInMilliseconds;
    }
    printVariableIfDebug(mActivityTimeInMilliseconds);
}

StateValue Activity::getValue()
{
    return value_t(mActivityTimeInMilliseconds / 1000);
}

time_t Activity::calcTimeInSeconds(uint8_t activityCount)
{
    printVariableIfDebug(activityCount);
    return min(mMaxTimeInSeconds,
            mTimeFirstSignalInSeconds + time_t(activityCount) * mIncreaseTimeNextSignalInSeconds);
}

void Activity::handleFS20Command(value_t value)
{
    value_t command = value / 4;
    if (command == 0x11) {
        setTime(60 * 60);
    } else if (command == 0x00) {
        setTime(0);
    }
}

void Activity::handleChange(address_t senderAddress, key_t key, StateValue data)
{
    value_t newValue = data.toInt();
    switch (key) {
        case MOVEMENT_NOTIFICATION:
            printStringIfDebug("MOVEMENT_NOTIFICATION device: ");
            printlnIfDebug(this->getDeviceNo());
            mState.moveDetected(newValue);
            if (newValue != 0) {
                setTime(calcTimeInSeconds(mState.getActivityWeight()));
            }
            break;
        case INIT_LIGHT_TIME_KEY:
            if (newValue >= 0 && newValue <= 10000) {
                mTimeFirstSignalInSeconds = newValue;
                setConfigValue(key, newValue);
            }
            break;
        case INC_LIGHT_TIME_KEY:
            if (newValue >= 1 && newValue <= 10000) {
                mIncreaseTimeNextSignalInSeconds = newValue;
                setConfigValue(key, newValue);
            }
            break;
        case MAX_LIGHT_TIME_KEY:
            if (newValue >= 5) {
                mMaxTimeInSeconds = newValue;
                setConfigValue(key, newValue);
            }
            break;
        case FS20_COMMMAND:
            handleFS20Command(newValue);
            mLightSwichtdOnByCommand = true;
            break;
        case SET_LIGHT_TIME:
            // Enforce sending new state to server if activity switches between on and off
            mLastValue = newValue == 0 ? 1 : 0;
            mLightSwichtdOnByCommand = newValue != 0;
            setTime(newValue);
            break;
    }
}
