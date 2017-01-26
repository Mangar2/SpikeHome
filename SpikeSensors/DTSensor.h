/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    DTSensor.h
 * Purpose: Controls a DS18B20 Temperature Sensor connected by onewire
 *
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */


#ifndef __DTSENSOR_H
#define __DTSENSOR_H

#include "State.h"

#define DTSENSOR_TEMP_DIFFERENCE_TO_REPORT 0.5

class DallasTemperature;

class DTSensor : public State {
public:
    /**
     * Constructs a new DS18B20 Temperature sensor
     * @param deviceNo number of the device the object belongs to
     * @param pin name/number of the input pin to use
     * @param index name/number of the index on the one-wire. The Index is usually zero, if only one sensor is tattached to the wire
     */
    DTSensor(device_t deviceNo, pin_t pin, uint8_t index);


private:

    /**
     * Checks if the state has changed.
     * @param curValue current state value
     * @param lastValue last state value
     * @return true, if state has changed
     */
    virtual bool hasChanged(StateValue curValue, StateValue lastValue);

    /**
     * Reads the sensor information returns the value
     * @return current temperature
     */
    virtual StateValue getValue();


    uint8_t mIndex;
    time_t mNextRead;
    static bool mSensorAvailable;
    static DallasTemperature* mpDT;
};

#endif // __DTSENSOR_H
