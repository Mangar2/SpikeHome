/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial4.ino
 * Purpouse:  Tutorial, Step 4, more sensors. You need to also include the "SpikeSensor" library, 
 * the DallasTemperature library and the OneWire library
 *
 * This tutorial will add sensors. Sensor output will be shown in the serial monitor in a form already
 * shown in older tutorials. Note that sensor values will be reported directly if value changes much.
 * A sensor will not send wait at least 5 seconds between two reports to the sever. The value is 
 * defined in State.h Constant: MIN_LOOPS_BETWEEN_SEND_IN_SECONDS
 * 
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

/**
 * Standard include, you need to include this to use the framework
 */
#include "SpikeHome.h"

/**
 * Additional sensors, you need to include this to use additional sensors 
 */
#include "SpikeSensors.h"

/**
 * You can set SOFTWARE_VERSION to any type you like. I think it is handy to know wich software version is currently
 * running on a home automation device. So increase the number when you upload a new version.
 */
const int SOFTWARE_VERSION      = 1;
/**
 * Devices are a basic concept of the library. Devices are used to separate objects
 * for example to group sensors of different rooms.
 */
const int DEVICE_AMOUNT         = 2;

/**
 * Serial speed to communicate, usually 9600
 */
const long SERIAL_SPEED         = 9600;

const pin_t DHT_SENSOR_PIN      = 5;
const pin_t DT_SENSOR_PIN       = 2;
const pin_t LDR_SENSOR_PIN      = A0;
const pin_t DT_SENSOR_NO        = 0;


void setup()
{
    /**
     * Initialization of the library including json based serial communication
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);

    /**
     * Add a brightness Sensor to device 0. The Brightness Sensor is a LDR
     * sensor between A0 and ground + a pull up resistor between A0 and +VCC
     * The brightness Sensor supports setting a analog read value as maximal
     * brightness. The maximal brightness will then be reported as 100% 
     * brightness by the sensor. 
     * Key 'b' reports brightness in % 
     */
    SpikeHome::addBrightnessSensor(0, LDR_SENSOR_PIN);
    
    /**
     * Add a DHT22 Sensor to device 0. The library only supports the DHT22 type, but it is easy to 
     * develop additional classes to support other DHT types.
     * key 't' reports temperature
     * key 'h' reports humidity
     * key 'r' reports read errors
     */
    SpikeHome::addDHTSensor(0, DHT_SENSOR_PIN);

    /**
     * Add a DT Sensor to device 0
     * key 's' reports temperature.
     */
    SpikeSensors::addDTSensor(0, DT_SENSOR_PIN, DT_SENSOR_NO);
}

/**
 * This is the typical loop of all programs. Changes are only made in setup.
 * Schedule calls all objects (Sensors, output devices, serial devices) regularily to give them time to do their job.
 * For a long time this is all you need to know about the loop function
 */
void loop()
{
    Schedule::nextTick();
}

