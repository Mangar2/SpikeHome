/*
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485.h
 * Purpose:   Implements a communication via. RS485 bus device. It uses a token-ring implementation
 *            to decide who may send.
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __RS485_H
#define __RS485_H

#include "RS485State.h"
#include "NotificationV2.h"
#include "SerialIO.h"

class Notification;

class RS485  : public SerialIO {

public:

    /**
     * Creates a new RS485 IO class
     * @param int readWritePin number of the pin to set read/write state to the RS486 Hardware
     */
    RS485(device_t deviceAmount, pin_t readWritePin);


    /**
     * Checks for an info in the serial input and handles it
     */
    virtual void pollNonBlocking();

    /**
     * Checks if sending is currently allowed because we have the token
     * @return true, if sending is allowed
     */
    virtual bool maySend()
    {
        return mState.maySend();
    }


private:

    /**
     * sends a notification to the RS485 bus
     * @param notification notification to write
     */
    virtual void sendNotification(const NotificationV2& notification);

    /**
     * Checks if an error occured while receiving a package and send an
     * error message to the server
    */
    void sendReceiveError();

    /**
     * Handles a notification received from RS485
     * Notification notification read from serial
     */
    void handleNotification(const NotificationV2& notification);

    /**
     * Handles a notification received of type "state"
     * @param notification notification read from serial
     */
    void handleStateNotification(const NotificationV2& notification);

    /**
     * Handles a notification received of type "command"
     * @param notification notification read from serial
     */
    void handleCommandNotification(const NotificationV2& notification);

    /**
     * Performs token handling if nothing has been send
     * @param stateType type of state change. See constant defintions in RS485State.h
     * @param version of the incoming message
     */
    void handleNewTokenState(value_t stateType, uint8_t messageVersion);


    static const int8_t    RS485_TRANSMIT            = HIGH;
    static const int8_t    RS485_RECEIVE             = LOW;
    static const time_t    BITS_PER_CHAR             = 9L;
    static const time_t    MILLISECONDS_IN_A_SECOND  = 1000L;


    pin_t      mReadWritePin;
    RS485State mState;

    bool       mStateChanged;
    value_t    mReceiveError;
    uint8_t    mReceiveBuf[NotificationV2::BUFFER_SIZE];
};

#endif // __RS485_H
