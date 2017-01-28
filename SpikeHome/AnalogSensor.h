/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      AnalogSensor.h
 * Purpose:   Regularily check the state of an analog sensor, provides an avarage value and
 *            notify its change
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __ANALOGSENSOR_H
#define __ANALOGSENSOR_H

#include "StdInclude.h"
#include "State.h"

class AnalogSensor : public State {
public:

    /**
     * Constructs a new analog sensor
     * @param pin name/number of the input pin to use
     * @param inverted true, if input will be inverted (high = off, low = on)
     * @param notifyKey key to notify for changed values
     */
    AnalogSensor(device_t deviceNo, pin_t pin, bool inverted, key_t notifyKey)
    : State(deviceNo, notifyKey), mPin(pin), mInverted(inverted) { }

protected:

    /**
     * Reads an analog value from an analog input pin
     * @return analog value of mPin, invertet if mInvert = true
     */
    value_t analogReadState()
    {
        value_t state = analogRead(mPin);
        if (mInverted) {
            state = MAX_ANALOG_READ_VALUE - state;
        }
        return state;
    }

    /*
     * Reads/Gets the current status value
     * @return average value of last reads
     */
    virtual StateValue getValue()
    {
        value_t state = analogReadState();
        return value_t((mLastValue.toInt() * 5 + state) / 6);
    }

    /**
     * Checks if the state has changed.
     * @param curValue current state value
     * @param lastValue last state value
     * @return true, if state has changed enough (see constants)
     */
    virtual bool hasChanged(StateValue curValue, StateValue lastValue)
    {

        bool enoughAbsoluteChange = abs(curValue.toInt() - lastValue.toInt()) > ABSOLUTE_CHANGE_TO_SEND;
        float relChangeToSend = REL_CHANGE_TO_SEND * curValue.toInt();

        bool enoughRelativeChange = abs(lastValue.toInt() - curValue.toInt()) >= relChangeToSend;

        return enoughAbsoluteChange && enoughRelativeChange;
    }

    pin_t mPin;
    bool mInverted;

    static const value_t ABSOLUTE_CHANGE_TO_SEND = 10;
    static constexpr float REL_CHANGE_TO_SEND = 0.2;

};

#endif // __ANALOGSENSOR_H
