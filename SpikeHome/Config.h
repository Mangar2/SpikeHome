/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:    Config.h
 * Purpose: Stores configuration settings for a device and defines configuration key values
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __CONFIG_H
#define __CONFIG_H


#include "NotifyTarget.h"
#include "EEPROMManager.h"

class Config : public NotifyTarget {

public:


    Config() : mEEPROM(100) {}

    /**
     * Gets a value identified by id
     * @param key identifier of the value
     * @returns value to store
     */
    value_t getValue(key_t key) const;

    /**
     * Sets a value identified by id
     * @param key identifier of the value
     * @param value value to store
     */
    void setValue(key_t key, value_t value);

    /**
     * Adds a value to the configuration. If the value is already there, nothing is done
     * If the value is in the cache but not in EEPROM the cache value will be used.
     * @param id identifier of the value (from 'A' to 'Z')
     * @param value initial value, only used if value not already set
     * @return current value of the configuration
     */
    value_t addValue(key_t id, value_t value);

    /**
     * Notifys the server about current settings. Sends only one element in each call
     * to not overload the network.
     * @param loopCount amount of notify loops already passed. May be used to select different values to send
     * @return true, if notification was successful
     */
    virtual bool notifyServer(uint16_t loopCount);

    // Prints settings to serial
    void print() const;

    /**
     * Gets the internal EEPROM Manager
     * @return
     */
    EEPROMManager& getEEPROM()
    {
        return mEEPROM;
    }

private:
    /**
     * Transmit one setting
     * @param pos position index for the value in the EEPROM
     * @return
     */
    bool transmitEntry(uint16_t pos);

    EEPROMManager mEEPROM;

};

#endif // __CONFIG_H
