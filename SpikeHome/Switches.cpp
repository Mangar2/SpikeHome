/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Swithces.cpp
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "Switches.h"

Switches::Switches(device_t deviceNo, switch_t pins)
: NotifyTarget(deviceNo)
{
    init(pins);
    value_t storedValue = addConfigValue(SWITCH_STATUS_KEY, 0x00);
    setAll(storedValue);
}

Switches::switch_t Switches::getCurValues()
{
    return getConfigValue(SWITCH_STATUS_KEY);
}

void Switches::init(switch_t pins)
{
    pin_t pin;
    mPins = pins;
    for (pin = 2; pin <= 13; pin++) {
        if (pins & 1) {
            pinMode(pin, OUTPUT);
        }
        pins >>= 1;
    }
    for (pin = A0; pin <= A3; pin++) {
        if (pins & 1) {
            pinMode(pin, OUTPUT);
        }
        pins >>= 1;
    }
}

void Switches::setAll(switch_t values)
{
    pin_t pin;
    switch_t pins = mPins;
    uint8_t pinValue;
    switch_t curValues = getCurValues();
    if (curValues != values) {
        setConfigValue(SWITCH_STATUS_KEY, values);
        for (pin = 2; pin <= 13; pin++) {
            if (pins & 1) {
                pinValue = (values & 1) == 1 ? HIGH : LOW;
                digitalWrite(pin, pinValue);
                values >>= 1;
            }
            pins >>= 1;
        }
        for (pin = A0; pin <= A3; pin++) {
            if (pins & 1) {
                pinMode(pin, OUTPUT);
            }
            pins >>= 1;
        }
    }
}

void Switches::changeSwitches(switch_t values)
{
    if (values < (SET_MODE | CLEAR_MODE)) {
        switch_t newValues = getCurValues();

        if (values & SET_MODE) {
            newValues |= values;
        } else if (values & CLEAR_MODE) {
            newValues &= ~values;
        } else {
            newValues = values;
        }
        newValues &= ~(SET_MODE | CLEAR_MODE);
        setAll(newValues);
    }
}

void Switches::handleChange(key_t type, StateValue data)
{
    if (type == SWITCH_STATUS_KEY) {
        changeSwitches(data.toInt());
    }
}
