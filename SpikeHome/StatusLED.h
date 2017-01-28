/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      StatusLED.h
 * Purpose:
 *
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 29. Dezember 2016, 21:49
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __STATUSLED_H
#define	__STATUSLED_H

#include "StdInclude.h"

class StatusLED : public NotifyTarget {
public:

    /**
     * Shows the status of a binary sensor by setting a pin to high/low
     * @param deviceNo number of the device of the sensor
     * @param pin pin to set LOW/HiGH on status
     * @param statusKey key of the binary object to show status
     */
    StatusLED(device_t deviceNo, pin_t pin, key_t statusKey) : NotifyTarget(deviceNo), mPin(pin), mStatusKey(statusKey)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    /**
     * Call this function to signal a change in values
     * @param key identifies the item that changed
     * @param data new value of the item
     */
    virtual void handleChange(key_t key, StateValue data)
    {
        if (key == mStatusKey) {
            digitalWrite(mPin, data.isZero() ? LOW : HIGH);
        }
    }

private:
    pin_t mPin;
    key_t mStatusKey;
};

#endif	/* __STATUSLED_H */

