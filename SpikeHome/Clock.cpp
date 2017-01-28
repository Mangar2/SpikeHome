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

time_t Clock::timestamp;
bool   Clock::initialized = false;


Clock::Clock(device_t deviceNo) : NotifyTarget(deviceNo), loops(0)
{
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_ALLWAYS);
}

void Clock::checkState(time_t scheduleLoops)
{
    if (initialized) {
        loops++;
        if (loops >= NotifyTarget::MILLISECONDS_IN_A_SECOND / NotifyTarget::MILLISECONDS_PER_LOOP) {
            loops = 0;
            timestamp++;
            notify(CLOCK_NOTIFICATION, (value_t) timestamp);
        }
    }
}

void Clock::handleChange(key_t key, StateValue value)
{
    switch (key) {
        case CLOCK_KEY:
            setTime(value.toInt() * 60L);
            break;
    }
}

void Clock::getTimeOfDay(timeOfDay_t buf)
{
    uint16_t hours = getHours();
    uint16_t minutes = getMinutes();
    uint16_t seconds = getSeconds();
    buf[0] = '0' + hours / 10;
    buf[1] = '0' + hours % 10;
    buf[2] = ':';
    buf[3] = '0' + minutes / 10;
    buf[4] = '0' + minutes % 10;
    buf[5] = ':';
    buf[6] = '0' + seconds / 10;
    buf[7] = '0' + seconds % 10;
    buf[8] = 0;
}

void Clock::printTimeOfDay()
{
#ifdef DEBUG
    timeOfDay_t buf;
    getTimeOfDay(timestamp, buf);
    printlnIfDebug(buf);
#endif
}
