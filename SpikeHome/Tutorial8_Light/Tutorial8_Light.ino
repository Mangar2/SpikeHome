/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial8_Light.ino
 * Purpouse:  Tutorial, Step 8, light (with movement and activity)
 *
 * This tutorial demonstrates the use of the light class 
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
 * You can set SOFTWARE_VERSION to any type you like. I think it is handy to know wich software version is currently
 * running on a home automation device. So increase the number when you upload a new version.
 */
const int SOFTWARE_VERSION      = 1;
/**
 * Devices are a basic concept of the library. Devices are used to separate objects
 * for example to group sensors of different rooms.
 */
const int DEVICE_AMOUNT         = 1;

/**
 * Serial speed to communicate, usually 9600
 */
const long SERIAL_SPEED         = 9600;

const pin_t MOVEMENT_SENSOR_PIN = 12;
const pin_t BRIGHTNESS_SENSOR_PIN = A0;
const pin_t LIGHT_PWM_PIN = 10;


void setup()
{
    /**
     * Initialization of the library including json based serial communication
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);
    
    /**
     * Add movement sensor. 
     */
     SpikeHome::addMovementSensor(0, MOVEMENT_SENSOR_PIN);

     /**
      * Add Activity.
      */
     SpikeHome::addActivity(0);

     /**
      * The light is the most complex actor in the library. First "light" is a subclass of
      * the Brightness class, thus it adds a brightness sensor. 
      * The light class is able to dim a "non dimmable" low-vlotage LED (typically GU5.3
      * spots with 12V DC.
      * To build this you need a low pass filter for the PWM (4,7K resistor between PWM output and 
      * gate of a MOSFET Transistor + 1 mikro Fahrad between MOSTFET Gate and Ground. The MOSTFET
      * must have a suitable 
      */
     SpikeHome::addLight(0, BRIGHTNESS_SENSOR_PIN, LIGHT_PWM_PIN);
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

