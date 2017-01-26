/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
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
    mLightOn = false;
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
    if (brightnessInPercent >= 30 && brightnessInPercent <= 200) {
        mMaximumBrightness = brightnessInPercent;
        setConfigValue(MAXIMUM_BRIGHTNESS_KEY, brightnessInPercent);
    }
}

void Light::handleFS20Command(value_t value)
{
    value_t command = value / 4;
    value_t suffix  = value & 3;
    printVariableIfDebug(command);
    printVariableIfDebug(suffix);
    if (suffix == 1 && command == 0x11) {
        measureLightAndAdjustSettings();
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

void Light::handleChange(key_t key, StateValue data)
{
    bool signalOn = (!data.isZero());
    value_t value = data.toInt();
    switch (key) {
        case LIGHT_ON_NOTIFICATION:
            mLightOn = signalOn;
            if (mLightOn) {
                mDarkEnough = isDarkEnoughToSwitchOnLight();
            }
            break;
        case MOVEMENT_NOTIFICATION:
            if (mLightOn && !mDarkEnough) {
                mDarkEnough = isDarkEnoughToSwitchOnLight();
            }
            break;
        case SYS_TEMPERATURE_NOTIFICATION: handleSystemTemperature(data);
            break;
        case FS20_COMMMAND: handleFS20Command(value);
            break;
        case ADJUST_LIGHT: measureLightAndAdjustSettings();
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
        default:
            BrightnessSensor::handleChange(key, data);
            break;

    }
}

bool Light::dimLight()
{
    int16_t lightDiff;
    int16_t curBrightness = int16_t(getAbsoluteValue());
    static const int16_t cStepDiff = 30;

    if (mLightOn && mDarkEnough) {
        int16_t targetBrightness = calcAbsoluteTarget(mTargetBrightness);

        if (mHeatAlarm == HEAT_ALARM_WARNING) {
            // Switch the lamp fully on to reduce heat
            targetBrightness = MAX_ANALOG_READ_VALUE;
        }
        if (mHeatAlarm == HEAT_ALARM_CRITICAL) {
            targetBrightness = 0;
        }
        lightDiff = targetBrightness - curBrightness;
        lightDiff += (lightDiff >= 0) ? cStepDiff - 10 : -cStepDiff + 10;
        lightDiff = min(1, max(-1, lightDiff / cStepDiff));
    } else {
        lightDiff = -1;
    }
    return changeLightVoltage(lightDiff);
}

void Light::checkState(time_t scheduleLoops)
{
    bool isDimming = false;
    if (scheduleLoops % mDimmingDelay == 0) {
        isDimming = dimLight();
    }
    if ((scheduleLoops & NotifyTarget::CHECKSTATE_SELDOM) == 0 && !isDimming) {
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

void Light::measureLightAndAdjustSettings()
{
    setStartVoltage(calcStartVoltage() + 5);
    setFullOnVoltage(calcFullOnVoltage());
    setLightVoltageToOutputPin(MAX_VOLTAGE);
    mFullOnBrightness = getAverageLightIntensity();
    setConfigValue(FULL_ON_VALUE_KEY, mFullOnBrightness);
    setLightVoltageToOutputPin(0);
}

uint16_t Light::calcNeededVoltage(uint16_t targetBrightness)
{
    int16_t curVoltage = MAX_VOLTAGE / 2;
    int16_t voltageStep = curVoltage / 2;
    uint16_t curLightIntensity;

    while (voltageStep > 1) {
        setLightVoltageToOutputPin(curVoltage);
        curLightIntensity = getAverageLightIntensity();
        if (curLightIntensity > targetBrightness) {
            curVoltage -= voltageStep;
        } else {
            curVoltage += voltageStep;
        }
        voltageStep /= 2;
    }
    return curVoltage;
}

int16_t Light::calcStartVoltage()
{
    int16_t startVoltage;
    const int16_t brightnessHysterese = 5;
    setLightVoltageToOutputPin(0);
    startVoltage = calcNeededVoltage(getAverageLightIntensity() + brightnessHysterese);
    startVoltage = max(0, startVoltage - 5);
    printIfDebug(F("Start voltage found: "));
    printlnIfDebug(startVoltage);
    return startVoltage;
}

int16_t Light::calcFullOnVoltage()
{
    int16_t fullOnVoltage;
    const int16_t brightnessHysterese = 5;
    printlnIfDebug(F("Calculating full on voltage"));
    setLightVoltageToOutputPin(MAX_VOLTAGE);
    fullOnVoltage = calcNeededVoltage(getAverageLightIntensity() - brightnessHysterese);
    fullOnVoltage = min(MAX_VOLTAGE, fullOnVoltage);
    printIfDebug(F("Full on voltage found: "));
    printlnIfDebug(fullOnVoltage);
    return fullOnVoltage;
}

uint16_t Light::getAverageLightIntensity(uint16_t testAmount)
{
    uint16_t i;
    uint32_t lightIntensity = 0;
    uint16_t curIntensity = 0;
    for (i = 0; i < testAmount; i++) {
        delay(DELAY_IN_MILLISECONDS_BETWEEN_BRIGHTNESS_MEASURES);
        curIntensity = getAbsoluteValue();
        lightIntensity += curIntensity;
    }
    lightIntensity /= testAmount;
    printIfDebug(F("Calculated light Intensity: "));
    printlnIfDebug(lightIntensity);
    return uint16_t(lightIntensity);
}

bool Light::startsReduction(int16_t voltage)
{
    bool targetVoltageBelowCurrentVoltage = mLightVoltage > voltage;
    return (targetVoltageBelowCurrentVoltage && (mLightVoltage > mOldLightVoltage));
}

int16_t Light::calcNewVoltage(int16_t voltageDiff)
{

    int16_t newVoltage = mLightVoltage;
    bool lightIsOnButMaxDimmed;

    if (voltageDiff != 0) {
        if (newVoltage + mStartVoltage > mFullOnVoltage) {
            newVoltage = voltageDiff > 0 ? MAX_VOLTAGE : mFullOnVoltage;
            newVoltage -= mStartVoltage;
        } else {

            newVoltage += voltageDiff;
            newVoltage = min(newVoltage, MAX_VOLTAGE - (int16_t) mStartVoltage);
            newVoltage = max(newVoltage, 0);
            lightIsOnButMaxDimmed = mLightOn && mMaxLightVoltage > 0 && mMaxLightVoltage < 5;
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
    //printVariableIfDebug(newVoltage);
}
