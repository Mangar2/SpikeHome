/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      DHTSensor.cpp
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "DHTSensor.h"

#ifdef __DEBUG
#define IF_DEBUG(x) x
char buf[1024];
int bufIndex;
#else
#define IF_DEBUG(x)
#endif

DHTSensor::DHTSensor(device_t deviceNo, pin_t pin)
    :NotifyTarget(deviceNo), mPin(pin)
{
    mLastReadOK = true;
}

bool DHTSensor::getValue(float& humidity, float& temperature)
{
    uint8_t readBuffer[5];
    humidity = NAN;
    temperature = NAN;
    bool result = false;

    if (readData(readBuffer)) {

        humidity = word(readBuffer[0], readBuffer[1]) * 0.1;
        temperature = word(readBuffer[2] & 0x7F, readBuffer[3]) * 0.1;

        if (readBuffer[2] & 0x80)
        {
            temperature = -temperature;
        }

        if (readBuffer[4] == uint8_t(readBuffer[0] + readBuffer[1] + readBuffer[2] + readBuffer[3]))
        {
            result = true;
        }
    }
    return result;
}

void DHTSensor::requestData()
{
    pinMode(mPin, OUTPUT);
    digitalWrite(mPin, HIGH);
    delay(20);

    digitalWrite(mPin, LOW);
    delay(WAIT_FOR_WAKEUP_IN_MILLISECONDS);

    digitalWrite(mPin, HIGH);
    delayMicroseconds(WAIT_FOR_INPUT_IN_MICROSECONDS);
    pinMode(mPin, INPUT);
}

int16_t DHTSensor::waitForSignal(int16_t signal, int16_t loops)
{
    int16_t index;

    for (index = 0; index < loops; index++) {
        uint8_t pinValue = digitalRead(mPin);
        IF_DEBUG(if (bufIndex < 1023) { buf[bufIndex] = pinValue == HIGH ? '1' : '.'; bufIndex++; })
        if (pinValue == signal) {
            break;
        }
        delayMicroseconds(9);
    }
    return index == loops ? -1 : index;
}

bool DHTSensor::readData(uint8_t readBuffer[5])
{

    // INIT BUFFERVAR TO RECEIVE DATA
    bool timeout = false;
    int16_t loopCount;


    for (uint8_t i = 0; i < 5; i++) {
        readBuffer[i] = 0;
    }

    requestData();
    IF_DEBUG(bufIndex = 0;)
    
    // Turn off interrupts temporarily due to time critical section.
    cli();

    delayMicroseconds(10);
    timeout |= waitForSignal(HIGH, WAIT_FOR_ACKNOWLEDGE_LOOPS) == -1;
    timeout |= waitForSignal(LOW, WAIT_FOR_ACKNOWLEDGE_LOOPS) == -1;

    for (uint8_t i = 0; (i < 40) && !timeout; i++)
    {
        timeout |= waitForSignal(HIGH, WAIT_FOR_DATA_LOOPS) == -1;
        loopCount = waitForSignal(LOW, WAIT_FOR_DATA_LOOPS);
        timeout |= loopCount == -1;

        readBuffer[i / 8] <<= 1;
        readBuffer[i / 8] |= loopCount > 3 ? 1 : 0;
    }

    pinMode(mPin, OUTPUT);
    digitalWrite(mPin, HIGH);

    sei();

    IF_DEBUG(buf[bufIndex] = 0; Serial.println(buf);)

    return !timeout;
}

float DHTSensor::getHumidity() {
    float humidity;
    float temperature;
    getValue(humidity, temperature);
    return humidity;
 }

 float DHTSensor::getTemperature() {
    float humidity;
    float temperature;
    getValue(humidity, temperature);
    return temperature;
 }

/**
 * Sends a notification including the data to send.
 */
bool DHTSensor::notifyServer(uint16_t loopCount)
{
    float humidity;
    float temperature;
    bool readOK = getValue(humidity, temperature);

    if (!readOK || !mLastReadOK) {
        sendToServer(READ_ERROR_NOTIFICATION, readOK ? uint16_t(0) : uint16_t(1));
    }

    if (readOK) {
        if (loopCount == 0) {
            sendToServer(HUMIDITY_NOTIFICATION, humidity);
        }
        notify(HUMIDITY_NOTIFICATION, humidity);
        if (loopCount == 1) {
            sendToServer(TEMPERATURE_NOTIFICATION, temperature);
        }
        notify(TEMPERATURE_NOTIFICATION, temperature);
    }

    mLastReadOK = readOK;
    return loopCount >= 1;
};
