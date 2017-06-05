/* ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:    Switches.h
 * Purpose: Controls a set of pins and switches them on or off. Usually to control relais or other
 *          switching devices.
 *          The pins controlled by this class are configured by providing a bitmask, the bitmask can
 *          be set by using the declared constants (PIN02, PIN03, ...). Once configured the bits
 *          can be set again by a bitmask related to the configuration.
 *          The upper three bits of che change bit mask are reserved. If they are zero the call to
 *          changeSwitches will set all switches against the provided bitmask. If SET_MODE is set
 *          bits with value 1 will be set to HIGH, if CLEAR_MODE is set bits with value 1 will be set
 *          to LOW
 *          Example: Switches(PIN11 + PIN12),
 *          changeSwitches(0x02) will set pin 11 to LOW and pin 12 to HIGH
 *          changeSwitches(SET_MODE + 0x01) will set pin 11 to HIGH and keep pin 12 unchanged
 *          changeSwitches(CLEAR_MODE + 0x01) will set pin 11 to LOW and keep pin 12 unchanged
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SWITCHES_H
#define __SWITCHES_H

#include "StdInclude.h"

class Switches : public NotifyTarget {
public:

    typedef uint16_t switch_t;

    /**
     * To configure which pins belong to this switch class
     */
    static const switch_t PIN02 = 0x0001;
    static const switch_t PIN03 = 0x0002;
    static const switch_t PIN04 = 0x0004;
    static const switch_t PIN05 = 0x0008;
    static const switch_t PIN06 = 0x0010;
    static const switch_t PIN07 = 0x0020;
    static const switch_t PIN08 = 0x0040;
    static const switch_t PIN09 = 0x0080;
    static const switch_t PIN10 = 0x0100;
    static const switch_t PIN11 = 0x0200;
    static const switch_t PIN12 = 0x0400;
    static const switch_t PIN13 = 0x0800;
    static const switch_t SW_PINA0 = 0x1000;
    static const switch_t SW_PINA1 = 0x2000;
    static const switch_t SW_PINA2 = 0x4000;
    static const switch_t SW_PINA3 = 0x8000;

    static const switch_t SET_MODE = 0x4000;
    static const switch_t CLEAR_MODE = 0x2000;


    /**
     * Create a new switch class
     *  @param deviceNo, device to add the configuration values
     * @param pins bitmask of pins controlled by this class
     */
    Switches(device_t deviceNo, switch_t pins);

    /**
     * Reacts on a data change on type SWITCH_STATUS_KEY
     * @param type, element that changed
     * @param data, new value of this element
     */
    virtual void handleChange(address_t senderAddress, key_t type, StateValue data);

    /**
     * Change current switches by applying a value.
     * @param values, value for the pins.
     */
    void changeSwitches(switch_t values);

private:
    /**
     * Initializes a new switch class
     * @param bit mask of digital pins used for switching lsb = D2, ...
     */
    void init(switch_t pins);

    /**
     * Sets all digital output pins configured to a new value.
     * @param values, value set to the pins lsb = first pin configured exampe to set pin11: init(PIN11 + PIN12), setAll(1).
     *
     */
    void setAll(switch_t values);

    /**
     * Gets current setting of values from config
     */
    switch_t getCurValues();


    device_t mDeviceNo;
    switch_t mPins;

};

#endif // __SWITCHES_H
