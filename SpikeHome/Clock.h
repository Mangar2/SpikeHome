/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      clock.h
 * Purpose:
 *
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 20. Januar 2017, 21:34
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __CLOCK_H
#define	__CLOCK_H

#include "StdInclude.h"

class Clock : public NotifyTarget
{

public:

    typedef char timeOfDay_t[9];

    /**
     * Constructor of clock, and sets the device number
     * @param deviceNo number of device the clock belongs to
     */
    Clock(device_t deviceNo);

    /**
     * Sets the time (unix timestamp)
     * @param time
     */
    static void setTime(time_t time)
    {
        timestamp = time;
        initialized = true;
    }

    /**
     * Needs to be called every 10 ms, updates the time
     * @param scheduleLoops not used
     */
    virtual void checkState(time_t scheduleLoops);

    /**
     * Reacts on a data change on type CLOCK_KEY
     * @param key type of change
     * @param value value of change
     */
    virtual void handleChange(key_t key, StateValue value);

    /**
     * Gets the seconds of current timestamp
     */
    static uint16_t getSeconds()
    {
        return timestamp % 60;
    }

    /**
     * Gets the minutes of current timestamp
     */
    static uint16_t getMinutes()
    {
        return (timestamp / 60) % 60;
    }

    /**
     * Get the hours of current timestamp
     */
    static uint16_t getHours()
    {
        return (timestamp / (60 * 60)) % 24;
    }

    /**
     * Gets the time of day in format HH:MM:SS
     * @param buf 9 byte buffer to receive the time of day
     */
    static void getTimeOfDay(timeOfDay_t buf);

    /**
     * Prints the time of day to serial (debug mode only)
     */
    static void printTimeOfDay();

    static time_t timestamp;
    static bool   initialized;
    uint8_t loops;

};


#endif	/* __CLOCK_H */
