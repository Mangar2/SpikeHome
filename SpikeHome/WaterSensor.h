/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      WaterSensor.h
 * Purpose:   Regularily check the water detection value and notifies its change
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#include "StdInclude.h"
#include "State.h"

class WaterSensor : public State {
    public:
        /**
         * Constructs a water sensor
         * @param pin name/number of the input pin to use
         */
        WaterSensor(device_t deviceNo, pin_t pin)
          : State(deviceNo, WATER_NOTIFICATION), mPin(pin)
        {
            pinMode(pin, INPUT);
            mLastValue = HIGH;
        }

    protected:

        /**
         * Reads the status of the water sensor on an analog input pin
         * @return invertet read input (0 = no water), largest value = 32
         */
        virtual StateValue getValue() {

            value_t state = analogReadState(mPin, INVERTED);
            return (mLastValue.toInt() * 3 + state) / (4 * 32);
        }

        pin_t  mPin;

};
