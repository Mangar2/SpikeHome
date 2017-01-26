/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Device.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#include "Device.h"

EEPROMManager Device::mEEPROM(2);
Config Device::mConfig[MAX_DEVICE_AMOUNT];
Notify Device::mNotify[MAX_DEVICE_AMOUNT];
device_t Device::mDeviceAmount;
SerialIO* Device::mpSerial;

void Device::init(value_t softwareVersion, device_t deviceAmount)
{
    device_t deviceNo;
    //clearConfig = true;
    deviceAmount = min(deviceAmount, MAX_DEVICE_AMOUNT);
    deviceAmount = max(deviceAmount, 1);
    mDeviceAmount = deviceAmount;
    for (deviceNo = 0; deviceNo < deviceAmount; deviceNo++) {
        mConfig[deviceNo].setDeviceNo(deviceNo);
    }
    addConfigValue(0, NotifyTarget::SOFTWARE_VERSION_KEY, softwareVersion);
    setConfigValue(0, NotifyTarget::SOFTWARE_VERSION_KEY, softwareVersion);
    mpSerial = 0;
}

value_t Device::readSensorType()
{
    if (mEEPROM.getEntryAmount() != 2 || mEEPROM.getValue('1') != 0x7531) {
        mEEPROM.clear();
        mEEPROM.addValue('1', 0x7531);
        mEEPROM.addValue('2', 0);
        mEEPROM.print();
    };
    return mEEPROM.getValue('2');
}

void Device::storeSensorType(value_t sensorType)
{
    value_t curType = readSensorType();

    if (curType != sensorType) {
        mEEPROM.setValue('2', sensorType);
    }
}

Config& Device::getConfig(device_t index)
{
    if (index >= 0 && index < mDeviceAmount) {
        return mConfig[index];
    } else {
        return mConfig[0];
    }
}

Notify& Device::getNotify(device_t index)
{
    if (index >= 0 && index < mDeviceAmount) {
        return mNotify[index];
    } else {
        return mNotify[0];
    }
}

device_t Device::addressToIndex(value_t address)
{
    device_t index;
    device_t result = -1;

    for (index = 0; index < mDeviceAmount; index++) {
        if (address == mConfig[index].getValue(NotifyTarget::ADDRESS_KEY)) {
            result = index;
            break;
        }
    }

    return result;
}
