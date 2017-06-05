/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      BrightnessSensor.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 30. Dezember 2016, 19:03
 * ---------------------------------------------------------------------------------------------------
 */

#define DEBUG
#include "BrightnessSensor.h"

BrightnessSensor::BrightnessSensor(device_t deviceNo, pin_t analogPin)
: State(deviceNo, BRIGHTNESS_NOTIFICATION), mPin(analogPin)
{
    State::setPullup(analogPin);
    mFullOnBrightness = addConfigValue(FULL_ON_VALUE_KEY, MAX_ANALOG_READ_VALUE / 2);
}

bool BrightnessSensor::hasChanged(StateValue curValue, StateValue lastValue)
{
    return abs((int16_t) curValue.toInt() - (int16_t) lastValue.toInt()) > 5;
}

value_t BrightnessSensor::getAbsoluteValue()
{
    return State::analogReadState(mPin, State::INVERTED);
}

StateValue BrightnessSensor::getValue()
{
    uint32_t measuredValue = getAbsoluteValue();
    uint32_t dividend = measuredValue * uint32_t(MAX_ANALOG_READ_VALUE - mFullOnBrightness);
    uint32_t divisor = uint32_t(MAX_ANALOG_READ_VALUE - measuredValue) * uint32_t(mFullOnBrightness);
    value_t curValueInPercent = (100L * dividend) / divisor;
    /*
    printVariableIfDebug(curValueInPercent);
    printVariableIfDebug(mFullOnBrightness);
    printVariableIfDebug(measuredValue);
    printVariableIfDebug(dividend);
    printVariableIfDebug(divisor);
     */
    return StateValue(curValueInPercent);
}

void BrightnessSensor::handleChange(address_t senderAddress, key_t key, StateValue data)
{
    if (key == FULL_ON_VALUE_KEY) {
        value_t value = data.toInt();
        if (value > (MAX_ANALOG_READ_VALUE / 8) && value < MAX_ANALOG_READ_VALUE - 24) {
            setConfigValue(FULL_ON_VALUE_KEY, value);
            mFullOnBrightness = value;
        }
    }
};

value_t BrightnessSensor::calcAbsoluteTarget(value_t brightnessInPercent)
{
    uint32_t targetValue = uint32_t(brightnessInPercent) * mFullOnBrightness / 100L;
    uint32_t dividend = targetValue * MAX_ANALOG_READ_VALUE;
    uint32_t divisor = MAX_ANALOG_READ_VALUE - mFullOnBrightness + targetValue;
    value_t absoluteTarget = dividend / divisor;

    return absoluteTarget;
}
