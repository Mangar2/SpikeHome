/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      NotifyTargetList.h
 * Purpose:   Helper file for Schedule. Holds a list of NotifyTargets. Each Targets has a timer-count
 * attachted to it to signal/show when to call its checkState Method
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 7. Januar 2017, 12:00
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __NOTIFYTARGETLIST_H
#define	__NOTIFYTARGETLIST_H

#include "StdInclude.h"

typedef uint16_t list_t;

class NotifyTargetList {

public:


    NotifyTargetList() : first(0) { };

    /**
     * Adds an element to the target list
     * @param target
     */
    void add(NotifyTarget* target)
    {
        target->setNext(first);
        first = target;
    }

    /**
     * Gets the target of a list index
     * @param index index in the list
     * @return target
     */
    NotifyTarget* getTarget(list_t index)
    {
        list_t i;
        NotifyTarget* cur;
        for (i = 0, cur = first; i < index && cur!= 0; i++, cur = cur->getNext()) { }
        return cur;
    }

    /**
     * Calls checkState functions if timer runs to zero.
     * @param loops number of checkState loops since reboot
     */
    void callCheckState(time_t loops)
    {
        NotifyTarget* cur;
        for (cur = first; cur!= 0; cur = cur->getNext()) {
            uint8_t mask = cur->getCheckMask();
            if ((loops & mask) == 0 && mask != NotifyTarget::CHECKSTATE_NEVER) {
                cur->checkState(loops);
            }
            loops ++;
        }
    }

    /**
     * Notifies all objects/sensors of a device of a configuration change
     * @param deviceNo number of device
     * @param senderAddress address of the sender
     * @param key key/identifier of the change
     * @param value new value
     */
    void notifyChange(device_t deviceNo, address_t senderAddress, key_t key, value_t value)
    {
        NotifyTarget* cur;
        for (cur = first; cur!= 0; cur = cur->getNext()) {
            if (cur->getDeviceNo() == deviceNo) {
                cur->handleChange(senderAddress, key, value);
            }
        }
    }

    /**
     * Gets the first notify target in the list
     * @return
     */
    NotifyTarget* getFirstNotifyTarget()
    {
        return first;
    }

private:
    NotifyTarget* first;
};


#endif	/* __NOTIFYTARGETLIST_H */

