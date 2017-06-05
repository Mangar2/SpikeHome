/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      MovementSensor.h
 * Purpose:
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __MOVEMENTSENSOR_H
#define	__MOVEMENTSENSOR_H

class MovementSensor : public State {

public:

    /**
     * Constructs a new binary sensor
     * @param deviceNo number of the device the object belongs to
     * @param pin name/number of the input pin to use
     * @param activeValue value to send on active move
     */
    MovementSensor(device_t deviceNo, pin_t pin, uint8_t activeValue = 1)
    : State(deviceNo, MOVEMENT_NOTIFICATION), mPin(pin), mActiveValue(activeValue)
    {
        pinMode(mPin, INPUT);
        mMoveDetected = false;
        NotifyTarget::setCheckMask(NotifyTarget::CHECKSTATE_NORMAL);
    }

protected:

    /**
     * Reads the status of the input (either analog or digital)
     * @returns the status value either "LOW" or "HIGH"
     */
    virtual StateValue getValue()
    {
        uint8_t result = 0;
        if (digitalReadState(mPin, false) == HIGH)
        {
            result = mActiveValue;
            mMoveDetected = true;
        }
        return result;
    }

    /**
     * Sends a notification on change.
     * @param value changed value
     */
    virtual void notifyChange(StateValue value)
    {
        notify(MOVEMENT_NOTIFICATION, value);
        notifyAllDevices(ACTIVITY_NOTIFICATION, value);
    }

    /**
     * Polling call from schedule to generate notifications
     * @param loopCount number of schedule loop. May be used to select the type of notification if multiple
     * notifiation types should be send.
     * @return true, if the server could be notified
     */
    virtual bool notifyServer(uint16_t loopCount)
    {
        bool result = true;
        if (mMoveDetected) {
            result = broadcast(ACTIVITY_NOTIFICATION, mActiveValue);
            if (result) {
                mMoveDetected = false;
            }
        }
        return result;
    }


    pin_t mPin;
    uint8_t mActiveValue;
    bool  mMoveDetected;

};


#endif	/* __MOVEMENTSENSOR_H */

