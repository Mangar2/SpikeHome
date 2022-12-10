/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      SerialIO.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 25. Dezember 2016, 08:15
 * ---------------------------------------------------------------------------------------------------
 */

#include "SerialIO.h"
#include "Device.h"
#include "Schedule.h"

SerialIO::SerialIO(device_t deviceAmount)
{
    mDeviceAmount = deviceAmount;
    mReceiverAddress = Device::addConfigValue(0, NotifyTarget::SERVER_ADDRESS_KEY, SERVER_ADDRESS);
    for (device_t deviceNo = 0; deviceNo < deviceAmount; deviceNo++) {
        mSenderAddress[deviceNo] = Device::addConfigValue(deviceNo, NotifyTarget::ADDRESS_KEY, ADDRESS_NOT_SET);
    }
    
}

void SerialIO::initSerial(HardwareSerial* pSerial, time_t serialSpeed)
{
    mpSerial = pSerial;
    mSerialSpeedInBitsPerSecond = serialSpeed;
    printIfDebug(F("Serial Speed = "));
    printlnIfDebug(serialSpeed);
}

void SerialIO::sendToServer(device_t deviceNo, key_t key, value_t value)
{
    NotificationV2 notification(key, value, mSenderAddress[deviceNo], mReceiverAddress);
    notification.setVersion(mMessageVersion);
    sendNotification(notification);
}

void SerialIO::broadcast(device_t deviceNo, key_t key, value_t value)
{
    NotificationV2 notification(key, value, mSenderAddress[deviceNo], BROADCAST_ADDRESS);
    notification.setVersion(mMessageVersion);
    sendNotification(notification);
}

void SerialIO::broadcast(device_t deviceNo, key_t key, value_t value, uint8_t messageVersion)
{
    NotificationV2 notification(key, value, mSenderAddress[deviceNo], BROADCAST_ADDRESS);
    notification.setVersion(messageVersion);
    sendNotification(notification);
}

void SerialIO::sendToAddress(device_t deviceNo, key_t key, value_t value, address_t receiverAddress)
{
    NotificationV2 notification(key, value, mSenderAddress[deviceNo], receiverAddress);
    notification.setVersion(mMessageVersion);
    sendNotification(notification);
}

void SerialIO::reply(const NotificationV2& notification)
{
    address_t receiverAddress = notification.getReceiverAddress();
    device_t deviceNo = getDeviceNoFromAddress(receiverAddress);
    const bool IsForMe = deviceNo != -1 && deviceNo < MAX_DEVICE_AMOUNT;
    const bool isBroadcast = notification.getReceiverAddress() == BROADCAST_ADDRESS;

    if (IsForMe && !isBroadcast) {
        NotificationV2 reply(notification.getKey(), notification.getValueInt(),
                receiverAddress, notification.getSenderAddress());
        reply.setVersion(notification.getVersion());
        sendNotification(reply);
    }
}

void SerialIO::notify(const NotificationV2& notification)
{
    key_t key = notification.getKey();
    value_t value = notification.getValueInt();
    address_t receiverAddress = notification.getReceiverAddress();
    address_t senderAddress = notification.getSenderAddress();
    device_t deviceNo = getDeviceNoFromAddress(receiverAddress);

    const bool IsForMe = deviceNo != -1 && deviceNo < MAX_DEVICE_AMOUNT;

    if (IsForMe) {

        if (key == NotifyTarget::SERVER_ADDRESS_KEY && senderAddress == SerialIO::SERVER_ADDRESS) {
            if (value != BROADCAST_ADDRESS && value < ADDRESS_NOT_SET) {
                Device::setConfigValue(0, key, value);
                mReceiverAddress = value;
            }
        } else if (key == NotifyTarget::ADDRESS_KEY && senderAddress == SerialIO::SERVER_ADDRESS && receiverAddress != BROADCAST_ADDRESS) {
            if (value > SERVER_ADDRESS && value < ADDRESS_NOT_SET) {
                Device::setConfigValue(deviceNo, key, value);
                mSenderAddress[deviceNo] = value;
            }
        } else {
            if (receiverAddress == BROADCAST_ADDRESS) {
                Schedule::broadcastChange(senderAddress, key, value);
            } else {
                Schedule::notifyChange(deviceNo, senderAddress, key, value);
            }
        }
    }
}

device_t SerialIO::getDeviceNoFromAddress(address_t address)
{
    device_t result = -1;
    if (address == BROADCAST_ADDRESS) {
        result = 0;
    } else {
        for (device_t deviceNo = 0; deviceNo < mDeviceAmount; deviceNo++) {
            if (mSenderAddress[deviceNo] == address) {
                result = deviceNo;
                break;
            }
        }
    }
    return result;
}
