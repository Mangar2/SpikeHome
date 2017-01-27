/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      LCDDevcie.h
 * Purpose:   Creates a new LCD Device to show Temperature, Humidity, Movement and Sys-Temp infos
 *
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 28. Dezember 2016, 07:51
 * ---------------------------------------------------------------------------------------------------
 */

#include <LiquidCrystal_I2C.h>
#include "NotifyTarget.h"

class LCDDevice : public NotifyTarget {
public:

    /**
     * Creates a new LCDDevice Object and sets its device
     * @param deviceNo device the object belongs to
     */
    LCDDevice(device_t deviceNo);

    /**
     * Reacts on a data change on type SWITCH_STATUS_KEY
     * @param key type of change
     * @param value value of change
     */
    virtual void handleChange(key_t key, StateValue value);

private:

    /**
     * Initializes the LCD Device
     */
    void init();

    LiquidCrystal_I2C lcd;


};
