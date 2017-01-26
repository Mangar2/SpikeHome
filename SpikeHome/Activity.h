/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Activity.h
 * Purpose:   Receives move notifications and sends activity notifications.
 *            Once a move is recognized it will send activity on and wait until a timeout to send
 *            notification off. The on state is retriggarable, thus the timer value is set fully
 *            again once it receives new move notifications
 *            The timout can be configured by the following configuration values:
 *            INIT_LIGHT_TIME_KEY: Timout in seconds on the first movement detected
 *            INC_LIGHT_TIME_KEY: Increase timeout cumulatively in seconds for every additional move detected
 *            MAX_LIGHT_TIME_KEY: Maximal timout in seconds
 *            SET_LIGHT_TIME: Timeout set externally. Might be larger than the maximal light time
 *            set by moves
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __ACTIVITY_H
#define __ACTIVITY_H

#include "StdInclude.h"
#include "State.h"

class Activity : public State {
public:

    static const time_t ACTIVITY_INTERVAL = ((CHECKSTATE_NORMAL + 1) * MILLISECONDS_PER_LOOP);

    /**
     * Initializes the activity manager
     */
    Activity(device_t deviceNo);


    /**
     * Call this function to signal a change in values
     * @param key identifies the item that changed
     * @param data new value of the item
     */
    virtual void handleChange(key_t key, StateValue data);

    /**
     * Checks the sensor state. This function must be called regularily in the loop.
     * @param scheduleLoops number of checkState loops since reboot
     */
    virtual void checkState(time_t scheduleLoops);

protected:

    /**
     * Gets the activity state
     */
    virtual StateValue getValue();

    /**
     * Checks if the state has changed. Only change from on to off is
     * a change that should be notified to the server
     * @param float mCurValue current state value
     * @param float mLastValue last state value
     * @return true, if state has changed
     */
    virtual bool hasChanged(StateValue curValue, StateValue lastValue)
    {
        return (curValue.toInt() == 0) != (lastValue.toInt() == 0);
    };

    /**
     * Notifies the server about current state
     * @param loopCount loop count of schedule
     * @return true, if notification has been successfully
     */
    virtual bool notifyServer(uint16_t loopCount)
    {
        return State::notifyServer(getValue());
    };

    /**
     * Sets the time the time the activity stayes active (on)
     * @param int32_t activitytimeInSeconds
     */
    void setTime(time_t activityTimeInSeconds);



private:

    /**
     * Initializes the configuration
     */
    void initConfig();

    /**
     * Increases the activity time
     *
     */
    void increaseActivityTime();

    /**
     * Increases activity time on movement detection by a sensor placed at the entry of a larger room
     */
    void setActivityTimeOnEntryMovement();

    /**
     * Handles a command comming from fs20 device
     * @param value command value (bits 2..10) including command suffix (bits 0..1) command 0 switches activity off
     * command 0x11 switches activity on
     */
    void handleFS20Command(value_t value);

    /**
     * Calculates the time to set in seconds
     * @return time to set in seconds
     */
    time_t calcTimeInSeconds();

    time_t mActivityTimeInMilliseconds;
    uint8_t mMoveActiveCount;
    uint8_t mActivityCount;

    value_t mTimeFirstSignalInSeconds;
    value_t mIncreaseTimeNextSignalInSeconds;
    value_t mMaxTimeInSeconds;
};

#endif // __ACTIVITY_H
