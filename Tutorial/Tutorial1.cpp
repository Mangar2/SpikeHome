/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Tutorial1.cpp
 * Purpouse:  Tutorial, Step 1, framework only
 *
 * This tutorial just uses the framework without any action. It shows what is
 * needed to use the classes.
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


void setup()
{
    /**
     * Initialization of the library including json based serial communication
     * with the PC.
     * Use a serial monitor to view the output the tutorial produces
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);
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

