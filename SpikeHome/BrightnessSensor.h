/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      BrightnessSensor.h
 * Purpose:   Regularily check the state of a brightness sensor sensor and notify its change
  * Author:   Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __BRIGHTNESSSENSOR_H
#define __BRIGHTNESSSENSOR_H

#include "StdInclude.h"
#include "State.h"

class BrightnessSensor : public State {
public:

    /**
     * Creates a brightness sensor
     * @param deviceNo device the sensor is attached to
     * @param pin the analog pin where the brightness sensor is attached to
     */
    BrightnessSensor(device_t deviceNo, pin_t analogPin);

protected:

    /**
     * Checks if the state has changed enhough to notify the server.
     * @param curValue current state value
     * @param lastValue last state value
     * @return true, if state has changed
     */
    virtual bool hasChanged(StateValue curValue, StateValue lastValue);

    /**
     * Gets the brightness of light. Return a reverse value, because light sensor is plugged
     * between ground and input. More light leads to lower values.
     * @return brightness of linght (absolute value)
     */
    value_t getAbsoluteValue();

    /*
     * Reads the current sensor value
     * @return current sensor value
     */
    virtual StateValue getValue();

    /**
     * Handles a change of the FULL_ON_VALUE. Register change type so that the class is informed on change
     * @param key key/identifier of the change
     * @param data new value
     */
    virtual void handleChange(key_t key, StateValue data);

    /**
     * Calculates the neccessary brightness value to reach a brightness level in percent of the
     * maximal possible brightness.
     * @param deviceNo number of the device
     * @param brightnessInPercent percentage of maximal brightness
     * @param absolute brightness value to reach the brightness level
     */
    value_t calcAbsoluteTarget(value_t brightnessInPercent);

    pin_t   mPin;
    value_t mFullOnBrightness;

};

#endif // __BRIGHTNESSSENSOR_H
