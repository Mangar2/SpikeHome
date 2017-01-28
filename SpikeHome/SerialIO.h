/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      SerialIO.h
 * Purpose:   Interface to handle input / output including some basic function to support serial
 *            handlers
 *            To implement an IO handler you need to derive from this class and implement the
 *            write method to send data and the pollNonBlocking method to poll for data and
 *            handle it.
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * Created on 23. Dezember 2016, 21:53
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SERIALIO_H
#define	__SERIALIO_H

#include "StdInclude.h"
#include "Notification.h"

class SerialIO
{

public:

    /**
     * Initializes a new serial io interface
     * @param deviceAmount amount of devices receiving/sending data
     */
    SerialIO(device_t deviceAmount);

    /**
     * Initializes the serial interface
     * @param pSerial pointer to a hardare serial device
     * @param serialSpeed speed of the serial interface
     */
    virtual void initSerial(HardwareSerial* pSerial, time_t serialSpeed);

    /**
     * Sends a notificaton (output)
     * @param deviceNo device number sending the command
     * @param key key of the notification
     * @param value value of the notification
     */
    void sendToServer(device_t deviceNo, key_t key, value_t value);

    /**
     * Broadcasts a notification
     * @param deviceNo device number sending the command
     * @param key key of the notification
     * @param value value of the notification
     */
    void broadcast(device_t deviceNo, key_t key, value_t value);

    /**
     * Sends a notificaton to a special device
     * @param deviceNo device number sending the command
     * @param key key of the notification
     * @param value value of the notification
     * @param receiverAddress address to receive the notification
     */
    void sendToAddress(device_t deviceNo, key_t key, value_t value, address_t receiverAddress);

    /**
     * Reads a command from serial. Non bloking -> if no command data is available or an error occured it
     * will be set to empty
     * Performs any action needed
     */
    virtual void pollNonBlocking() = 0;

    /**
     * Checks if sending is currently allowed. It is always true for the standard serial interface but
     * not for other serial or wireless IO systems
     */
    virtual bool maySend()
    {
        return true;
    }

protected:

    /**
     * sends a notification
     * @param notification notification to write
     */
    virtual void sendNotification(const Notification& notification) = 0;

    /**
     * Replies to a notification by sending the same info back to the sender
     * @param notification notification to reply to
     */
    void reply(const Notification& notification);


    /**
     * Notifies all registered sensors for the new data
     * @param deviceNo
     * @param notification
     * @return number of device this notification has been sent to
     */
    void notify(const Notification& notification);

    /**
     * Loops through the list of devices for a matching address. Returns the device number if found and -1 else
     * @param address address to search for
     * @return device number or -1 if not found
     */
    device_t getDeviceNoFromAddress(address_t address);

    static const address_t BROADCAST_ADDRESS         = 0;

    device_t  mDeviceAmount;
    address_t mReceiverAddress;
    time_t    mSerialSpeedInBitsPerSecond;
    HardwareSerial* mpSerial;
    address_t mSenderAddress[MAX_DEVICE_AMOUNT];

};

#endif	/* __SERIALIO_H */
