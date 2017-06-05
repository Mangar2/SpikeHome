/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      clock.h
 * Purpose:
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
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
    /**
     * Initializes the clock
     * @param deviceNo device the clock belongs to
     */
    Clock(device_t deviceNo);

    /**
     * Sets the time in minutes
     * @param timeInMinutes
     */
    void setTimeInMinutes(value_t timeInMinutes);

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
    virtual void handleChange(address_t senderAddress, key_t key, StateValue value);

    /**
     * Send notifications to the server.
     * @param loopCount amount of notify loops already passed. May be used to select different values to send
     * @return true, if all notification send
     */
    virtual bool notifyServer(uint16_t loopCount);

    /**
     * Gets the minutes of current time
     */
    uint8_t getMinutes()
    {
        return mCurTime % 60;
    }

    /**
     * Get the hours of current time
     */
    uint8_t getHours()
    {
        return (mCurTime / 60);
    }

private:

    value_t mCurTime;
    uint16_t loops;
    bool initialized;

    const static value_t MINUTES_IN_A_DAY = 60 * 24;

};


#endif	/* __CLOCK_H */
