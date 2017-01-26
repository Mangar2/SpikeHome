/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      LCDDevice.cpp
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 31. Dezember 2016, 15:24
 * ---------------------------------------------------------------------------------------------------
 */

#include <Wire.h>
#include "LCDDevice.h"
#include "Clock.h"

LCDDevice::LCDDevice(device_t deviceNo) : NotifyTarget(deviceNo), lcd(0x27, 20, 4)
{
    init();
}

void LCDDevice::init()
{
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
}

void LCDDevice::handleChange(key_t key, StateValue value)
{
    Clock::timeOfDay_t buf;
    switch (key) {
        case TEMPERATURE_NOTIFICATION:
            lcd.setCursor(0, 0);
            lcd.print(F("Temp.    : "));
            lcd.print(value.toFloat());
            break;
        case HUMIDITY_NOTIFICATION:
            lcd.setCursor(0, 1);
            lcd.print(F("Humidity : "));
            lcd.print(value.toFloat());
            break;
        case SYS_TEMPERATURE_NOTIFICATION:
            lcd.setCursor(0, 2);
            lcd.print(F("SysTemp  : "));
            lcd.print(value.toFloat());
            break;
        case TIMER_NOTIFICATION:
            lcd.print(value.toInt());
            break;
        case CLOCK_NOTIFICATION:
            lcd.setCursor(0,2);
            lcd.print(F("Clock    : "));
            Clock::getTimeOfDay(buf);
            lcd.print(buf);
            break;
        default:
            lcd.setCursor(0, 3);
            lcd.print(F("Key: "));
            lcd.print(char(key));
            lcd.print(F(" Value: "));
            lcd.print(value.toInt());
            lcd.print(F("  "));
            break;
    }
}
