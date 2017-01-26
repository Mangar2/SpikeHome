/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Schedule.h
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */
#include "Schedule.h"
#include "Device.h"

time_t              Schedule::mLoops;
NotifyTargetList    Schedule::mTargetList;
NotifyTarget*       Schedule::mNotifyIterator;
time_t              Schedule::mNotifyTimer;
uint16_t            Schedule::mNotifyLoopCount;
value_t             Schedule::mConfigInfoPeriod;

void Schedule::init()
{
    mLoops = 0;
    mNotifyLoopCount = 0;
    mNotifyTimer = 0;
    mNotifyIterator = 0;
    mConfigInfoPeriod = Device::addConfigValue(0, NotifyTarget::CONFIG_INFO_PERIOD_KEY, 2);
}

void softwareReset()
{
    asm volatile(" jmp 0");
}

void Schedule::nextTick()
{
#ifdef DEBUG
    checkSettings();
#endif
    time_t nextDelay;
    time_t curTimeInMilliseconds = millis();
    time_t targetTimeInMilliseconds = mLoops * NotifyTarget::MILLISECONDS_PER_LOOP;
    mLoops++;
    // Respects millis overflow a-b works well for unsigned variables even when a overflows.
    nextDelay = timeDiff(targetTimeInMilliseconds, curTimeInMilliseconds);
    Device::getIOHandler()->pollNonBlocking();
    notify();
    checkState();
#ifdef DEBUG
    static time_t oldLag = 0;
    if (nextDelay == 0) {
        time_t lag = curTimeInMilliseconds - targetTimeInMilliseconds + 10;
        if (oldLag < lag) {
            oldLag = lag;
            printIfDebug(F("lag: ")); printlnIfDebug(curTimeInMilliseconds - targetTimeInMilliseconds + 10);
        }
    } else {
        oldLag = 0;
    }
#endif
    delay(nextDelay);
}

void Schedule::addTarget(NotifyTarget* pTarget)
{
    mTargetList.add(pTarget);
}

void Schedule::notifyConfigChange(device_t deviceNo, key_t key, value_t value)
{
    if (key == NotifyTarget::CONFIG_INFO_PERIOD_KEY) {
        if  (value > 0 && deviceNo == 0) {
            mConfigInfoPeriod = value;
            Device::setConfigValue(0, key, value);
        }
    } else {
        mTargetList.notifyConfigChange(deviceNo, key, value);
    }
}

void Schedule::broadcastChange(key_t key, value_t value)
{
    if (key == NotifyTarget::CONFIG_INFO_PERIOD_KEY) {
        if  (value > 0) {
            mConfigInfoPeriod = value;
            Device::setConfigValue(0, key, value);
        }
    } else {
        for (device_t deviceNo = 0; deviceNo < Device::getDeviceAmount(); deviceNo++) {
            mTargetList.notifyConfigChange(deviceNo, key, value);
        }
    }
}

void Schedule::notify()
{
    bool enoughTimeElapsed = millis() - mNotifyTimer > mConfigInfoPeriod * NotifyTarget::MILLISECONDS_IN_A_SECOND;

    if (Device::getIOHandler()->maySend() && enoughTimeElapsed && mTargetList.getFirstNotifyTarget() != 0)  {
        mNotifyTimer = millis();
        if (mNotifyIterator == 0) {
            mNotifyIterator = mTargetList.getFirstNotifyTarget();
        }
        if (mNotifyIterator->notifyServer(mNotifyLoopCount)) {
            mNotifyLoopCount = 0;
            mNotifyIterator = mNotifyIterator->getNext();
        } else {
            mNotifyLoopCount ++;
        }
    }
}

void Schedule::checkState()
{
    mTargetList.callCheckState(mLoops);
}

void Schedule::checkSettings()
{
#ifdef DEBUG
    static bool firstCall = true;
    if (!firstCall) {
        return;
    }
    firstCall = false;

    printlnIfDebug(F("Checking settings...."));
    printIfDebug(F("Memory left (space between heap and stack) in Bytes: ")); printlnIfDebug(Trace::getFreeMemory());
    printIfDebug(F("Amount of Devices: ")); printlnIfDebug(Device::getDeviceAmount());
    printlnIfDebug(F("Starting 1000 checkState loop to test timings..."));
    time_t start = millis();
    for (mLoops = 0; mLoops < 1000; mLoops++) {
        checkState();
    }
    mLoops = 0;
    printIfDebug(F("Time used per loop (must be << 10) : ")); printlnIfDebug((millis() - start) / 1000.0);
    printIfDebug(F(""));
    for (device_t deviceNo = 0; deviceNo < Device::getDeviceAmount(); deviceNo++) {
        printIfDebug(F("Listeners per device: ")); printIfDebug(Device::getNotify(deviceNo).getListenerAmount());
        printIfDebug(F(" Should be less than: ")); printlnIfDebug(MAX_NOTIFY_TARGETS_PER_DEVICE);
    }
#endif
}
