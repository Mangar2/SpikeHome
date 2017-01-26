/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    Config.cpp
 *
 * Author:  Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */


#include "Config.h"

// gets a value identified by id
value_t Config::getValue(key_t key) const {
    return mEEPROM.getValue(key);
}

void Config::setValue(key_t key, value_t value)
{
    if (mEEPROM.hasValue(key)) {
        mEEPROM.setValue(key, value);
    }
}

value_t Config::addValue(key_t id, value_t value)
{
    return mEEPROM.addValue(id, value);
}

bool Config::notifyServer(uint16_t loopCount) {
    int16_t amount = mEEPROM.getAddedEntryAmount();
    transmitEntry(loopCount);
    return ((int16_t) loopCount >= amount - 1);
}

bool Config::transmitEntry(uint16_t index) {
    key_t id      = mEEPROM.getKeyByIndex(index);
    value_t value = mEEPROM.getValueByIndex(index);
    return sendToServer(id, value);
}

// Prints settings to serial
void Config::print() const {
    mEEPROM.print();
}
