/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      NotificationV2.h
 * Purpose:   Stores all data for a notification that might be send to other arduinos or to a pc
 * Version 2: Added support for CRC16 and 4byte data
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 24. Dezember 2016, 06:58
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __NOTIFICATIONV2_H
#define	__NOTIFICATIONV2_H

#include "StdInclude.h"

class SerialReader;

class NotificationV2
{
public:
    typedef uint8_t base_t;
    typedef uint8_t error_t;
    typedef uint16_t check_t;

    static const uint8_t MAX_SUPPORTED_MESSAGE_VERSION  = 1;

    static const error_t NO_ERROR = 0;
    static const error_t NO_DATA  = 1;
    static const error_t INVALID_LENGTH_ERROR = 2;
    static const error_t CHECK_ERROR = 3;
    static const error_t ILLEGAL_VERSION = 4;
    static const base_t VERSION_SHIFT = 1;
    static const base_t VERSION = 1; 
    static const base_t BUFFER_SIZE_V0 = 7;
    static const uint8_t BUFFER_SIZE       = 
        sizeof(base_t) * 4 +    // From, To, Acknowledge, Length
        sizeof(key_t) +  
        sizeof(value_t) + 
        sizeof(check_t);        // CRC

    typedef base_t  buffer_t[BUFFER_SIZE];

    /**
     * Empty notification to read from serial
     */
    NotificationV2();

    /**
     * Creates a notification
     * @param key key of the notification
     * @param value value of the notification
     */
    NotificationV2(key_t key, StateValue value);

    /**
     * Creates a notification
     * @param key key of the notification
     * @param value value of the notification
     * @param senderAddress address of the sender of the notification
     * @param receiverAddress address of the receiver of the notification
     */
    NotificationV2(key_t key, StateValue value, base_t senderAddress, base_t receiverAddress);


    /**
     * Creates a notification
     * @param buffer buffer received from a transmission containing the notification data in a base_t stream
     * @param bytesReceived amount of bytes in the receive buffer
     */
    NotificationV2(buffer_t buffer, base_t bytesReceived);

    /**
     * Sets the message version
     * @param version new message version (currently supported: 0, 1)
     */
    void setVersion(base_t version) 
    {
        if (version <= MAX_SUPPORTED_MESSAGE_VERSION) {
            mVersion = version;
        }
    }

    /**
     * Gets the current message version
     * @returns message version
     */
    base_t getVersion() const
    {
        return mVersion;
    }

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
    check_t calcCRC16() const;

    /**
     * Sets a value from a serial reader reading string type streams
     * @param reader class to read from serial
     * @return true, if value has been found
     */
    bool setValueFromSerialReader(SerialReader& reader);

    /**
     * Sets data of a version 0 message
     * @param buffer buffer received from a transmission containing the notification data in a base_t stream
     * @param bytesReceived amount of bytes in the receive buffer
     */ 
    void setVersion0(buffer_t buffer, base_t bytesReceived);

    /**
     * Sets data of a version 1 message
     * @param buffer buffer received from a transmission containing the notification data in a base_t stream
     * @param bytesReceived amount of bytes in the receive buffer
     */ 
    void setVersion1(buffer_t buffer, base_t bytesReceived);

    /**
     * Writes part of the data of a Version 1 message (helper for writeToSerial)
     * @param serial serial device
     */
    void writeV1ToSerial(HardwareSerial* serial) const;

    /**
     * Writes part of the data of a Version 0 message (helper for writeToSerial)
     * @param serial serial device
     */
    void writeV0ToSerial(HardwareSerial* serial) const;

    /**
     * Calculates a parity value
     * @returns paraty byte
     */
    base_t NotificationV2::calcParity() const;

    base_t mSenderAddress;
    base_t mReceiverAddress;
    base_t mAcknowledge;
    base_t mVersion;
    base_t mSize;
    key_t  mKey;
    StateValue mValue;
    uint8_t mError;
    uint8_t mBytesReceived;
};


#endif	/* __NOTIFICATION_H */
