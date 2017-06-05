/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. 
 * It is furnished "as is", without any support, and with no warranty, express or implied, as to its 
 * usefulness for any purpose.
 *
 * File:      Tutorial5_Blink.ino
 * Purpouse:  Tutorial, Step 5, adding a new actor
 *
 * This tutorial will show how to add a new actor not already included. I use a simple class letting
 * an LED blink
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

/**
 * Pin the led is attached to. We are using the internal LED usually on pin 13
 */
const pin_t LED_PIN             = 13;

/**
 * The blink class. Derive any new class from NotifyTarget. It can then be 
 * registered to Schedule. For sensors you may use the "State" a subclass of 
 * NotifyTarget if you define sensors that should report changes fast to 
 * the server. 
 */
 
class Blink : public NotifyTarget
{
public:
    Blink(device_t deviceNo) : NotifyTarget(deviceNo)
    {
        // Arduino standard call
        pinMode(LED_PIN, OUTPUT);
        // Ensures that blink is allways called by schedule.
        NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_ALLWAYS);
        blinks = 0;
    }

    /**
     * Called 100 times per seconds. Switches the LED on/off each seconds and notifies the server
     * @param scheduleLoops amount of calls allready made.
     */
    virtual void checkState(time_t scheduleLoops)
    {
        if (scheduleLoops % 100 == 0) {
            int ledState = scheduleLoops % 200 == 0 ? HIGH : LOW;
            digitalWrite(LED_PIN, ledState);
            if (ledState == HIGH) {
                blinks++;
            }
            // look at the monitor. It will show the on/off state in a json format. We look at the format later
            NotifyTarget::sendToServer('l', ledState);
        }
    }

    /**
     * Called to send notifications to the server. This function will be called
     * regularily, once the object is registered but there might be lots of seconds between calls.
     * @param loopCount the value is increased with each call until the method returns true.
     * Used to send different values and only one per call
     * @return true, if all available values/notification has been send to the server. If you
     * have only one then just return true
     */
    virtual bool notifyServer(uint16_t loopCount)
    {
        NotifyTarget::sendToServer('b', blinks);
        return true;
    }

    value_t blinks;
    
};

void setup()
{
    /**
     * Initialization of the library including json based serial communication
     */
    SpikeHome::initTextIO(SOFTWARE_VERSION, DEVICE_AMOUNT, SERIAL_SPEED);

    /**
     * Register the blink class
     */
    SpikeHome::addToSchedule(new Blink(0));
    
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

