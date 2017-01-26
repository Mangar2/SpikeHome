/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:    RollerShutter.h
 * Purpose: Steers a roller shutter. Status is set by providing a "close" percentage.
 *          100% = fully closed. The amount of time the roller needs to move from fully open to
 *          fully closed must be configured exactly to achieve this.
 *
 * Author:  Volker Böhm
 * Copyright: Volker Böhm
 * Version: 1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __ROLLERSHUTTER_H
#define __ROLLERSHUTTER_H

#include "StdInclude.h"

class RollerShutter : public NotifyTarget {

public:

    static const time_t ACTIVITY_INTERVAL           = MILLISECONDS_IN_A_SECOND / 10;

    typedef uint8_t movement_t;
    typedef uint8_t target_t;

    /**
    * Creates a new RollerShutter class.
    * @param deviceNo, device to add the configuration values
    * @param powerPin pin to control the power on/off relais.
    * @param directionPin pin to control the up/down relais.
    */
    RollerShutter(device_t deviceNo, pin_t powerPin, pin_t directionPin);

    /**
     * Reacts on a chang signal for Signal a change on Roller Shutter settings.
     * @param key key/identifier of the change
     * @param data new value
     */
    virtual void handleChange(key_t key, StateValue data);

    /**
     * Notifys server of current roller state
     **/
    virtual bool notifyServer();

protected:
  /**
   * Sets the movement of the roller
   * @param movement MOVING_NOT, MOVING_UP, MOVING_DOWN
   */
    void setMovement(movement_t movement);

    /**
     * Moves the roller to a target state in percent;
     * 0 = fully open
     * 100 = fully closed
     */
    void moveRoller(target_t target);

    /*
     * Checks the sensor state. This function must be called regularily in the loop.
     * @param scheduleLoops number of checkState loops since reboot
     */
    virtual void checkState(time_t scheduleLoops);

    static const movement_t MOVING_NOT   = 0;
    static const movement_t MOVING_UP    = 1;
    static const movement_t MOVING_DOWN  = 2;
    pin_t       mPowerPin;
    pin_t       mDirectionPin;
    uint8_t     mRollerTime;
    float       mRollerStatus;
    movement_t  mRollerMovement;
    device_t    mDeviceNo;
    target_t    mRollerTarget;
    bool        mStatusUnknown;
    bool        mInformServer;

};

#endif // __ROLLERSHUTTER_H
