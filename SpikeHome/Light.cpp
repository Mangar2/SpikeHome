/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:        Light.cpp
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */
//#define DEBUG
#include "Light.h"

Light::Light(device_t deviceNo, pin_t brightnessPin, pin_t lightOutputPin)
: BrightnessSensor(deviceNo, brightnessPin)
{

    mLightVoltage = 0;
    mOldLightVoltage = 0;
    mLightOutputPin = lightOutputPin;
    // if system temperature is not measured the system reacts like in warning mode (lights are always fully on)
    mHeatAlarm = HEAT_ALARM_OFF;
    pinMode(mLightOutputPin, OUTPUT);

    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_ALLWAYS);
    initConfig();
}

void Light::initConfig()
{
    mMaximumBrightness = addConfigValue(MAXIMUM_BRIGHTNESS_KEY, 90);
    mTargetBrightness = addConfigValue(TARGET_BRIGHTNESS_KEY, 100);
    mStartVoltage = addConfigValue(START_VOLTAGE_KEY, 100);
    mFullOnVoltage = addConfigValue(FULL_ON_VOLTAGE_KEY, MAX_VOLTAGE);
    mDimmingDelay = addConfigValue(DIMMING_DELAY_KEY, 20) / MILLISECONDS_PER_LOOP;
}

void Light::setTargetBrightness(value_t brightnessInPercent)
{
    if (brightnessInPercent >= 0 && brightnessInPercent <= 300) {
        mState.targetBrightnessChanged(mTargetBrightness, brightnessInPercent);
        mTargetBrightness = brightnessInPercent;
        setConfigValue(TARGET_BRIGHTNESS_KEY, mTargetBrightness);
        mMaxLightVoltage = 0;
        mOldLightVoltage = 0;

        dimLight();
    }
}

void Light::setStartVoltage(value_t startVoltage)
{
    if (startVoltage < MAX_VOLTAGE) {
        mStartVoltage = startVoltage;
        setConfigValue(START_VOLTAGE_KEY, mStartVoltage);
    }
}

void Light::setFullOnVoltage(value_t fullOnVoltage)
{
    if (fullOnVoltage < MAX_VOLTAGE) {
        mFullOnVoltage = fullOnVoltage;
        setConfigValue(FULL_ON_VOLTAGE_KEY, mFullOnVoltage);
    }
}

void Light::setDimmingDelay(value_t dimmingDelayInMilliseconds)
{
    if (dimmingDelayInMilliseconds >= 10 && dimmingDelayInMilliseconds <= 999) {
        mDimmingDelay = dimmingDelayInMilliseconds / MILLISECONDS_PER_LOOP;
        setConfigValue(DIMMING_DELAY_KEY, dimmingDelayInMilliseconds);
    }

}

void Light::setMaximumBrightness(value_t brightnessInPercent)
{
    if (brightnessInPercent >= 10 && brightnessInPercent <= 200) {
        mMaximumBrightness = brightnessInPercent;
        setConfigValue(MAXIMUM_BRIGHTNESS_KEY, brightnessInPercent);
        mState.checkForDarkness(isDarkEnoughToSwitchOnLight());
    }
}

void Light::handleFS20Command(value_t value)
{
    value_t command = value / 4;
    value_t suffix  = value & 3;
    printVariableIfDebug(command);
    printVariableIfDebug(suffix);
    if (suffix == 1 && command == 0x11) {
        mState.setAdjustLight();
    } else if (command <= 0x10 && command > 0) {
        setTargetBrightness(floor(command * 6.25));
    } else if (command == 0x13) {
        setTargetBrightness(min(120, mTargetBrightness + 6));
    } else if (command == 0x14) {
        setTargetBrightness(mTargetBrightness >= 6 ? mTargetBrightness - 6 : 0);
    }
}

void Light::handleSystemTemperature(StateValue value)
{
    float heat = value.toFloat();
    if (mHeatAlarm == HEAT_ALARM_CRITICAL) {
        heat *= HEAT_HYSTERESE;
    }
    mHeatAlarm = HEAT_ALARM_OFF;
    if (heat > HEAT_CRITICAL_VALUE) {
        mHeatAlarm = HEAT_ALARM_CRITICAL;
    } else if (heat > HEAT_WARNING_VALUE) {
        mHeatAlarm = HEAT_ALARM_WARNING;
    }

}

/**
 * Returns true, if it is dark enough to switch on the lights
 * @param deviceNo number of the device
 * @return true, if brightness is below a configured threshold
 */
bool Light::isDarkEnoughToSwitchOnLight()
{
    return getAbsoluteValue() <= calcAbsoluteTarget(mMaximumBrightness);
}

void Light::handleChange(address_t senderAddress, key_t key, StateValue data)
{
    bool signalOn = (!data.isZero());
    value_t value = data.toInt();
    switch (key) {
        case LIGHT_ON_NOTIFICATION:
            mState.setLight(signalOn, isDarkEnoughToSwitchOnLight());
            break;
        case MOVEMENT_NOTIFICATION:
            mState.checkForDarkness(isDarkEnoughToSwitchOnLight());
            break;
        case SYS_TEMPERATURE_NOTIFICATION: handleSystemTemperature(data);
            break;
        case FS20_COMMMAND: handleFS20Command(value);
            break;
        case ADJUST_LIGHT: mState.setAdjustLight();
            break;
        case MAXIMUM_BRIGHTNESS_KEY: setMaximumBrightness(value);
            break;
        case TARGET_BRIGHTNESS_KEY: setTargetBrightness(value);
            break;
        case START_VOLTAGE_KEY: setStartVoltage(value);
            break;
        case FULL_ON_VOLTAGE_KEY: setFullOnVoltage(value);
            break;
        case DIMMING_DELAY_KEY: setDimmingDelay(value);
            break;
        case SET_LIGHT_TIME:
            if (value != 0) {
                mState.setLight(true, true);
            }
            break;
        default:
            BrightnessSensor::handleChange(0, key, data);
            break;

    }
}

bool Light::dimLight()
{
    int16_t voltageDiff = 0;

    static const int16_t STEP_DIFF = 20;
    static const int16_t HYSTERESE = 5;

    if (mState.isUsingLight()) {
        int16_t curBrightness = int16_t(getAbsoluteValue());
        int16_t targetBrightness = calcAbsoluteTarget(mTargetBrightness);
        if (mHeatAlarm == HEAT_ALARM_WARNING) {
            // Switch the lamp fully on to reduce heat
            targetBrightness = MAX_ANALOG_READ_VALUE;
        }
        if (mHeatAlarm == HEAT_ALARM_CRITICAL) {
            targetBrightness = 0;
        }

        voltageDiff = mState.dimmingStep(curBrightness, targetBrightness);
    } else if (mLightVoltage > 0) {
        voltageDiff = -1;
    }

    bool isDimming = changeLightVoltage(voltageDiff);
    return isDimming;
}

int16_t Light::calcNextVoltage(int16_t curVoltage, bool higher)
{
    int16_t result;
    int16_t lowestBit;
    for (lowestBit = 1; lowestBit < 256; lowestBit *=2 ) {
        if ((curVoltage & lowestBit) != 0) {
            break;
        }
    }
    if (lowestBit == 1) {
        result = curVoltage;
    } else if (higher) {
        result = curVoltage + lowestBit / 2;
    } else {
        result = curVoltage - lowestBit / 2;
    }
    return result;
}

int16_t Light::adjustLight(int16_t curVoltage, int16_t curBrightness)
{
    int16_t result = 0;
    switch (mState.getState()) {
        case LightState::LIGHT_MEASURE_MAX_BRIGHTNESS:
            result = 255;
            if (mState.isBrightnessReliable()) {
                setFullOnBrightness(curBrightness);
                mState.nextAdjustLightState();
                result = 128;
            }
            break;
        case LightState::LIGHT_MEASURE_MIN_VOLTAGE:
            result = curVoltage;
            if (mState.isBrightnessReliable()) {
                result = calcNextVoltage(curVoltage, curBrightness < 3);
                if (result == curVoltage) {
                    setStartVoltage(curVoltage);
                    mState.nextAdjustLightState();
                    result = 128;
                }
            }
            break;
        case LightState::LIGHT_MEASURE_MAX_VOLTAGE:
            result = curVoltage;
            if (mState.isBrightnessReliable()) {
                result = calcNextVoltage(curVoltage, curBrightness < mFullOnBrightness);
                if (result == curVoltage) {
                    setFullOnVoltage(curVoltage);
                    mState.nextAdjustLightState();
                    result = 128;
                }
            }
            break;
    }

    if (result != 0) {
        if (mState.isBrightnessReliable()) {
            Serial.println(mState.getState());
            Serial.println(curVoltage);
            Serial.println(curBrightness);
            Serial.println(result);
        }
        mState.waitForReliableBrightness();
    }
    return result;
}

void Light::checkState(time_t scheduleLoops)
{
    bool isDimming = false;

    if (scheduleLoops % mDimmingDelay == 0) {
        if (mState.isAdjustProgramRunning()) {
            mLightVoltage = adjustLight(mLightVoltage, getAbsoluteValue());
            this->setLightVoltageToOutputPin(mLightVoltage);
        } else {
            isDimming = dimLight();
        }
    }
    if ((scheduleLoops & NotifyTarget::CHECKSTATE_SELDOM) == 0 && !isDimming) {
        mState.checkForDarkness(isDarkEnoughToSwitchOnLight());
        State::checkState(scheduleLoops);
    }
}

bool Light::notifyServer(StateValue value)
{
    bool result = true;
    result = sendToServer(BRIGHTNESS_NOTIFICATION, value);
    //result = sendToServer(LIGHT_VOLTAGE, mLightVoltage + mStartVoltage);
    return result;
}

bool Light::hasMaxVoltage()
{
    return mLightVoltage >= (int16_t)(MAX_VOLTAGE - mStartVoltage);
}

int16_t Light::calcNewVoltage(int16_t voltageDiff)
{
    int16_t newVoltage = mLightVoltage;
    bool lightIsOnButMaxDimmed;

    if (voltageDiff != 0) {
        int16_t targetVoltage = ((mFullOnVoltage - mStartVoltage) * (mTargetBrightness + 10)) / 100;

        if (mLightVoltage + mStartVoltage > mFullOnVoltage) {
            newVoltage = voltageDiff > 0 ? MAX_VOLTAGE : mFullOnVoltage;
            newVoltage -= mStartVoltage;
        } else if (newVoltage < targetVoltage || voltageDiff < 0) {
            newVoltage = mLightVoltage + voltageDiff;
            newVoltage = min(newVoltage, MAX_VOLTAGE - (int16_t) mStartVoltage);
            newVoltage = max(newVoltage, 0);
            lightIsOnButMaxDimmed = mState.isUsingLight() && mMaxLightVoltage > 0 && mMaxLightVoltage < 5;
            // Prevent blinking
            if (lightIsOnButMaxDimmed && newVoltage == 0) {
                newVoltage = 1;
            }
        }
    }

    return newVoltage;
}

bool Light::changeLightVoltage(int16_t voltageDiff)
{
    int16_t newVoltage = calcNewVoltage(voltageDiff);
    bool res = false;

    if (newVoltage != mLightVoltage) {
        mMaxLightVoltage = max(mMaxLightVoltage, newVoltage);
        if (newVoltage == 0) {
            mMaxLightVoltage = 0;
        }
        mLightVoltage = newVoltage;
        setLightVoltageToOutputPin(newVoltage == 0 ? 0 : newVoltage + mStartVoltage);
        res = true;
    }
    return res;
}

void Light::setLightVoltageToOutputPin(int16_t newVoltage)
{
    analogWrite(mLightOutputPin, newVoltage);
    printVariableIfDebug(newVoltage);
}


