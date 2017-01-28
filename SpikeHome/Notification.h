/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Notification.h
 * Purpose:   Stores all data for a notification that might be send to other arduinos or to a pc
 *
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 24. Dezember 2016, 06:58
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __NOTIFICATION_H
#define	__NOTIFICATION_H

#include "StdInclude.h"

class SerialReader;

class Notification
{
public:
    typedef uint8_t base_t;
    typedef uint8_t error_t;

    static const error_t NO_ERROR = 0;
    static const error_t NO_DATA  = 1;
    static const error_t INVALID_LENGTH_ERROR = 2;
    static const error_t PARITY_ERROR = 3;
    static const uint8_t BUFFER_SIZE       = sizeof(base_t) * 3 + sizeof(key_t) + sizeof(value_t) + sizeof(base_t);

    typedef base_t  buffer_t[BUFFER_SIZE];

    /**
     * Empty notification to read from serial
     */
    Notification();

    /**
     * Creates a notification
     * @param key key of the notification
     * @param value value of the notification
     */
    Notification(key_t key, StateValue value);

    /**
     * Creates a notification
     * @param key key of the notification
     * @param value value of the notification
     * @param senderAddress address of the sender of the notification
     * @param receiverAddress address of the receiver of the notification
     */
    Notification(key_t key, StateValue value, base_t senderAddress, base_t receiverAddress);


    /**
     * Creates a notification
     * @param buffer buffer received from a transmission containing the notification data in a base_t stream
     * @param bytesReceived amount of bytes in the receive buffer
     */
    Notification(buffer_t buffer, base_t bytesReceived);

    /**
     * Checks if the notification is empty or unset
     * @return true, if the notification is empty or not set
     */
    bool isEmpty() const
    {
        return mKey == 0;
    }

    /**
     * True, if the acknowledge flag is set
     * @return true, if the sender requests an acknowlege
     */
    bool isAcknowledge() const
    {
        return mAcknowledge;
    }

    /**
     * Returns an error code from the notification receive check.
     * @return error code
     */
    error_t getError() const
    {
        return mError;
    }

    /**
     * Checks for error
     * @return true, if the notification received has an error
     */
    bool hasError() const
    {
        return mError != NO_ERROR;
    }

    /**
     * Writes the data to a serial device in binary form
     * @param serial serial device
     */
    void writeToSerial(HardwareSerial* serial) const;

    /**
     * Prints the data to a serial device in more readable form
     * @param serial serial device
     */
    void printToSerial(HardwareSerial* serial) const;

    /**
     * Prints the data to a serial device in Json format
     * @param serial serial device
     */
    void printJsonToSerial(HardwareSerial* serial) const;

    /**
     * Gets a notification content from serial (in json format)
     * @param serial serial device to read from
     * @param serialSpeed speed setting of serial interface to calculate timeouts
     * @return true, if value has beed read
     */
    bool getJsonFromSerial(HardwareSerial* serial, time_t serialSpeed);

    /**
     * Gets the sender address of the notification
     * @return sender address
     */
    base_t getSenderAddress() const
    {
        return mSenderAddress;
    }

    /**
     * Gets the receiver address of the notification
     * @return receiver address
     */
    base_t getReceiverAddress() const
    {
        return mReceiverAddress;
    }

    /**
     * Gets the key of the notification
     * @return key of th notification
     */
    key_t getKey() const
    {
        return mKey;
    }

    /**
     * Gets the amount of bytes received
     * @return amount of bytes received
     */
    uint8_t getBytesReceived() const
    {
        return mBytesReceived;
    }

    /**
     * Gets the value of the notification as int
     * @return integer value of the notification
     */
    value_t getValueInt() const
    {
        return mValue.toInt();
    }

private:

    /**
     * Calculates a parity value from the notification. The parity value is used to find errors.
     * @return parity value
     */
    base_t calcParity() const;

    /**
     * Sets a value from a serial reader reading string type streams
     * @param reader class to read from serial
     * @return true, if value has been found
     */
    bool setValueFromSerialReader(SerialReader& reader);

    base_t mSenderAddress;
    base_t mReceiverAddress;
    base_t mAcknowledge;
    key_t  mKey;
    StateValue mValue;
    uint8_t mError;
    uint8_t mBytesReceived;
};


#endif	/* __NOTIFICATION_H */
