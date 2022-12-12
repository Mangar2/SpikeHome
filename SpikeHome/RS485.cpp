/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

//#define DEBUG
#include "RS485.h"

RS485::RS485(device_t deviceAmount, pin_t readWritePin)
 :SerialIO(deviceAmount)
{
    mReadWritePin = readWritePin;

    // Receive mode is standard.
    pinMode(mReadWritePin, OUTPUT);
    digitalWrite(mReadWritePin, RS485_RECEIVE);  // Enable Receive

    mMessageVersion = NotificationV2::MAX_SUPPORTED_MESSAGE_VERSION;

}

void RS485::sendNotification(const NotificationV2& notification)
{


    int32_t bitsPerNotification = BITS_PER_CHAR * int32_t(NotificationV2::BUFFER_SIZE);
#ifdef DEBUG
    uint8_t version = notification.getVersion();
    printVariableIfDebug(version);
#endif
    digitalWrite(mReadWritePin, RS485_TRANSMIT);
    // Short delay to ensure write state is set on RS485
    delay(3);
    notification.writeToSerial(mpSerial);

    // Wait until Hardware Serial has sent all data before disabling send state of RS485
    delay(bitsPerNotification * MILLISECONDS_IN_A_SECOND / mSerialSpeedInBitsPerSecond);
    // Savety buffer
    delay(2);
    digitalWrite(mReadWritePin, RS485_RECEIVE);
    if (notification.getSenderAddress() == 1 && notification.getKey() != RS485State::TOKEN) {
        notification.printToSerial(&Serial);
    }
}

void RS485::pollNonBlocking()
{
    time_t timeLastCharRead = 0;
    time_t timeoutInMilliseconds = 3 + BITS_PER_CHAR * MILLISECONDS_IN_A_SECOND / mSerialSpeedInBitsPerSecond;
    uint8_t i = 0;
    do {
        if (mpSerial->available())
        {
            timeLastCharRead = millis();
            mReceiveBuf[i] = mpSerial->read();
            if (i > 0 || (mReceiveBuf[0] != 0 && mReceiveBuf[0] <= 0x7F)) {
                i++;
            }
        }
    } while ((timeLastCharRead + timeoutInMilliseconds > millis()) && (i < NotificationV2::BUFFER_SIZE));

#ifdef DEBUG
    if (i > 0) {

        for (int j = 0; j < i; j++) {
            Trace::printHex(mReceiveBuf[j]);
        }

        printlnIfDebug("");
    }
#endif

    handleNotification(NotificationV2(mReceiveBuf, i));

}

void RS485::handleStateNotification(const NotificationV2& notification)
{
    value_t value = notification.getValueInt();

    bool notForMe = notification.getReceiverAddress() != mSenderAddress[0];
    mState.storeSenderAddress( notification.getSenderAddress(), mSenderAddress[0]);
    sendReceiveError();
    if (value == RS485State::PASS_SEND_TOKEN_TO_NEXT_DEVICE) {
        mMessageVersion = notification.getVersion();    
    }
    value_t newState = mState.changeState(value, notForMe);
    handleNewTokenState(newState, notification.getVersion());
}

void RS485::handleCommandNotification(const NotificationV2& notification)
{
#ifdef DEBUG
    uint8_t state = mState.getState();
#endif    
    sendReceiveError();
    printVariableIfDebug(state);
    if (!mState.ignoreCommands()) {
        if (notification.isAcknowledge()) {
            reply(notification);
        }
        notify(notification);
    }
}

void RS485::handleNewTokenState(value_t stateType, uint8_t messageVersion)
{
    if (stateType != 0) {

        if (stateType == RS485State::STATE_CHANGED) {
            mStateChanged = true;
        }

        if (stateType == RS485State::REGISTRATION_INFO || (mStateChanged && maySend() )) {
            sendToServer(0, NotifyTarget::STATE_NOTIFICATION, value_t(mState.getState()) * 0x100 + mState.getReceiverAddress());
            sendToServer(0, NotifyTarget::MEM_LEFT_NOTIFICATION, Trace::getFreeMemory());
            mStateChanged = false;
        }
        if (stateType == RS485State::PASS_SEND_TOKEN_TO_NEXT_DEVICE) {
            sendToAddress(0, RS485State::TOKEN, RS485State::PASS_SEND_TOKEN_TO_NEXT_DEVICE, mState.getReceiverAddress());
        } else if (stateType != RS485State::STATE_CHANGED) {
            broadcast(0, RS485State::TOKEN, stateType, messageVersion);
        }
    }
}

void RS485::handleNotification(const NotificationV2& notification)
{

    switch(notification.getError()) {
        case NotificationV2::INVALID_LENGTH_ERROR:
            mReceiveError = 0x0100 + notification.getBytesReceived();
            printIfDebug("Invalid Length: ");
            printlnIfDebug(notification.getBytesReceived());
            break;
        case NotificationV2::CHECK_ERROR:
            mReceiveError = 0x0200 + notification.getKey();
            break;
        case NotificationV2::NO_DATA:
            handleNewTokenState(mState.changeStateNoInfo(), mMessageVersion);
            break;
        case NotificationV2::NO_ERROR:
#ifdef DEBUG
            notification.printToSerial(&Serial);
#endif
            mReceiveError = 0;
            if (notification.getKey() == RS485State::TOKEN) {
                handleStateNotification(notification);
            } else {
                handleCommandNotification(notification);
            }
            break;
        default:
            printlnIfDebug("Unknown error type in Notification");
            break;
    }
}

void RS485::sendReceiveError()
{
    if (mReceiveError != 0) {
        sendToServer(0, NotifyTarget::RECEIVE_ERROR_NOTIFICATION, mReceiveError);
        mReceiveError = 0;
    }
}
