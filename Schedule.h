/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Schedule.h
 * Purpose:   Schedules tasks to all registered objects (sensors, handlers, ...). They must derive
 *            from NotifyTarget.
 *            1. Every registered object will continously receive a checkstate event.
 *            2. Every registered object will continously receive a notifyServer event. This sould
 *            be used to send the current state of the object to the server (via. RS485 Bus).
 *            One object after another receives this event and the interval between two objects
 *            receiving the event can be configured in 1 seconds steps.
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include "StdInclude.h"
#include "NotifyTargetList.h"

typedef uint8_t timer_t;

class Schedule {
public:


    static const time_t ONE_SECOND                 = 1000L;

    /**
     * Initializes the count - class
     */
    static void init();

    /**
     * Gets the amount of seconds elapsed since last reset
     * @return seconds elapsed since last reset
     */
    static time_t getTimeElapsedInSeconds() {
        return millis() / ONE_SECOND;
    }

    /**
     * Gets the amount of time elapsed in milliseconds since last reset
     * @return milliseconds elapsed since last reset
     */
    static time_t getTimeElapsedInMilliseconds() {
        return millis();
    }

    /**
     * Main schedule function. Needs to be added to the main loop. Handles all scheduling and ensures that
     * 10 milliseconds are passed between two ticks by calling the necessary delay. (Example if objects took 7
     * milliseconds to handle their work the delay will last 3 milliseconds).
     * Even if on tick lasts longer than 10 milliseconds the function tries to reach 10 seconds in average by
     * reducing the time of the next ticks.
     */
    static void nextTick();

    /**
     * Adds a target to check regularily by calling his checkState method
     * @param pTarget pointer to a notification Target
     */
    static void addTarget(NotifyTarget* pTarget);

    /**
     * Notifies all objects/sensors of a device of a configuration change
     * @param deviceNo number of device
     * @param senderAddress address of the sender
     * @param key key/identifier of the change
     * @param value new value
     */
    static void notifyChange(device_t deviceNo, address_t senderAddress, key_t key, value_t value);

    /**
     * Broadcasts a change to all devices
     * @param senderAddress address of the sender
     * @param key key of the change
     * @param value value of the change
     */
    static void broadcastChange(address_t senderAddress, key_t key, value_t value);

private:

    /**
     * regularily calls notify functions of registered objects if enough time is elapsed
     */
    static void notify();

    /**
     * Calculates the difference of two times for unsigned numbers. Ensures that the result is not below zero
     * @param start time to subtract from
     * @param sub time to subtract
     * @return 0 if sub >= time, time - sub else
     */
    static time_t timeDiff(time_t start, time_t sub) {
        time_t result = start > sub ? start - sub : 0;
        if (result > NotifyTarget::MILLISECONDS_PER_LOOP) {
            result = NotifyTarget::MILLISECONDS_PER_LOOP;
        }
        return result;
    }

    /**
     * Regularily calls checkState for registered objects
     */
    static void checkState();

    /**
     * Checks settings in debug mode
     */
    static void checkSettings();


    static const time_t NOTIFY_INTERVAL_IN_MILLISECONDS = ONE_SECOND * 1;


    static time_t         mNextLoop;
    static time_t         mLoops;
    static NotifyTargetList mTargetList;
    static time_t         mNotifyTimer;
    static NotifyTarget*  mNotifyIterator;
    static uint16_t       mNotifyLoopCount;
    static value_t        mConfigInfoPeriod;

};

#endif // __SCHEDULE_H
