/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      SerialTextIO.h
 * Purpose:   IO handler sending notification in readable form to the serial device
 *            usually used for debugging
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 25. Dezember 2016, 08:25
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SERIALTEXTIO_H
#define	__SERIALTEXTIO_H

#include "SerialIO.h"

class SerialTextIO : public SerialIO
{
public:
    SerialTextIO(device_t deviceAmount) : SerialIO(deviceAmount) { }

    virtual void sendNotification(const NotificationV2& notification)
    {
        notification.printJsonToSerial(&Serial);
    }

    virtual void pollNonBlocking()
    {
        NotificationV2 notification;
        if (notification.getJsonFromSerial(&Serial, mSerialSpeedInBitsPerSecond)) {
            if (notification.isAcknowledge()) {
               reply(notification);
            }
            notify(notification);
        }
    }
};



#endif	/* __SERIALTEXTIO_H */
