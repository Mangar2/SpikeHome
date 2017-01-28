/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Initialization.h
 * Purpose:   Initializes environment and sensors. Pure static class
 *
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 28. Dezember 2016, 07:51
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __INITIALIZATION_H
#define	__INITIALIZATION_H

#include "StdInclude.h"
#include "Switches.h"
#include "Schedule.h"

class BinarySensor;

class SpikeHome {
public:

    /**
     * Initializes all, call it in the setup function of the main program
     * @param softwareVersion version number of the software. It can be set to whatever number you like
     * @param deviceAmount amount of subdevices to create. Each subdevice hat its own configuration.
     */
    static void init(value_t softwareVersion, device_t deviceAmount);

    /**
     * Initializes all for a RS485 interface, call it in the setup function of the main program
     * @param softwareVersion version number of the software. It can be set to whatever number you like
     * @param deviceAmount amount of subdevices to create. Each subdevice hat its own configuration.
     * @param serialSpeed speed in bits per second of the serial device
     * @param readWritePin pin to select between read and write mode
     */
    static void initRS485(value_t softwareVersion, device_t deviceAmount, time_t serialSpeed, pin_t readWritePin);

    /**
     * Initializes all with a RS485 interface, call it in the setup function of the main program
     * @param softwareVersion version number of the software. It can be set to whatever number you like
     * @param deviceAmount amount of subdevices to create. Each subdevice hat its own configuration.
     * @param serialSpeed speed in bits per second of the serial device
     */
    static void initTextIO(value_t softwareVersion, device_t deviceAmount, time_t serialSpeed);

    /**
     * registers a notification target for change notifications of a
     * device.
     * @param deviceNo number of the device to add the notification target
     * @param pTarget sensor to register
     * @return pTarget, to use for further function calls.
     */
    static NotifyTarget* onChange(device_t deviceNo, NotifyTarget* pTarget);

    /**
     * registers a notification target for change notifications of a
     * device. The device nuber is read from the object
     * @param pTarget sensor to register
     * @return pTarget, to use for further function calls.
     */
    static NotifyTarget* onChange(NotifyTarget* pTarget);

    /**
     * Adds a notification target to the scheduling loop that is called regularily
     * @param pTarget pointer to a NotifyTarget class
     * @return pTarget, to use for further function calls.
     */
    static NotifyTarget* addToSchedule(NotifyTarget* pTarget);

    /**
     * Creates an light activity object and registeres it to the device
     * @param deviceNo number of the device to add the activity
     */
    static NotifyTarget* addActivity(device_t deviceNo);

    /**
     * Creates an analog sensor and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @param invert, true, if sensor reads inverted values
     * @param notifyKey key used to send notifications
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addAnalogSensor(device_t deviceNo, pin_t pin, bool invert, key_t notifyKey);

    /**
     * Creates a binary sensor and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @param invert, true, if sensor reads inverted values
     * @param notifyKey key used to send notifications
     * @return newly created and registered sensor object
     */
    static BinarySensor* addBinarySensor(device_t deviceNo, pin_t pin, bool invert, key_t notifyKey);

    /**
     * Creates a brightness sensor object and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addBrightnessSensor(device_t deviceNo, pin_t pin);


    /**
     * Creates a temperature, humidity sensor DHT22 and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     */
    static NotifyTarget* addDHTSensor(device_t deviceNo, pin_t pin);

    /**
     * Creates an FS20UART device to communicate with a FS20UART
     * @param deviceNo number of the device the UART is attached to
     * @param rxPin software serial rx pin
     * @param txPin software serial tx pin
     */
    static NotifyTarget* addFS20UART(device_t deviceNo, pin_t rxPin, pin_t txPin);


    /**
     * Adds a light device including brightness measurement and PWM controlled LED light
     * @param deviceNo number of the device to add the light
     * @param brightnessPin pin the brightness sensor is attached to
     * @param pwmPin pwm output pin for the LEDs. Make sure to amplify it correctly. Mostfet is recommended...
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addLight(device_t deviceNo, pin_t brightnessPin, pin_t pwmPin);

    /**
     * Creates a movement sensor object and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @param key key the movement sensor sends on activation. Usually selected:
     * "a" for activity => directly turns on/off a "light" object
     * "m" for move => used by activity object to calculate light on time (more moves, longer time)
     * "n" for "entry move" => movement sensor at the entry of a larger room. Switches light on for a short period.
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addMovementSensor(device_t deviceNo, pin_t pin, key_t key = NotifyTarget::MOVEMENT_NOTIFICATION);

    /**
     * Creates a roller shutter and registeres it to count and notify
     * @param deviceNo number of the device to add the shutter
     * @param powerPin pin to switch power of the roller on/off
     * @param directionPin pin to switch the diretion relais
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addRollerShutter(device_t deviceNo, pin_t powerPin, pin_t directionPin);

    /**
     * Shows the status of a binary object sending change Notifications by setting a pin to high/low
     * @param deviceNo number of the device of the sensor/object
     * @param pin pin to set LOW/HiGH on status
     * @param statusKey key of the binary object to show status
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addStatusLED(device_t deviceNo, pin_t statusPin, key_t statusKey);

    /**
     * Creates an object that switches pins on or off and registeres it to Schedule and Notifiy
     * @param deviceNo number of the device to add the switch handler
     * @param pinBitMaskLSBD2 bitmask which pins will be used for switching. The Least Significant Byte
     * enables/disables the Pin 2. Pin 2 to Pin 13 can be created. Use the constants defined in Switches.h
     * Example: PIN02 + PIN13 will use PIN02 and PIN13 for digital IO
     * @return newly created and registered object
     */
    static NotifyTarget* addSwitches(device_t deviceNo, uint16_t pinBitMaskLSBD2);

    /**
     * Creates a water sensor object and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addWaterSensor(device_t deviceNo, pin_t pin);

    /**
     * Creates an open window object and registeres it to the device
     * @param deviceNo number of the device to add the sensor
     * @param pin pin the sensor is attached to
     * @return newly created and registered sensor object
     */
    static NotifyTarget* addWindowSensor(device_t deviceNo, pin_t pin);


private:
    /**
     * Do not create an Initialization class
     */
    SpikeHome();



};

#endif	/* __INITIALIZATION_H */
