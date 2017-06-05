/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial7_Activity.ino
 * Purpouse:  Tutorial, Step 7, movement and activity
 *
 * This tutorial demonstrates the use of the movement sensors and the activity class
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


void setup()
{
    /**
     * Initialization of the library including json based serial communication
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);
    
    /**
     * Adding a movement sensor. You can buy low cost movement sensor ready for use. Search vor "movement" + "arduino" 
     * to find several offers. Movement sensors typically reports movement and holds the movement info for some time
     * As we like to control everything by the arduino set the movement report to the shortest time possible and 
     * let the activity class do the job.
     * Key 'm' current movement status reported from the movement sensor
     */
     SpikeHome::addMovementSensor(0, MOVEMENT_SENSOR_PIN);

     /**
      * Class to start activities (like swithing light). A light actor will be added in Tutorial 8. The activity
      * supports several configurations that you may set from the server
      * The logic of the activity is to be active for a short time on first movement and then increase active time
      * more and more until a maximum. The purpouse is to control a light. If you enter a room and leave it 
      * shortly the light goes off fast. If you enter a room and stay in with more moves the light keeps on 
      * longer. This reduces the risk that the light gets off if you just do not move enough to be detected.
      * Configure the values until the lights never gets off accidentally
      * Key 'K' Activity period on first movement
      * Key 'L' Increase for activity period on every additional movement (example on third movement the activity
      * period is 'K' + (3-1) * 'L' 
      * Key 'M' Maximal activity period to ensure that 'K' + (n-1) * 'L' will not get too large
      * Key 'l' current activity period
      */
     SpikeHome::addActivity(0);
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

