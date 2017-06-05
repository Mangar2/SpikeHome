/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:        EEPROMManager.cpp
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

#define DEBUG
#include <avr/eeprom.h>
#include "EEPROMManager.h"

pos_t EEPROMManager::mFreePos = 0;

EEPROMManager::EEPROMManager(pos_t maxEntries)
{
    mStartPos   = mFreePos;
    mMaxEntries = maxEntries;
    mAddedEntries  = 0;
    mFreePos += (maxEntries + 2) * ENTRY_SIZE;
}

value_t EEPROMManager::getValue(key_t key) const
{
    pos_t pos = findPos(key);
    value_t res = 0;
    if (pos != NOT_FOUND) {
        res = getValueByPos(pos + ID_SIZE);
    }
    return res;
}

bool EEPROMManager::hasValue(key_t key) const
{
    return findPos(key) != NOT_FOUND;
}

bool EEPROMManager::setValue(key_t key, value_t value)
{
      pos_t pos = findPos(key);
      bool res = false;

      if (pos != NOT_FOUND) {
          res = setValueByPos(pos + ID_SIZE, value);
          if (!res) {
              writeData(pos, INVALID_CELL);
              res = append(key, value);
          }
      }
      return res;
}

void EEPROMManager::insertEntryByPos(pos_t pos, key_t key, value_t value, bool moveCurrent)
{
    bool  entrySet   = false;
    pos_t maxPos     = calcPosByIndex(mMaxEntries - 1);
    // loop handling the insert
    while (!entrySet && pos <= maxPos) {

        if (moveCurrent) {
            append(getKeyByPos(pos), getValueByPos(pos + ID_SIZE));
        }
        entrySet = setEntryByPos(pos, key, value);

        mAddedEntries++;

        pos += ENTRY_SIZE;
        moveCurrent = true;
    }

}

value_t EEPROMManager::addValue(key_t key, value_t value)
{
    pos_t curPos = findPos(key);
    pos_t newPos = calcPosByIndex(mAddedEntries);
    bool  entryMoved = false;

    value_t newValue;

    if (curPos != NOT_FOUND) {
        newValue  = getValueByPos(curPos + ID_SIZE);
        if (curPos == newPos) {
            // The key is exactly at the position it should be, only increase the entries
            mAddedEntries++;
        } else if (curPos > newPos) {
            // The key has been stored before, move it and then insert it
            entryMoved = moveEntryByPos(newPos, curPos);
            insertEntryByPos(newPos, key, newValue, !entryMoved);
        }
    } else {
        // The key has not been found, insert it
        insertEntryByPos(newPos, key, value, true);
        newValue = value;
    }

    return newValue;
}

bool EEPROMManager::moveEntryByPos(pos_t from, pos_t to)
{
    bool res = true;
    if (from != to) {
        key_t key = getKeyByPos(from);
        value_t value = getValueByPos(from + ID_SIZE);
        if (!setEntryByPos(to, key, value)) {
            append(key, value);
        }
    }
    return res;
}

bool EEPROMManager::append(key_t key, value_t value)
{
    bool writeCheckOk;
    pos_t maxPos     = calcPosByIndex(mMaxEntries - 1);
    pos_t pos;

    do {
        pos_t amount = getEntryAmount();
        pos = calcPosByIndex(amount);
        if (pos <= maxPos) {
            setEntryAmount(amount + 1);
            writeCheckOk = setEntryByPos(pos, key, value);
        }
    } while (!writeCheckOk && pos <= maxPos);
    return writeCheckOk;
}

pos_t EEPROMManager::findPos(key_t key) const {
    pos_t i;
    pos_t pos;
    pos_t res = NOT_FOUND;
    pos_t amount = getEntryAmount();
    for (i = 0, pos = calcPosByIndex(0); i < amount; i++, pos += ENTRY_SIZE) {

        if (getKeyByPos(pos) == key) {
            res = pos;
            break;
        }

    }
    return res;
}

EEPROMManager::eeprom_t EEPROMManager::readData(pos_t pos) const
{
    //return EEPROM.read(pos);
    return eeprom_read_byte( (uint8_t*) pos );
}

bool EEPROMManager::writeData(pos_t pos, eeprom_t element)
{
    if (readData(pos) != element) {
        eeprom_write_byte( (uint8_t*) pos, element );
        //EEPROM.write(pos, element);
    }

    return readData(pos) == element;
}

bool EEPROMManager::setValueByPos(pos_t pos, value_t newValue) {

    return writeData(pos, (eeprom_t) newValue) && writeData(pos + 1, (eeprom_t) (newValue / 256));
}

value_t EEPROMManager::getValueByPos(pos_t pos) const
{
    value_t res = readData(pos + 1);
    res *= 256;
    res += readData(pos);

    return res;
}

bool EEPROMManager::setEntryByPos(pos_t pos, key_t key, value_t value)
{
    bool writeOK = false;

    if (pos <= calcPosByIndex(mMaxEntries)) {
        writeOK = setKeyByPos(pos, key) && setValueByPos(pos + ID_SIZE, value);
        if (!writeOK) {
            // writes key = 0 to the defect key/value store. This only works if key can be written
            // We assume that keys are seldom written thus they will not be defect
            writeData(pos, INVALID_CELL);
        }
    }

    return writeOK;
}

pos_t EEPROMManager::getEntryAmount() const
{
    return readData(mStartPos);
}

value_t EEPROMManager::getValueByIndex(pos_t index) const
{
    value_t value = 0;
    if (index < getEntryAmount()) {
        pos_t pos = calcPosByIndex(index);
        value = getValueByPos(pos + ID_SIZE);
    }
    return value;
}

key_t EEPROMManager::getKeyByIndex(pos_t index) const
{
    key_t key = 0;
    if (index < getEntryAmount()) {
        pos_t pos = calcPosByIndex(index);
        key = getKeyByPos(pos);
    }
    return key;
}

void EEPROMManager::setEntryAmount(pos_t amount)
{
    writeData(mStartPos, amount);
}

key_t EEPROMManager::getKeyByPos(pos_t pos) const
{
    key_t res = 0;
    for (uint8_t i = 0; i < ID_SIZE; i++) {
        res *= 256;
        res += readData(pos + i);
    }
    return res;
}

bool EEPROMManager::setKeyByPos(pos_t pos, key_t key)
{
    bool res = true;
    for (int8_t i = ID_SIZE - 1; i >= 0; i--) {
        res = res && writeData(pos + i, (eeprom_t) key);
        key /= 256;
    }
    return res;
}


pos_t EEPROMManager::calcPosByIndex(pos_t index) const
{
    return mStartPos + sizeof(pos_t) + HEADER_SIZE + ENTRY_SIZE * index;
}

void EEPROMManager::clear()
{
    setEntryAmount(0);
    resetInsertPos();
}

void EEPROMManager::shrink()
{
    setEntryAmount(mAddedEntries);
}

void EEPROMManager::print() const
{
      printVariableIfDebug(mStartPos);
      pos_t entryAmount = getEntryAmount();
      printVariableIfDebug(entryAmount);
      printVariableIfDebug(mMaxEntries);
      printlnIfDebug("");
      for (pos_t i = 0; i < entryAmount; i++) {
            key_t key = getKeyByIndex(i);
            if (key >= '0' && key <= 'z') {
                printIfDebug((char)key);
            } else {
                printIfDebug(key);
            }
            printIfDebug("->");
            printlnIfDebug(getValueByIndex(i));
      }
}
