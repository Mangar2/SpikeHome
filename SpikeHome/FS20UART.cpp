/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      FS20UART.cpp
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

//#define DEBUG
#include <SoftwareSerial.h>

#include "FS20UART.h"
#include "Device.h"

FS20UART::FS20UART(device_t deviceNo, pin_t rxPin, pin_t txPin)
:NotifyTarget(deviceNo), mState(STATE_WAITING), mReceiverAddress(0)
{
    mpSerial = new SoftwareSerial(rxPin, txPin);
    mpSerial->begin(4800);
    delay(100);
    enableFS20Data();
    getStats();
    NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_ALLWAYS);
}

void FS20UART::getStats()
{
     // Returns status of weather data receiver
    mpSerial->print(F("\x02\x01\xF0"));
    delay(10);
}


void FS20UART::enableFS20Data()
{
    // Receive FS20 data and transmit immediately
    mpSerial->print(F("\x02\x02\xF1\x01"));
    delay(10);
}


void FS20UART::enableWeatherData()
{
    // Receive weather data and transmit immediately
    mpSerial->print(F("\x02\x02\xF2\x01"));
    delay(10);
}

void FS20UART::enableTextMode()
{
    mpSerial->print(F("\x02\x02\xFB\x01"));
    delay(10);

}

void FS20UART::printFS20Code(uint8_t code)
{
    int shift;
    for (shift = 6; shift >= 0; shift -= 2) {
        printIfDebug(((code >> shift) & 3 ) + 1);
    }
}

uint8_t FS20UART::getNextChar()
{
    uint8_t c = mpSerial->read();
    return c;
}

void FS20UART::scanStatus()
{
    #ifdef DEBUG
    uint8_t status = getNextChar();
    uint8_t baudRate = getNextChar();
    uint8_t FS20PackageAmount = getNextChar();
    uint8_t WeatherPackageAmount = getNextChar();

    printIfDebug(F("Status: ")); printIfDebug(status);
    printIfDebug(F(" Baud: ")); printIfDebug(baudRate);
    printIfDebug(F(" FS20: ")); printIfDebug(FS20PackageAmount);
    printIfDebug(F(" Weather: ")); printIfDebug(WeatherPackageAmount);
    printlnIfDebug("");
    #endif

}

void FS20UART::handleCommand(address_t address, value_t command)
{
    device_t deviceNo = Device::addressToIndex(address);
    printVariableIfDebug(deviceNo);
    if (deviceNo != -1) {
        if ((command & 3) == 3) {
            for (deviceNo = 0; deviceNo < Device::getDeviceAmount(); deviceNo++) {
                Device::getNotify(deviceNo).change(FS20_COMMMAND, command);
            }
        } else {
            printVariableIfDebug(command);
            Device::getNotify(deviceNo).change(FS20_COMMMAND, command);
        }
    } else {
        mReceiverAddress = address;
        mCommand = command;
    }
}

uint8_t FS20UART::FS20AddressToArduinoAddress(uint8_t FS20Address)
{
    return FS20Address / 4;
}

uint8_t FS20UART::FS20AddressToCommandSuffix(uint8_t FS20Address)
{
    return FS20Address & 3;
}


void FS20UART::scanFS20()
{
    uint8_t homeCode1 = getNextChar();
    uint8_t homeCode2 = getNextChar();
    uint8_t address = getNextChar();
    value_t command = getNextChar();

    // command extension is not used here
    getNextChar();

    printIfDebug(F(" HC: ")); printFS20Code(homeCode1); printFS20Code(homeCode2);
    printIfDebug(F(" ADR: ")); printIfDebug(FS20AddressToArduinoAddress(address));
    printIfDebug(F(" SUF: ")); printIfDebug(FS20AddressToCommandSuffix(address));
    printIfDebug(F(" CMD: ")); printIfDebug(command);
    printlnIfDebug("");
    if (homeCode1 == HC1 && homeCode2 == HC2) {
        handleCommand(
            FS20AddressToArduinoAddress(address),
            command * 4 + FS20AddressToCommandSuffix(address)
        );
    }

}

void FS20UART::scanWeather()
{

}

void FS20UART::scanReply()
{
    uint8_t len = getNextChar();
    uint8_t command = getNextChar();
    switch(command) {
    case 0xA0: if (len == 0x05) { scanStatus(); } break;
    case 0xA1: if (len == 0x06) { scanFS20(); }  break;
    case 0xA2: if (len == 0x0C) { scanWeather(); } break;
    default: break;
    }
}


void FS20UART::checkState(uint32_t scheduleLoops)
{
    if (mReceiverAddress != 0) {
        if (sendToAddress(FS20_COMMMAND, mCommand, mReceiverAddress)) {
            mReceiverAddress = 0;
        }
    }
    if (mpSerial->available()) {
        if (mState == STATE_WAITING) {
            // Wait for next call to ensure all chars are received
            mState = STATE_CHAR_RECEIVED;
        } else {
            uint8_t c = getNextChar();
            if (c == 0x02) {
                scanReply();
            }
            mState = STATE_WAITING;
        }
    } else {
        mState = STATE_WAITING;
    }
}
