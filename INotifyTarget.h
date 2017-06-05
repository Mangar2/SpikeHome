/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      INotifyTarget.h
 * Purpose:   Notification interface
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __INOTIFYTARGET_H
#define	__INOTIFYTARGET_H

class INotifyTarget {

    /**
     * Signal a change. Register change type so that the class is informed on change
     * @param key key/identifier of the change
     * @param data new value
     */
    virtual void handleChange(key_t key, StateValue data) = 0;

    /**
     * Checks the state. Once the object is registered, this function will be called regularily in the loop.
     * @param loops number of checkState loops since reboot
     */
    virtual void checkState(time_t loops) = 0;

    /**
     * remembers the class to send notifications to the server. This function will be called
     * regularily, once the object is registered.
     * @param loopCount amount of notify loops already passed. May be used to select different values to send
     * @return true, if all notification send
     */
    virtual bool notifyServer(uint16_t loopCount) = 0;
};


#endif	/* __INOTIFYTARGET_H */

