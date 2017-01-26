/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    Activity.cpp
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "Activity.h"

Activity::Activity(device_t deviceNo)
: State(deviceNo, LIGHT_ON_NOTIFICATION)
{
    mActivityTimeInMilliseconds = 0;
    mMoveActiveCount = 0;
    mActivityCount = 0;
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
    if (mMoveActiveCount == 0) {
        if (mActivityTimeInMilliseconds > ACTIVITY_INTERVAL) {
            mActivityTimeInMilliseconds -= ACTIVITY_INTERVAL;
        } else {
            mActivityTimeInMilliseconds = 0;
        }
    }
    State::checkState(scheduleLoops);
}

void Activity::setTime(time_t activityTimeInSeconds)
{
    printVariableIfDebug(activityTimeInSeconds);
    mActivityTimeInMilliseconds = activityTimeInSeconds * MILLISECONDS_IN_A_SECOND;
    printIfDebug(F("Activity(ms):"));
    printlnIfDebug(mActivityTimeInMilliseconds);
}

StateValue Activity::getValue()
{
    return value_t(mActivityTimeInMilliseconds / 1000);
}

time_t Activity::calcTimeInSeconds()
{
    return min(mMaxTimeInSeconds,
            mTimeFirstSignalInSeconds + time_t(mActivityCount) * mIncreaseTimeNextSignalInSeconds);
}

void Activity::increaseActivityTime()
{

    if (mActivityTimeInMilliseconds == 0) {
        mActivityCount = 0;
    } else {
        mActivityCount = min(100, mActivityCount + 1);
    }
    setTime(calcTimeInSeconds());
}

void Activity::setActivityTimeOnEntryMovement()
{
    bool recentMove = int32_t(mActivityTimeInMilliseconds) >= (int32_t(calcTimeInSeconds()) - 20) * int32_t(MILLISECONDS_IN_A_SECOND);
    if (mActivityTimeInMilliseconds < mTimeFirstSignalInSeconds || recentMove) {
        setTime(mTimeFirstSignalInSeconds);
    }
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

void Activity::handleChange(key_t key, StateValue data)
{
    value_t newValue = data.toInt();
    switch (key) {
        case MOVEMENT_NOTIFICATION:
            if (newValue == 0) {
                if (mMoveActiveCount > 0) {
                    mMoveActiveCount--;
                }
            } else {
                mMoveActiveCount++;
                increaseActivityTime();
            }
            break;
        case ENTRY_MOVEMENT_NOTIFICATION:
            if (newValue != 0) {
                setActivityTimeOnEntryMovement();
            }
            break;
        case INIT_LIGHT_TIME_KEY:
            if (newValue >= 5 && newValue <= 10000) {
                mTimeFirstSignalInSeconds = newValue;
                setConfigValue(key, newValue);
            }
            break;
        case INC_LIGHT_TIME_KEY:
            if (newValue >= 5 && newValue <= 10000) {
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
        case FS20_COMMMAND: handleFS20Command(newValue);
            break;
        case SET_LIGHT_TIME:
            // Enforce sending new state to server if activity switches between on and off
            mLastValue = newValue == 0 ? 1 : 0;
            setTime(newValue);
            break;
    }
}
