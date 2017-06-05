/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:    Notify.h
 * Purpose:   Holds an array of registered objects receiving change notifications if anything changes
 *            (Temperature, ...). Each device has its own notification object. The object can only manage
 *            MAX_LISTENERS amount of objects. If you want to register more you need to increase the
 *            "MAX_LISTENERS" constant
 *            Additionally Notify declares all constants used to classify notifications
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __NOTIFY_H
#define __NOTIFY_H

#include "StdInclude.h"

class NotifyTarget;

class Notify {
    public:
        typedef uint8_t amount_t;



        Notify();


        /**
         * Notifies a change of the sensor
         * @param char type type/name of the notification (see constants)
         * @param any data content of the change
         */
        void change(key_t key, StateValue data);

        /**
         * Registers an activity to receive a change info of a certain type
         * @param Target* pTarget class to send the on change notification
         */
        void onChange(NotifyTarget* pTarget);

        /**
         * Gets the amount of registered listeners
         * @return
         */
        amount_t getListenerAmount()
        {
            return mListenerAmount;
        }

    private:


        amount_t         mListenerAmount;
        NotifyTarget*    mListener[MAX_NOTIFY_TARGETS_PER_DEVICE];

};

#endif // __NOTIFY_H
