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

    static const uint8_t CHECKSTATE_NEVER   = 255;
    static const uint8_t CHECKSTATE_ALLWAYS = 0;
    static const uint8_t CHECKSTATE_NORMAL  = 0x07;
    static const uint8_t CHECKSTATE_SELDOM  = 0x7F;


    static const key_t STATE_NOTIFICATION           = 'a';
    static const key_t BRIGHTNESS_NOTIFICATION      = 'b';
    static const key_t CLOCK_NOTIFICATION           = 'c';
    static const key_t DEBUG_INFO_NOTIFICATION      = 'd';
    static const key_t RECEIVE_ERROR_NOTIFICATION   = 'e';
    static const key_t HUMIDITY_NOTIFICATION        = 'h';
    static const key_t TIMER_NOTIFICATION           = 'i';
    static const key_t LIGHT_ON_NOTIFICATION        = 'l';
    static const key_t MOVEMENT_NOTIFICATION        = 'm';
    static const key_t ENTRY_MOVEMENT_NOTIFICATION  = 'n';
    static const key_t WINDOW_OPEN_NOTIFICATION     = 'o';
    static const key_t AIR_PRESSURE_NOTIFICATION    = 'p';
    static const key_t READ_ERROR_NOTIFICATION      = 'r';
    static const key_t SYS_TEMPERATURE_NOTIFICATION = 's';
    static const key_t TEMPERATURE_NOTIFICATION     = 't';
    static const key_t LIGHT_VOLTAGE_NOTIFICATION   = 'v';
    static const key_t WATER_NOTIFICATION           = 'w';
    static const key_t MEM_LEFT_NOTIFICATION        = 'z';
    static const key_t ADDRESS_KEY                  = 'A';
    static const key_t MAXIMUM_BRIGHTNESS_KEY       = 'B';
    static const key_t CLOCK_KEY                    = 'C';
    static const key_t LED_STATUS_KEY               = 'E';
    static const key_t FS20_COMMMAND                = 'F';
    static const key_t CONFIG_INFO_PERIOD_KEY       = 'G';
    static const key_t FULL_ON_VOLTAGE_KEY          = 'H';
    static const key_t TARGET_BRIGHTNESS_KEY        = 'I';
    static const key_t FULL_ON_VALUE_KEY            = 'J';
    static const key_t INIT_LIGHT_TIME_KEY          = 'K';
    static const key_t INC_LIGHT_TIME_KEY           = 'L';
    static const key_t MAX_LIGHT_TIME_KEY           = 'M';
    static const key_t START_VOLTAGE_KEY            = 'O';
    static const key_t DIMMING_DELAY_KEY            = 'P';
    static const key_t ADJUST_LIGHT                 = 'Q';
    static const key_t ROLLER_SHUTTER_KEY           = 'R';
    static const key_t SERVER_ADDRESS_KEY           = 'S';
    static const key_t ROLLER_TIME_KEY              = 'T';
    static const key_t SENSOR_INTERVAL_KEY          = 'U';
    static const key_t SET_LIGHT_TIME               = 'V';
    static const key_t SWITCH_STATUS_KEY            = 'X';
    static const key_t SOFTWARE_VERSION_KEY         = 'Z';

    NotifyTarget(device_t deviceNo = 0)
    : mDeviceNo(deviceNo), mCheckMask(CHECKSTATE_NEVER)
    { }

    /**
     * Signal a change. Register change type so that the class is informed on change
     * @param key key/identifier of the change
     * @param data new value
     */
    virtual void handleChange(key_t key, StateValue data) { }

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
     * Sends a notifycation to all other objects in the current device
     * @param key indentifier of the value
     * @param value the new value
     */
    void notify(key_t key, StateValue value);

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
