/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial1.cpp
 * Purpouse:  Tutorial, Step 1, framework only
 *
 * This tutorial just uses the framework without any action. It shows what is
 * needed to use the classes.
 * 
 * Addresses: 
 * Addresses are numbers from 1 to 127. The address 0 is reserved for broadcasts. Addresses gets
 * important if you use the RS485 bus with many attached Arduinos. It is comparable with a simple
 * IP Address.
 * 
 * Open the serial monitor
 * Watch the output of the sketch. It will print Informations in Json Format. The meaning of the 
 * elements:
 * "S" (Sender-Address)   : The current address of your Home Automation device
 * "R" (Receiver-Address) : The address of the server/your computer. Usually = 1
 * "A" (Acknowledge)      : True (1) if the sender request an acknowledgement of the message, else False (0)
 * "K" (Key)              : Key or identifier of the value
 * "V" (Value)            : The Value
 * 
 * You can see the following Keys already. Additional classes will add additional type of keys
 * "A"                    : Address of the device
 * "Z"                    : Version of the software currently running (see SOFTWARE_VERSION constant)
 * "G"                    : Time in seconds between two standard messages 
 * "S"                    : Address of the server the arduino will send messages 
 * 
 * Set the address to 20 by entering
 * {"K": "A", "V": 20}
 * Enter the following to get a standard message every second
 * {"R": 20, "K": "G", "V": 1}
 * Enter a wrong receiver address. Nothing happens ... as you send the command to Address 21
 * {"R": 21, "K": "G", "V": 5}
 * You can reset the device or reload the sketch. The settings will still be present. They are stored
 * in the ROM of the device.
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

