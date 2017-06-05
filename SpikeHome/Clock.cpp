/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Clock.cpp
 *
 * Author:   Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 22. Januar 2017, 07:45
 * ---------------------------------------------------------------------------------------------------
 */

#include "Clock.h"


Clock::Clock(device_t deviceNo) : NotifyTarget(deviceNo), loops(0), initialized(false)
{
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_ALLWAYS);
}


void Clock::setTimeInMinutes(value_t timeInMinutes)
{
    mCurTime = timeInMinutes;
    initialized = true;
    loops = 0;
    notify(CLOCK_NOTIFICATION, mCurTime);
}

void Clock::checkState(time_t scheduleLoops)
{
    value_t newTimeInMinutes;
    if (initialized) {
        loops++;
        if (loops >= 65 * LOOPS_PER_SECOND) {
            newTimeInMinutes = mCurTime + 1;
            if (newTimeInMinutes >= MINUTES_IN_A_DAY) {
                newTimeInMinutes -= MINUTES_IN_A_DAY;
            }
            setTimeInMinutes(newTimeInMinutes);
            // Wait 65 seconds to increase the minute to see if the server sends a new minute info. But wait only 60 seconds
            // for the following minutes if the server stopped sending
            loops = 5 * NotifyTarget::LOOPS_PER_SECOND;
        }
    }
}

void Clock::handleChange(address_t senderAddress, key_t key, StateValue value)
{
    if ((key == CLOCK_KEY) && (value.toInt() < 60 * 24)) {
        setTimeInMinutes(value.toInt());
     }
}

bool Clock::notifyServer(uint16_t loopCount)
{
    sendToServer('c', mCurTime);
    return true;
}

