/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      NotifyTarget.cpp
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "NotifyTarget.h"
#include "Device.h"
#include "Schedule.h"

value_t NotifyTarget::getConfigValue(key_t key)
{
    return Device::getConfigValue(getDeviceNo(), key);
}

void NotifyTarget::setConfigValue(key_t key, value_t value)
{
    return Device::setConfigValue(getDeviceNo(), key, value);
}

value_t NotifyTarget::addConfigValue(key_t key, value_t value)
{
    return Device::addConfigValue(getDeviceNo(), key, value);
}

bool NotifyTarget::sendToServer(key_t key, StateValue value)
{
    bool res = false;
    if (Device::getIOHandler()->maySend()) {
        Device::getIOHandler()->sendToServer(getDeviceNo(), key, value.toInt());
        res = true;
    }
    return res;
}

bool NotifyTarget::sendToAddress(key_t key, StateValue value, address_t receiverAddress)
{
    bool res = false;
    if (Device::getIOHandler()->maySend()) {
        Device::getIOHandler()->sendToAddress(getDeviceNo(), key, value.toInt(), receiverAddress);
        res = true;
    }
    return res;
}

bool NotifyTarget::broadcast(key_t key, StateValue value)
{
    bool res = false;
    if (Device::getIOHandler()->maySend()) {
        Device::getIOHandler()->broadcast(getDeviceNo(), key, value.toInt());
        res = true;
    }
    return res;
}


void NotifyTarget::notify(key_t key, StateValue value)
{
    Device::getNotify(mDeviceNo).change(key, value);
}

void NotifyTarget::notifyAllDevices(key_t key, StateValue value)
{
    Schedule::broadcastChange(0, key, value.toInt());
}

