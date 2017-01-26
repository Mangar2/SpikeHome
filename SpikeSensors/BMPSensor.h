/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    BMPSensor.h
 * Purpose: Regularily check the state of a pressure sensor sensor and notify its change
 * Author:  Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "Config.h"
#include "State.h"

class BMPSensor : public State {
    public:

        /**
         * Constructs a new BMP sensor
         * @param deviceNo number of the device the sensor belongs to
         */
        BMPSensor(device_t deviceNo)
            :State(deviceNo, AIR_PRESSURE_NOTIFICATION)
        {
            mSensorAvailable = mBmp.begin();
            if (!mSensorAvailable) {
                printlnIfDebug(F("Could not find a valid BMP085 sensor, check wiring!"));
            }
        }

    private:

        /**
         * Checks if the state has changed.
         * @param curValue current state value
         * @param lastValue last state value
         * @return true, if state has changed
         */
        virtual bool hasChanged(StateValue curValue, StateValue lastValue)
        {
            return curValue.toFloat() - lastValue.toFloat() > 10;
        };

        /**
         * Reads the sensor information and notifys his changes
         */
        StateValue getValue() {
            // Barometric sensor information
            StateValue result = 0;
            if (mSensorAvailable) {
                result = uint16_t(mBmp.readPressure() / 2);
                //mTemperature = mBmp.readTemperature();
            }
            return result;
        }

        bool      mSensorAvailable;
        Adafruit_BMP085 mBmp;

};
