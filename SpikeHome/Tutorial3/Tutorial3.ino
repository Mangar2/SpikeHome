/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial3.ino
 * Purpouse:  Tutorial, Step 3, devices
 *
 * This tutorial will add a second device, devices seperates sensors/actors in packages 
 * They can be addressed separately as every device has his own address. Now the two pins can be
 * controlled separately
 * First set the address of the device 1 to 20 and device 2 to 21 (see tutorial 1 if you do not know
 * how to do it)
 * Set pin 12 to high:
 * {"R":20, "K":"X", "V":1}
 * Set pin 13 to high (pin 12 is not affected)
 * {"R":21, "K":"X", "V":1}
 * Set both pins to low (Remember Address 0 = Broadcast)
 * {"R":0, "K":"X", "V":1}
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
const int DEVICE_AMOUNT         = 2;

/**
 * Serial speed to communicate, usually 9600
 */
const long SERIAL_SPEED         = 9600;


void setup()
{
    /**
     * Initialization of the library including json based serial communication
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);
    /**
     * Adding a switch to device 0 with PIN12. 
     */
    SpikeHome::addSwitches(0, Switches::PIN12);
    /**
     * Adding a switch to device 1 with PIN13. 
     */
    SpikeHome::addSwitches(1, Switches::PIN13);
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

