/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Initialization.cpp
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 28. Dezember 2016, 07:53
 * ---------------------------------------------------------------------------------------------------
 */

#include "SpikeHome.h"
#include "SerialTextIO.h"
#include "RS485.h"
#include "Device.h"
#include "Schedule.h"
#include "Activity.h"
#include "AnalogSensor.h"
#include "BinarySensor.h"
#include "BrightnessSensor.h"
#include "DHTSensor.h"
#include "FS20UART.h"
#include "Light.h"
#include "RollerShutter.h"
#include "Schedule.h"
#include "StatusLED.h"
#include "WaterSensor.h"


void SpikeHome::init(value_t softwareVersion, device_t deviceAmount)
{
    Device::init(softwareVersion, deviceAmount);
    randomSeed(analogRead(0));
    delay(200);
    printlnIfDebug(F("Reboot"));
    // Initializes the loop handler
    Schedule::init();

    for (device_t deviceNo = 0; deviceNo < deviceAmount; deviceNo++) {
        Schedule::addTarget(&Device::getConfig(deviceNo));
    }

}

void SpikeHome::initRS485(value_t softwareVersion, device_t deviceAmount, time_t serialSpeed, pin_t readWritePin)
{

    Serial.begin(serialSpeed);
    init(softwareVersion, deviceAmount);
    RS485* serial = new RS485(deviceAmount, readWritePin);
    serial->initSerial(&Serial, serialSpeed);
    Device::setIOHandler(serial);

}

void SpikeHome::initTextIO(value_t softwareVersion, device_t deviceAmount, time_t serialSpeed)
{

    Serial.begin(serialSpeed);
    init(softwareVersion, deviceAmount);
    SerialTextIO* serial = new SerialTextIO(deviceAmount);
    serial->initSerial(&Serial, serialSpeed);
    Device::setIOHandler(serial);

}

NotifyTarget* SpikeHome::onChange(device_t deviceNo, NotifyTarget* pTarget) {
    Device::onChange(deviceNo, pTarget);
    return pTarget;
}

NotifyTarget* SpikeHome::onChange(NotifyTarget* pTarget) {
    return onChange(pTarget->getDeviceNo(), pTarget);
}

NotifyTarget* SpikeHome::addToSchedule(NotifyTarget* pTarget)
{
    Schedule::addTarget(pTarget);
    return pTarget;
}

NotifyTarget* SpikeHome::addActivity(device_t deviceNo)
{
    return onChange(addToSchedule(new Activity(deviceNo)));
}

NotifyTarget* SpikeHome::addAnalogSensor(device_t deviceNo, pin_t pin, bool invert, key_t notifyKey)
{
    return addToSchedule(new AnalogSensor(deviceNo, pin, invert, notifyKey));
}

BinarySensor* SpikeHome::addBinarySensor(device_t deviceNo, pin_t pin, bool invert, key_t notifyKey)
{
    BinarySensor* sensor = new BinarySensor(deviceNo, pin, invert, notifyKey);
    addToSchedule(sensor);
    return sensor;
}

NotifyTarget* SpikeHome::addBrightnessSensor(device_t deviceNo, pin_t pin)
{
    return addToSchedule(new BrightnessSensor(deviceNo, pin));
}

NotifyTarget* SpikeHome::addLight(device_t deviceNo, pin_t brightnessPin, pin_t pwmPin)
{
    return onChange(addToSchedule(new Light(deviceNo, brightnessPin, pwmPin)));
}

NotifyTarget* SpikeHome::addMovementSensor(device_t deviceNo, pin_t pin, key_t notifyKey)
{
    return addBinarySensor(deviceNo, pin, BinarySensor::NOT_INVERTED, notifyKey);
}

NotifyTarget* SpikeHome::addRollerShutter(device_t deviceNo, pin_t powerPin, pin_t directionPin)
{
    return addToSchedule(new RollerShutter(deviceNo, powerPin, directionPin));
}

NotifyTarget* SpikeHome::addStatusLED(device_t deviceNo, pin_t statusPin, key_t statusKey)
{
    return onChange(new StatusLED(deviceNo, statusPin, statusKey));
}

NotifyTarget* SpikeHome::addSwitches(device_t deviceNo, uint16_t pinBitMaskLSBD2)
{
    return addToSchedule(new Switches(deviceNo, pinBitMaskLSBD2));
}

NotifyTarget* SpikeHome::addWaterSensor(device_t deviceNo, pin_t pin)
{
    return addToSchedule(new WaterSensor(deviceNo, pin));
}

NotifyTarget* SpikeHome::addWindowSensor(device_t deviceNo, pin_t pin)
{
    BinarySensor* sensor = addBinarySensor(deviceNo, pin, BinarySensor::NOT_INVERTED, NotifyTarget::WINDOW_OPEN_NOTIFICATION);
    sensor->setPullup();
    return sensor;
}

NotifyTarget*  SpikeHome::addDHTSensor(device_t deviceNo, pin_t pin)
{
    return addToSchedule(new DHTSensor(deviceNo, pin));
}

NotifyTarget*  SpikeHome::addFS20UART(device_t deviceNo, pin_t rxPin, pin_t txPin)
{
    return addToSchedule(new FS20UART(deviceNo, rxPin, txPin));
}