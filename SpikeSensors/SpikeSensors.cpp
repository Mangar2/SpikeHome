/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      SpikeSensors.cpp
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 29. Dezember 2016, 10:10
 * ---------------------------------------------------------------------------------------------------
 */

#include "SpikeSensors.h"
#include "Schedule.h"
#include "DHTSensor.h"
#include "DTSensor.h"
#include "LCDDevice.h"
#include "FS20UART.h"


void SpikeSensors::addDTSensor(device_t deviceNo, pin_t pin, uint8_t index)
{
    SpikeHome::addToSchedule(new DTSensor(deviceNo, pin, index));
}


void SpikeSensors::addLCDDevice(device_t deviceNo)
{
    SpikeHome::onChange(new LCDDevice(deviceNo));
}
