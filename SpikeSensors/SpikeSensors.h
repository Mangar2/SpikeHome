/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      SpikeSensors.h
 * Purpose:   Static functions to create an registers sensor classes that needs additional libraries
 *
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 29. Dezember 2016, 10:09
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SPIKESENSORS_H
#define	__SPIKESENSORS_H

#include "SpikeHome.h"
#include "Schedule.h"

class SpikeSensors {
public:

    /**
     * Creates a temperature sensor and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @param index index to address the right sensor on 1-wire
     */
    static void addDTSensor(device_t deviceNo, pin_t pin, uint8_t index);

    /**
     * Creates a new LCD Device and registeres it to notify
     * @param deviceNo number of the device to add the sensor
     */
    static void addLCDDevice(device_t deviceNo);

private:
    /**
     * Do not create a spike Sensors instance, pure static
     */
    SpikeSensors();
};

#endif	/* __SPIKESENSORS_H */

