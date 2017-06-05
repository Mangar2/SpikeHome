/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      FS20UART.h
 * Purpose:   Intetrates a UART of the FS20 System. Uses softwareserial for the serial connection
 *            to the UART. Gets FS20 Commands and translates them into internal command to switch
 *            and dim light. EXPERIMENTAL
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#include "NotifyTarget.h"

class SoftwareSerial;

#define FS20_CONV(code) (((code / 1000) - 1) << 6) + ((((code / 100) - 1) % 10) << 4) + ((((code / 10) - 1) % 10) << 2) + ((code % 10) - 1)

class FS20UART : public NotifyTarget
{
public:

    typedef uint8_t fs20_t;

    /**
     * Creates an initializes a new FS20UART Controller
     * @param deviceNo number of the device the FS20 controller is attached to
     * @param rxPin read-pin for SoftwareSerial
     * @param txPin transmit-pin for SoftwareSerial
     */
    FS20UART(device_t deviceNo, pin_t rxPin, pin_t txPin);

    /*
     * Checks the sensor state. This function must be called regularily in the loop.
     * @param scheduleLoops number of checkState loops since reboot
     */
    virtual void checkState(time_t scheduleLoops);


private:

    /**
     * Wether data receiving enabled
     */
    void enableWeatherData();

    /**
     * FS20 Data receiving enabled
     */
    void enableFS20Data();

    /**
     * Sends a getStats command to the UART that will return the Weather data
     */
    void getStats();

    /**
     * Enables the test mode, infos will be send as human readable text instead of binary data
     */
    void enableTextMode();

    /**
     * Prints a FSCode in readable form to Serial if DEBUG is defined in Trace.h
     * @param code
     */
    void printFS20Code(fs20_t code);

    fs20_t FS20AddressToArduinoAddress(fs20_t FS20Address);

    fs20_t FS20AddressToCommandSuffix(fs20_t FS20Address);

    /**
     * Gets the next character read from serial input
     * @return next character read
     */
    fs20_t getNextChar();

    /**
     * Scans a status information form FS20 UART
     */
    void scanStatus();

    /**
     * Scans a FS20 command from FS20 UART
     */
    void scanFS20();

    /**
     * Scancs a weather information form FS20 UART
     */
    void scanWeather();

    /**
     * Receives and scans an input from the RS20 UART
     */
    void scanReply();


    /**
     * Handles a command received from FS20
     * @param address address of the reciver
     * @param command command (bits 2..10) incl. suffix (bits 0..1)
     */
    void handleCommand(address_t address, value_t command);

    static const uint16_t HC1 = FS20_CONV(1232);
    static const uint16_t HC2 = FS20_CONV(2323);

    static const uint8_t STATE_WAITING = 0;
    static const uint8_t STATE_CHAR_RECEIVED = 1;

    SoftwareSerial* mpSerial;
    uint8_t     mState;
    key_t       mCommand;
    address_t   mReceiverAddress;

};
