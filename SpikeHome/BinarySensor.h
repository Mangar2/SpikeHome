/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      BinarySensor.h
 * Purpouse:  Controls a generic sensor with a binary status, either by reading an analog or a digital
 *            pin
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __BINARYSENSOR_H
#define __BINARYSENSOR_H

#include "StdInclude.h"
#include "State.h"

class BinarySensor : public State {
public:

    /**
     * Constructs a new binary sensor
     * @param deviceNo number of the device the object belongs to
     * @param pin name/number of the input pin to use
     * @param invert true, if input will be inverted (high = off, low = on)
     * @param notifyKey identifier to send notifications
     */
    BinarySensor(device_t deviceNo, pin_t pin, bool invert, key_t notifyKey)
    : State(deviceNo, notifyKey), mPin(pin), mInvert(invert)
    {
        pinMode(mPin, INPUT);
        mLastValue = mInvert ? HIGH : LOW;
    }

    /**
     * Enables the pullup resistor
     */
    void setPullup()
    {
        State::setPullup(mPin);
    }


protected:

    /**
     * Reads the status of the input (either analog or digital)
     * @returns the status value either "LOW" or "HIGH"
     */
    virtual StateValue getValue()
    {
        return digitalReadState(mPin, mInvert);
    }

    pin_t mPin;
    bool mInvert;

};

#endif // __BINARYSENSOR_H
