/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    State.h
 * Purpose: Adds functionality for notification targets with values thate are regularily read and
 *          send to the server
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef STATE_H
#define STATE_H

#include "StdInclude.h"

class State : public NotifyTarget {
public:

    static const value_t  MAX_ANALOG_READ_VALUE                             = 1024;


    /**
     * @param deviceNo number of device the object belongs to
     * @param notify type of the notification
     */
    State(device_t deviceNo, key_t notify);

    /**
     * Checks the sensor state. This function must be called regularily in the loop.
     * @param scheduleLoops number of checkState loops since reboot
     */
    virtual void checkState(time_t scheduleLoops);

   /**
    * Handles a change notification. This function is called by Notify if the object is registered
    * @param notifyType type of the change
    * @param float data new value
    */
    virtual void handleChange(key_t notifyType, float data) {};

    /**
     * Reads an analog value from an analog input pin
     * @param pin analog pin to read from
     * @param inverted true, if value is inverted
     * @return analog value of pin, invertet if mInvert = true
     */
    static value_t analogReadState(pin_t pin, bool inverted);

    /**
     * Reads the status of an input pin as digital value. The pin may either be an analog or a digital pin
     * On Analog pins values > 100 are concidered as HIGH.
     * @param pin pin (analog or digital) to read from
     * @param inverted true, if value is inverted
     * @returns the status value either "LOW" or "HIGH"
     */
    static value_t digitalReadState(pin_t pin, bool invert);

    /**
     * Enables the pullup resistor of an arduino pin. It works for binary and analog pins
     * Please note that the Arduino and äquivalent does not provide pullups for A6 and A7 pins.
     * @param pin pin where the pullup resistor will be activated
     */
    static void setPullup(pin_t pin);

protected:

    /*
     * Reads/Gets the current status value
     */
    virtual StateValue getValue() { return 0; }

    /**
     * Checks if the state has changed.
     * @param mCurValue current state value
     * @param mLastValue last state value
     * @return true, if state has changed
     */
    virtual bool hasChanged(StateValue curValue, StateValue lastValue)
    {
        return curValue.toInt() != lastValue.toInt();
    };

    /**
     * Sends a notification on change.
     * @param value changed value
     */
    virtual void notifyChange(StateValue value);

    /**
     * Sends a notification to the server
     * @param value value to notify
     */
    virtual bool notifyServer(StateValue value);

    /**
     * Polling call from schedule to generate notifications
     * @param loopCount number of schedule loop. May be used to select the type of notification if multiple
     * notifiation types should be send.
     * @return true, if the server could be notified
     */
    virtual bool notifyServer(uint16_t loopCount);


    /**
     * Retuns true, if the sensor may send its state because enough time is elapsed
     * @param scheduleLoops number of checkState loops since reboot
     */
    bool maySend(time_t scheduleLoops);

    /**
     * Reads sensor value and notifies registered objects and server about changes
     * @param timeElapsedInSeconds time elapsed in seconds since server reboot
     */
    void readValueAndNotify(time_t timeElapsedInSeconds);

    key_t      mNotifyKey;
    StateValue mLastValue;
    time_t     mLoopsOnLastStateSend;
    bool       mNotifyServer;

private:

    static const time_t MIN_LOOPS_BETWEEN_SEND_IN_SECONDS = 5 * 100;
};

#endif //STATE_H
