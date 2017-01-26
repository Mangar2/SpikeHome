/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Device.h
 * Purpose:   Device singelton to access configuration and notify classes
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */


#ifndef __DEVICE_H
#define __DEVICE_H

#include "StdInclude.h"
#include "Notify.h"
#include "Config.h"
#include "SerialIO.h"

class EEPROManager;

class Device {
public:

    /**
     * Initializes the device, must be called before any use of this toolbox
     * @param softwareVersion version number of the software. It can be set to whatever number you like
     * @param deviceAmount amount of subdevices to create. Each subdevice hat its own configuration,
     * address and objects attached to it.
     */
    static void init(value_t softwareVersion, device_t deviceAmount = MAX_DEVICE_AMOUNT);

    /**
     * Gets a configuration class
     * @param index index of the class to get
     * @return configuration class
     */
    static Config& getConfig(device_t index);

    /**
     * Gets a notification class
     * @param index index of the class to get
     * @return notify class
     */
    static Notify& getNotify(device_t index);

    /**
    * Gets a configuration value identified by configuration index and key
    * @param index device nubmer
    * @param key identifier of the value
    * @returns configuration value
     */
    static value_t getConfigValue(device_t index, key_t key)
    {
        return getConfig(index).getValue(key);
    }

    /**
    * Sets a configuration value identified by configuration index and key
    * @param index device nubmer
    * @param key identifier of the value
    * @param value value to store
     */
    static void setConfigValue(device_t index, key_t key, value_t value)
    {
        getConfig(index).setValue(key, value);
    }

    /**
     * Adds a value to the configuration. If the value is already there, nothing is done
     * If the value is in the cache but not in EEPROM the cache value will be used.
     * @param index device nubmer
     * @param id identifier of the value (from 'A' to 'Z')
     * @param value initial value, only used if value not already set
     */
    static value_t addConfigValue(device_t index, key_t id, value_t value)
    {
        return getConfig(index).addValue(id, value);
    }

    /**
    * Registers an activity to receive a change info of a certain type
    * @param index device nubmer
    * @param Target* pTarget class to send the on change notification
    * @returns value to store
     */
    static void onChange(device_t index, NotifyTarget* pTarget)
    {
        getNotify(index).onChange(pTarget);
    }

    /**
     * Finds the environment number of an address and returns the environment
     * number. If no environment is found it returns -1
     * param address adress to search
     */
    static device_t addressToIndex(value_t address);

    /**
    * gets the type of the sensor stored in EEPROM. Set if currently zero
    */
    static value_t readSensorType();

    /**
    * sets the type of the sensor and stores it in EEPROM
    */
    static void storeSensorType(value_t sensorType);

    /**
     * Gets the amount of active devices
     */
    static device_t getDeviceAmount()
    {
        return mDeviceAmount;
    }

    /**
     * Sets the io handler for the device
     * @param serial the class handling the input/output of the device.
     */
    static void setIOHandler(SerialIO* serial)
    {
        mpSerial = serial;
    }

    /**
     * Gets the io handler for the device
     * @return the class handling the input/output of the device.
     */
    static SerialIO* getIOHandler()
    {
        return mpSerial;
    }

    static void print()
    {
        mEEPROM.print();
    }

private:

    static device_t mDeviceAmount;
    static Config mConfig[MAX_DEVICE_AMOUNT];
    static Notify mNotify[MAX_DEVICE_AMOUNT];
    static EEPROMManager mEEPROM;
    static SerialIO* mpSerial;

};

#endif // __DEVICE_H
