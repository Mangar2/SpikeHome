/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RollerShutter.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "RollerShutter.h"

RollerShutter::RollerShutter(device_t deviceNo, pin_t powerPin, pin_t directionPin)
: NotifyTarget(deviceNo), mPowerPin(powerPin), mDirectionPin(directionPin)
{
    pinMode(mPowerPin, OUTPUT);
    pinMode(mDirectionPin, OUTPUT);
    mRollerTime = addConfigValue(ROLLER_TIME_KEY, 10);
    mRollerStatus = 0;
    mStatusUnknown = true;
    mRollerMovement = MOVING_NOT;
    mRollerTarget = 0;
    mInformServer = false;
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_NORMAL);
}

void RollerShutter::setMovement(movement_t movement)
{
    mRollerMovement = movement;
    if (movement == MOVING_NOT) {
         // Do not change the Direction pin here, because the power switch may be a little slower thus
         // the roller will just move a short time in the opposit direction
        digitalWrite(mPowerPin, LOW);
        mInformServer = true;
    } else {
        digitalWrite(mDirectionPin, movement == MOVING_UP ? HIGH : LOW);
        digitalWrite(mPowerPin, HIGH);
    }
}

void RollerShutter::moveRoller(target_t target)
{
    movement_t movement = MOVING_NOT;
    if (target > 100) {
         // Values larger than 100% are ignored as they are not supported
         // As the server might have received the new target already we send here a new correct value
        mInformServer = true;
        return;
    }
    mRollerTarget = target;
    if (mStatusUnknown) {
        movement = target == 100 ? MOVING_DOWN : MOVING_UP;
        mRollerStatus = target == 100 ? 0 : 100;
        mStatusUnknown = false;
    } else {
        if (target == mRollerStatus) {
            movement = MOVING_NOT;
        } else if (target > mRollerStatus) {
            movement = MOVING_DOWN;
        } else {
            movement = MOVING_UP;
        }
    }
    setMovement(movement);
}

bool RollerShutter::notifyServer()
{
    return sendToServer(ROLLER_SHUTTER_KEY, target_t(mRollerStatus));
}

void RollerShutter::handleChange(key_t key, StateValue data)
{
    uint16_t dataInt = data.toInt();
    if (key == ROLLER_SHUTTER_KEY) {
        moveRoller(dataInt);
    }
    if (key == ROLLER_TIME_KEY && dataInt > 0 && dataInt <= 250) {
        mRollerTime = dataInt;
        setConfigValue(ROLLER_TIME_KEY, dataInt);
    }
}

void RollerShutter::checkState(time_t scheduleLoops)
{
    if (mInformServer) {
        mInformServer = !notifyServer();
    }

    if (mRollerMovement == MOVING_NOT) {
         // Disabling the direction relay on MOVING_NOT here ensures that it is done some time after
         // the power relay has been disabled
        if (digitalRead(mDirectionPin) == HIGH) {
            digitalWrite(mDirectionPin, LOW);
        }
    } else {

        float advanceInPercentPerTick = 100.0 * ACTIVITY_INTERVAL / MILLISECONDS_IN_A_SECOND / mRollerTime;
        if (mRollerMovement == MOVING_UP) {
            mRollerStatus = max(0, mRollerStatus - advanceInPercentPerTick);
            if (mRollerStatus <= mRollerTarget) {
                setMovement(MOVING_NOT);
            }
        } else if (mRollerMovement == MOVING_DOWN) {
            mRollerStatus = min(100, mRollerStatus + advanceInPercentPerTick);
            if (mRollerStatus >= mRollerTarget) {
                setMovement(MOVING_NOT);
            }
        }
    }

};
