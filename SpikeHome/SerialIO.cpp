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
    mReceiverAddress = Device::addConfigValue(0, NotifyTarget::SERVER_ADDRESS_KEY, 1);
    for (device_t deviceNo = 0; deviceNo < deviceAmount; deviceNo++) {
        mSenderAddress[deviceNo] = Device::addConfigValue(deviceNo, NotifyTarget::ADDRESS_KEY, 127);
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
    Notification notification(key, value, mSenderAddress[deviceNo], mReceiverAddress);
    sendNotification(notification);
}

void SerialIO::broadcast(device_t deviceNo, key_t key, value_t value)
{
    Notification notification(key, value, mSenderAddress[deviceNo], BROADCAST_ADDRESS);
    sendNotification(notification);
}

void SerialIO::sendToAddress(device_t deviceNo, key_t key, value_t value, address_t receiverAddress)
{
    Notification notification(key, value, mSenderAddress[deviceNo], receiverAddress);
    sendNotification(notification);
}

void SerialIO::reply(const Notification& notification)
{
    Notification reply(notification.getKey(), notification.getValueInt(),
            notification.getReceiverAddress(), notification.getSenderAddress());
    sendNotification(reply);
}

void SerialIO::notify(const Notification& notification)
{
    key_t key = notification.getKey();
    value_t value = notification.getValueInt();
    address_t address = notification.getReceiverAddress();
    device_t deviceNo = getDeviceNoFromAddress(address);

    if (deviceNo != -1 && deviceNo < MAX_DEVICE_AMOUNT) {

        if (key == NotifyTarget::SERVER_ADDRESS_KEY) {
            if (value > 0 && value < 127) {
                Device::setConfigValue(0, key, value);
                mReceiverAddress = value;
            }
        } else if (key == NotifyTarget::ADDRESS_KEY) {
            if (value > 1 && value < 127) {
                Device::setConfigValue(deviceNo, key, value);
                mSenderAddress[deviceNo] = value;
            }
        } else {
            if (address == 0) {
                Schedule::broadcastChange(key, value);
            } else {
                Schedule::notifyConfigChange(deviceNo, key, value);
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
