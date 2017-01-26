/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    DTSensor.cpp
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "DTSensor.h"

bool DTSensor::mSensorAvailable;
DallasTemperature* DTSensor::mpDT = 0;

DTSensor::DTSensor(device_t deviceNo, pin_t pin, uint8_t index)
    :State(deviceNo, SYS_TEMPERATURE_NOTIFICATION), mNextRead(0)
{
    mIndex = index;
    if (mpDT == 0) {
        mpDT = new DallasTemperature(new OneWire(pin));
        mpDT->begin();
        mSensorAvailable = mpDT->getDeviceCount() > 0;

        if (!mSensorAvailable) {
            printIfDebug(F("Could not find a Dallas Temperature sensor on pin "));
            printlnIfDebug(pin);
        } else {
            printIfDebug(F("Dallas Temperature found: "));
            printlnIfDebug(mpDT->getDeviceCount());
        }
    }
}


bool DTSensor::hasChanged(StateValue curValue, StateValue lastValue)
{
    return abs(curValue.toFloat() - lastValue.toFloat()) > DTSENSOR_TEMP_DIFFERENCE_TO_REPORT;
};

/**
 * Reads the sensor information and notifys his changes
 */
StateValue DTSensor::getValue()
{
    StateValue result = mLastValue;
    time_t curTime    = millis();
    if (mSensorAvailable && mNextRead < curTime && mpDT->requestTemperaturesByIndex(mIndex)) {
        result = mpDT->getTempCByIndex(mIndex);
        mNextRead = curTime + MILLISECONDS_IN_A_SECOND * 60;
    }
    return result;
}
