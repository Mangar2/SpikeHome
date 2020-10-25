/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:        DHTSensor.h
 * Purpose:     Controls a dht sensor and provides humidity and temperature
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __DHTSENSOR_H
#define __DHTSENSOR_H

#include <StdInclude.h>

class DHTSensor : public NotifyTarget {
public:

    /**
     * Constructs a new DHT sensor
     * @param deviceNo number of the device the object belongs to
     * @param uint16_t pin name/number of the input pin to use
     * @param bool invert true, if input will be inverted (high = off, low = on)
     */
    DHTSensor(device_t deviceNo, pin_t pin);

    /**
     * Measures humidity and retuns the value
     * @return humidity
     */
    float getHumidity();

    /**
     * Meastures temperature and returns the value
     * @return measured temperature
     */
    float getTemperature();

    /**
     * Reads data from a DHT22 sensor
     * @param humidity output: humidity read
     * @param temperature output: temperature read
     * @return true, if all ok, else false
     */
    bool getValue(float& humidity, float& temperature);

private:

    typedef int8_t dht_t;
    static const time_t WAIT_FOR_WAKEUP_IN_MILLISECONDS = 2;
    static const time_t WAIT_FOR_INPUT_IN_MICROSECONDS = 30;
    static const time_t WAIT_FOR_ACKNOWLEDGE_LOOPS = 20;
    static const time_t WAIT_FOR_DATA_LOOPS = 9;

    /**
     * Requests data from sensor
     */
    void requestData();

    /**
     * Reads data from sensor into a read buffer
     * @param readBuffer buffer to read data to
     * @return true, if data read successfully, else false
     */
    bool readData(uint8_t readBuffer[5]);
    int _readSensor(uint8_t bits[5]);


    /**
     * remembers the class to send notifications to the server. This function will be called
     * regularily, once the object is registered.
     * @param loopCount amount of notify loops already passed. May be used to select different values to send
     * @return true, if notification was successful
     */
    virtual bool notifyServer(uint16_t loopCount);

    /**
     * Waits for a pin to have a dedicated signal
     * @param signal signal to wait for (HIGH or LOW)
     * @param loops amount of loops until timeout. Each loop lasts 10 microseconds
     * @return amount of loops until signal found, or -1 for timeout
     */
    int16_t waitForSignal(int16_t signal, int16_t loops);



    pin_t mPin;
    bool  mLastReadOK;
};

#endif // __DHTSENSOR_H
