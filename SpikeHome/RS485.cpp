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

#include "RS485.h"

RS485::RS485(device_t deviceAmount, pin_t readWritePin)
 :SerialIO(deviceAmount)
{
    mReadWritePin = readWritePin;

    // Receive mode is standard.
    pinMode(mReadWritePin, OUTPUT);
    digitalWrite(mReadWritePin, RS485_RECEIVE);  // Enable Receive
}

void RS485::sendNotification(const Notification& notification)
{
    int32_t bitsPerNotification = BITS_PER_CHAR * int32_t(Notification::BUFFER_SIZE);

    digitalWrite(mReadWritePin, RS485_TRANSMIT);
    // Short delay to ensure write state is set on RS485
    delay(3);
    notification.writeToSerial(mpSerial);

    // Wait until Hardware Serial has sent all data before disabling send state of RS485
    delay(bitsPerNotification * MILLISECONDS_IN_A_SECOND / mSerialSpeedInBitsPerSecond);
    // Savety buffer
    delay(2);
    digitalWrite(mReadWritePin, RS485_RECEIVE);
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
    } while ((timeLastCharRead + timeoutInMilliseconds > millis()) && (i < Notification::BUFFER_SIZE));

    handleNotification(Notification(mReceiveBuf, i));

}

void RS485::handleStateNotification(const Notification& notification)
{
    value_t value = notification.getValueInt();

    bool notForMe = notification.getReceiverAddress() != mSenderAddress[0];
    mState.storeSenderAddress( notification.getSenderAddress(), mSenderAddress[0]);
    sendReceiveError();
    handleNewTokenState(mState.changeState(value, notForMe));
}

void RS485::handleCommandNotification(const Notification& notification)
{
    if (notification.isAcknowledge()) {
        reply(notification);
    }
    notify(notification);
}

void RS485::handleNewTokenState(value_t stateType)
{
    if (stateType != 0) {

        if (stateType == RS485State::STATE_CHANGED) {
            mStateChanged = true;
        }

        if (stateType == RS485State::REGISTRATION_INFO || (mStateChanged && maySend())) {
            sendToServer(0, NotifyTarget::STATE_NOTIFICATION, value_t(mState.getState()) * 100 + mState.getReceiverAddress());
            sendToServer(0, NotifyTarget::MEM_LEFT_NOTIFICATION, Trace::getFreeMemory());
            mStateChanged = false;
        }
        if (stateType == RS485State::ENABLE_SEND) {
            sendNotification(Notification(
                RS485State::TOKEN,
                RS485State::ENABLE_SEND,
                mSenderAddress[0],
                mState.getReceiverAddress()));
        } else if (stateType != RS485State::STATE_CHANGED) {
            broadcast(0, RS485State::TOKEN, stateType);
        }
    }
}

void RS485::handleNotification(const Notification& notification)
{

    switch(notification.getError()) {
        case Notification::INVALID_LENGTH_ERROR:
            mReceiveError = 0x0100 + notification.getBytesReceived();
            break;
        case Notification::PARITY_ERROR:
            mReceiveError = 0x0200 + notification.getKey();
            break;
        case Notification::NO_DATA:
            handleNewTokenState(mState.changeStateNoInfo());
            break;
        case Notification::NO_ERROR:
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
