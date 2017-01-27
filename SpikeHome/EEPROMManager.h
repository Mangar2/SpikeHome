/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:        EEPromManager.h
 * Purpose:     Provides a key/value store in the EEPROM, the key has one byte, the value two bytes
 *              The key/values are store consecutive. It needs to search through the list of entries
 *              to find a key. The keys are not sorted.
 *              It has a simple mechanism to skip cells where the value cannot be stored due to a
 *              hardware defect of the corresponding cell. key = 0 is reserved to mark defect cells!!
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __EEPROMMANAGER_H
#define __EEPROMMANAGER_H

#include "StdInclude.h"

class EEPROMManager {

public:

    typedef uint8_t eeprom_t;

    /**
     * Creates a new manager
     * @param maxEnties maximum amount of entries available for the manager.
     */
    EEPROMManager(pos_t maxEntries = 26);

    /**
     * Gets a value identified by id
     * @param key identifier of the value, key = 0 is not allowed as 0 is reserved for defect cells
     * @return value of the id or 0 if id has not been found
     */
    value_t getValue(key_t key) const;


    /**
     * Checks if a value is available in the EEPROM
     * @param key identifier of the value, key = 0 is not allowed as 0 is reserved for defect cells
     * @return true, if the id is stored, else false
     */
    bool hasValue(key_t key) const;

    /**
     * Sets a value identified by key. Does nothing, if key is not found
     * @param key identifier of the value, key = 0 is not allowed as 0 is reserved for defect cells
     * @param value value to store
     * @return true, if value has been set, else returns false
     */
    bool setValue(key_t key, value_t value);

    /**
     * Adds a value to the EEPROM. If the value is already there, nothing is done, the value will NOT be changed
     * @param key identifier of the value, key = 0 is not allowed as 0 is reserved for defect cells
     * @param value initial value, only used if value not already set
     * @return value of the element added
     */
    value_t addValue(key_t key, value_t value);

    /**
     * Gets a value by index
     * @param index of the entry
     */
    value_t getValueByIndex(pos_t index) const;

    /**
     * Gets an id by index
     * @param index of the entry
     */
    key_t getKeyByIndex(pos_t index) const;

    /**
     * Deletes all entries from the EEPROM
     */
    void clear();

    /**
     * Delete unused elements
     */
    void shrink();

    /**
     * Prints the content of the EEPROM to serial, only if debug flag is set in Trace.h
     */
    void print() const;

    /**
     * Resets the position where the next element will be inserted. Debugging functionality ...
     */
    void resetInsertPos()
    {
        mAddedEntries = 0;
    }

    /**
     * Gets the amount of entries added since last reboot
     * @return
     */
    key_t getAddedEntryAmount() const {
        return mAddedEntries;
    }

    /**
     * Gets amount of entries in the EEPROM
     * @return amount of entries
     */
    pos_t getEntryAmount() const;

    /**
     * Calculates a position specified by the number of the entry
     * @param index Index of the entry
     * @returns position of the entry in the EEPROM
     */
    pos_t calcPosByIndex(pos_t index) const;

protected:


    /**
     * Sets amount of entries
     * @param amount new amount of entries
     */
    void setEntryAmount(pos_t amount);

private:
    /**
     * Appends a value to the EEPROM.
     * @param key identifier of the value
     * @param value initial value
     */
    bool append(key_t key, value_t value);


    /**
     * Inserts an entry in the list by moving current entry at the pos to the end of the list and
     * sets the current key/value at the position. Handles EERPOM read/write errors by trying next
     * element since it could be written
     * is appended. If from and to is equal nothing is done
     * @param pos position of insert the entry
     * @param key key to insert
     * @parem value value to insert
     * @param moveCurrent true, if current element should be moved to the end of the list
     */
    void insertEntryByPos(pos_t pos, key_t key, value_t value, bool moveCurrent);

    /**
     * Moves an entry to a new location, if we encounter a write error at the new location the value
     * is appended. If from and to is equal nothing is done
     * @param from current position of the entry
     * @param to new position of the entry
     * @return true, if the value has been moved successfully. False, if the value could not be set
     * this is usually due to a hardware defect in the eeprom cell
     */
    bool moveEntryByPos(pos_t from, pos_t to);

    /**
     * Finds an id in the EEPROM
     * @param key key to find
     * @return position of the entry
     */
    pos_t findPos(key_t key) const;

    /**
     * Gets a value indentified by the position in the eeprom
     * @param pos position of the value
     * @return value found at pos
     */
    value_t getValueByPos(pos_t pos) const;

    /**
     * Sets a value at a position
     * @param pos position of the value (NOT the key = beginning of entry)
     * @param value value to set
     * @return true, if the value has been set successfully. False, if the value could not be set
     * this is usually due to a hardware defect in the eeprom cell
     */
    bool setValueByPos(pos_t pos, value_t value);

    /**
     * Sets a entry at a position
     * @param pos position of the key/value
     * @param key key to set
     * @param value value to set
     * @return true, if the value has been set successfully. False, if the value could not be set
     * this is usually due to a hardware defect in the eeprom cell
     */
    bool setEntryByPos(pos_t pos, key_t key, value_t value);

    /**
     * Writes data to the EEPROM
     * @param pos position in the EEPROM
     * @param element element to write
     * @return true, if data has been written sucessfully
     */
    bool writeData(pos_t pos, eeprom_t element);

    /**
     * Reads an entry from the eeprom
     * @param pos position of the entry
     * @return entry of the position
     */
    eeprom_t readData(pos_t pos) const;

    /**
     * Gets the key of an entry
     */
    key_t getKeyByPos(pos_t pos) const;

    /**
     * Sets a key at a position
     * @param pos position of the key
     * @param key key to set
     * @return true, if the value has been set successfully. False, if the value could not be set
     * this is usually due to a hardware defect in the eeprom cell
     */
    bool setKeyByPos(pos_t pos, key_t key);

    static pos_t mFreePos;
    pos_t        mStartPos;
    pos_t        mMaxEntries;
    key_t        mAddedEntries;

    static const pos_t NOT_FOUND   = -1;
    static const key_t INVALID_CELL = 0;
    static const pos_t HEADER_SIZE = 1;        // Size of the header containing the amount of elements in the EEPROM
    static const pos_t ID_SIZE     = sizeof(key_t);
    static const pos_t ENTRY_SIZE  = ID_SIZE + sizeof(value_t);         // Size of one entry
    static const pos_t EEPROM_SIZE = 1024;


};

#endif // __EEPROMMANAGER_H
