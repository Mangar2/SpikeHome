#ifndef __NOTIFYTARGET_H
#define __NOTIFYTARGET_H

/*
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      NotifyTarget.h
 * Purpose:   Interface class for classes receiving notifications. Three types of notifications are
 *            supported:
 *            handleChange: Changes of values (from sensors, configuration, ...)
 *            checkState:   Regularily called methods to check the state (read value from sensor, ..)
 *            notifyServer: Regularily called to notify the server for any changes
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */


#include "StdInclude.h"

class NotifyTarget {
public:

    static const bool INVERTED      = true;
    static const bool NOT_INVERTED  = false;
    static const time_t MILLISECONDS_IN_A_SECOND = 1000L;
    static const time_t MILLISECONDS_PER_LOOP    = 10L;
    static const time_t LOOPS_PER_SECOND         = MILLISECONDS_IN_A_SECOND / MILLISECONDS_PER_LOOP;

    static const uint8_t CHECKSTATE_NEVER   = 255;
    static const uint8_t CHECKSTATE_ALLWAYS = 0;
    static const uint8_t CHECKSTATE_NORMAL  = 0x07;
    static const uint8_t CHECKSTATE_SELDOM  = 0x7F;


    /**
     * Notifies the server about the current communication state of the token bases RS485 protocol. States are
     * documented in RS485State.h
     */
    static const key_t STATE_NOTIFICATION           = 'a';

    /**
     * Notifies about brightness in percent.
     */
    static const key_t BRIGHTNESS_NOTIFICATION      = 'b';

    /**
     * Notifies about clock changes. Will be reported every second
     */
    static const key_t CLOCK_NOTIFICATION           = 'c';

    /**
     * Notifies about any value currently debugging.
     */
    static const key_t DEBUG_INFO_NOTIFICATION      = 'd';

    /**
     * Notifies about read-errors of a device (DHT22)
     */
    static const key_t RECEIVE_ERROR_NOTIFICATION   = 'e';

    /**
     * Notifies about humidity. Fixed point floating value. The first byte is the integer digit, the second byte is the
     * decimal digit (0..99 => ,0 .. ,99)
     *
     */
    static const key_t HUMIDITY_NOTIFICATION        = 'h';

    /**
     * Notifies about a run out timer
     */
    static const key_t TIMER_NOTIFICATION           = 'i';

    /**
     * Notifies about light state. Sends the amount of seconds the light will be switched on. If a 0 is send the light
     * is switched off
     */
    static const key_t LIGHT_ON_NOTIFICATION        = 'l';

    /**
     * Notifies about a move. Value = 1 on movement and Value = 0 on no movement
     */
    static const key_t MOVEMENT_NOTIFICATION        = 'm';

    /**
     * Notifies about a move. Value = 1 on movement and Value = 0 on no movement. Used to distinguish movement detectors
     * "m" type and "n" type
     */
    static const key_t ENTRY_MOVEMENT_NOTIFICATION  = 'n';

    /**
     * Notifies about window state - value == 1 window is open, value == 0 window is closed
     */
    static const key_t WINDOW_OPEN_NOTIFICATION     = 'o';

    /**
     * Notifies about air pressure. The value must be multiplied by 2 to get the pressure
     */
    static const key_t AIR_PRESSURE_NOTIFICATION    = 'p';

    /**
     * Notifies about a read error of a device (currently only DHT22)
     */
    static const key_t READ_ERROR_NOTIFICATION      = 'r';

    /**
     * Notifies about a system temperature. 't' is used for room temperature and 's' is used to measure internal
     * temperatures to check system helth
     */
    static const key_t SYS_TEMPERATURE_NOTIFICATION = 's';

    /**
     * Notifies about a room temperature. 't' is used for room temperature and 's' is used to measure internal
     * temperatures to check system health
     */
    static const key_t TEMPERATURE_NOTIFICATION     = 't';

    /**
     * Notifies about PWM output voltage dimming a light. Usually for debugging purposes
     */
    static const key_t LIGHT_VOLTAGE_NOTIFICATION   = 'v';

    /**
     * Notifies about water 0 = no water 1..15 water (the higher the less resistance and thus the more water)
     */
    static const key_t WATER_NOTIFICATION           = 'w';

    /**
     * Notifies about an acivity
     */
    static const key_t ACTIVITY_NOTIFICATION         = 'y';


    /**
     * Notifies about the space between heap and stack for the running sketch. Used for debugging
     */
    static const key_t MEM_LEFT_NOTIFICATION        = 'z';

    /**
     * Address of the device (2..127). 0 is reserved for broadcast and 1 is reserved for the server/pc
     */
    static const key_t ADDRESS_KEY                  = 'A';

    /**
     * Maximum brightness in percent when light will not turn on (minimal 10%).
     */
    static const key_t MAXIMUM_BRIGHTNESS_KEY       = 'B';
    static const key_t CLOCK_KEY                    = 'C';

    /**
     * Brighness adjust on evening event
     */
    static const key_t EVENING_BRIGHTNESS_KEY       = 'D';

    static const key_t LED_STATUS_KEY               = 'E';
    static const key_t FS20_COMMMAND                = 'F';
    /**
     * Time in seconds between two infos send from the arduino if nothing interessting happens
     */
    static const key_t CONFIG_INFO_PERIOD_KEY       = 'G';
    /**
     * Minimal AnalogWrite value where the lights are fully on
     */
    static const key_t FULL_ON_VOLTAGE_KEY          = 'H';
    /**
     * Brighness target to achieve in percent. Lower it to dim lights
     */
    static const key_t TARGET_BRIGHTNESS_KEY        = 'I';

    /**
     * Analog read value measured by the brightness sensor, when light is fully on.
     * The brightness sensor will report brightness relative to this setting.
     */
    static const key_t FULL_ON_VALUE_KEY            = 'J';

    /**
     * Activity period on first movement
     */
    static const key_t INIT_LIGHT_TIME_KEY          = 'K';

    /**
     * Time an activity is set ot active on additional moves. Times are always added until the activity deactivates.
     */
    static const key_t INC_LIGHT_TIME_KEY           = 'L';

    /**
     * Maximal time an activity is active without additional moves
     */
    static const key_t MAX_LIGHT_TIME_KEY           = 'M';

    /**
     * Brightness setting at night.
     */
    static const key_t NIGHT_BRIGHTNESS_KEY         = 'N';

    /**
     * Minimal value where light is not off (minimally on)
     */
    static const key_t START_VOLTAGE_KEY            = 'O';
    /**
     * Delay in milliseconds between two dimming steps
     */
    static const key_t DIMMING_DELAY_KEY            = 'P';
    /**
     * Command to measure light intensity and adjust the settings
     * {"R":30, "K": "Q", "V":1}
     */
    static const key_t ADJUST_LIGHT                 = 'Q';
    static const key_t ROLLER_SHUTTER_KEY           = 'R';

    /**
     * Address of the server
     */
    static const key_t SERVER_ADDRESS_KEY           = 'S';
    static const key_t ROLLER_TIME_KEY              = 'T';
    
    /**
     * Switches the light on for a time period in seconds or off (0)
     */
    static const key_t SET_LIGHT_TIME               = 'V';

    /**
     * Additional time in "increases" an activity will last if it is triggered in a non active status.
     * Use a value > 0, if the activity should always be activated by a sensor placed near the entrance.
     * If it is activated without directly a person is already in the room and the light went off nevertheless.
     */
    static const key_t ROOM_SENSOR_ADD_INC_KEY      = 'W';

    /**
     * Bitmask of the switch status. Every bit corresponds to one switch.
     */
    static const key_t SWITCH_STATUS_KEY            = 'X';

    /**
     * Key of the currently installed software version (send only)
     */
    static const key_t SOFTWARE_VERSION_KEY         = 'Z';

    NotifyTarget(device_t deviceNo = 0)
    : mDeviceNo(deviceNo), mCheckMask(CHECKSTATE_NEVER)
    { }

    /**
     * Signal a change. Register change type so that the class is informed on change
     * @param senderAddress address of the sender
     * @param key key/identifier of the change
     * @param data new value
     */
    virtual void handleChange(address_t senderAddress, key_t key, StateValue data) { }

    /**
     * Checks the state. Once the object is registered, this function will be called regularily in the loop.
     * @param loops number of checkState loops since reboot
     */
    virtual void checkState(time_t loops) { }

    /**
     * remembers the class to send notifications to the server. This function will be called
     * regularily, once the object is registered.
     * @param loopCount amount of notify loops already passed. May be used to select different values to send
     * @return true, if all notification send
     */
    virtual bool notifyServer(uint16_t loopCount)
    {
        return true;
    }

    /**
     * Sets the moule number for multiple module installations
     * @pram deviceNo number of the device this object belongs to
     */
    void setDeviceNo(device_t deviceNo)
    {
        mDeviceNo = deviceNo;
    }

    /**
     * Gets the number of the device this object belongs to
     * @return number of device
     */
    device_t getDeviceNo() {
        return mDeviceNo;
    }

    /**
     * Gets a configuration value from current device setting
     * @param key identifier of the value
     * @return value
     */
    value_t getConfigValue(key_t key);

    /**
     * Sets a configuration value to the current device setting
     * @param key indentifier of the value
     * @param value the new value
     */
    void setConfigValue(key_t key, value_t value);

    /**
     * Adds a configuration value to the current device setting
     * @param key indentifier of the value
     * @param value initial value, if the current config value is not yet existant
     */
    value_t addConfigValue(key_t key, value_t value);

    /**
     * Sends a new value information to a device via RS485 interface
     * @param key indentifier of the value
     * @param value new value
     * @param receiverAddress address to receive the notification
     * @return true, if it has been send
     */
    bool sendToAddress(key_t key, StateValue value, address_t receiverAddress);

    /**
     * Sends a new value information to the server via RS485 interface
     * @param key indentifier of the value
     * @param value new value
     * @return true, if it has been send
     */
    bool sendToServer(key_t key, StateValue value);

    /**
     * Broadcasts a new value information in the current device and via RS485 interface
     * @param key indentifier of the value
     * @param value new value
     * @return true, if it has been send
     */
    bool broadcast(key_t key, StateValue value);

    /**
     * Sends a notifycation to all other objects in the current device
     * @param key indentifier of the value
     * @param value the new value
     */
    void notify(key_t key, StateValue value);

    /**
     * Sends a notifycation to all other objects in all devices
     * @param key indentifier of the value
     * @param value the new value
     */
    void notifyAllDevices(key_t key, StateValue value);


    /**
     * Gets the next notification target in the list
     * @return pointer to next target
     */
    NotifyTarget* getNext()
    {
        return next;
    }

    /**
     * Sets the next notification target in the list
     * @param target next notification target
     */
    void setNext(NotifyTarget* target)
    {
        next = target;
    }

    /**
     * Sets the bit mask to check if checkState will be called.
     * the checkstate function will be called if schedule loop cound & mask == 0.
     * Exception: value 255 signal no calls at all
     * @param mask new mask to set
     */
    void setCheckMask(uint8_t mask)
    {
        mCheckMask = mask;
    }

    /**
     * Gets the bit mask to check if checkState will be called.
     * @return check mask used by schedule
     */
    uint8_t getCheckMask()
    {
        return mCheckMask;
    }


private:

    device_t mDeviceNo;
    NotifyTarget* next;
    uint8_t  mCheckMask;
};

#endif // __NOTIFYTARGET_H
