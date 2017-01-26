/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      Notification.cpp
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 24. Dezember 2016, 07:04
 * ---------------------------------------------------------------------------------------------------
 */

#include "Notification.h"
#include "SerialReader.h"

Notification::Notification()
:Notification(0, 0)
{
}

Notification::Notification(key_t key, StateValue value)
{
    mKey = key;
    mValue = value;
    mAcknowledge = 0;
    mSenderAddress = 0;
    mReceiverAddress = 0;
    mError = NO_ERROR;
}

Notification::Notification(key_t key, StateValue value, base_t senderAddress, base_t receiverAddress)
{
    mKey = key;
    mValue = value;
    mAcknowledge = 0;
    mSenderAddress = senderAddress;
    mReceiverAddress = receiverAddress;
    mError = NO_ERROR;
}

Notification::Notification(buffer_t buffer, base_t bytesReceived)
{
    mSenderAddress = buffer[0];
    mReceiverAddress = buffer[1];
    mAcknowledge = buffer[2];
    mKey = buffer[3];
    mValue = StateValue(buffer[4], buffer[5]);
    base_t parity = buffer[6];
    mBytesReceived = bytesReceived;
    mError = NO_ERROR;

    if (bytesReceived == 0) {
        mError = NO_DATA;
    } else if (bytesReceived != BUFFER_SIZE) {
        mError = INVALID_LENGTH_ERROR;
        mKey = 0;
    } else if (calcParity() != parity) {
        mError = PARITY_ERROR;
        mKey = 0;
    }
}

void Notification::writeToSerial(HardwareSerial* serial) const
{
    serial->write(mSenderAddress);
    serial->write(mReceiverAddress);
    serial->write(mAcknowledge);
    serial->write(mKey);
    serial->write(mValue.getIntPlaces());
    serial->write(mValue.getDecPlaces());
    serial->write(calcParity());
}

void Notification::printToSerial(HardwareSerial* serial) const
{
    serial->print(mSenderAddress);
    serial->print(F("->"));
    serial->print(mReceiverAddress);
    serial->print(F("("));
    serial->print(mAcknowledge);
    serial->print(F(") "));
    serial->print((char) mKey);
    serial->print(F(" = "));
    switch (mKey) {
        case 't':
        case 'h':
        case 's':
            serial->print(mValue.toFloat());
            break;
        case 'p':
            serial->print(mValue.toInt() * 2L);
            break;
        default:
            serial->print(mValue.toInt());
            break;
    }
    serial->println();
}

void Notification::printJsonToSerial(HardwareSerial* serial) const
{
    serial->print(F("{\"S\": "));
    serial->print(mSenderAddress);
    serial->print(F(", \"R\": "));
    serial->print(mReceiverAddress);
    serial->print(F(", \"A\": "));
    serial->print(mAcknowledge);
    serial->print(F(", \"K\": \""));
    serial->print((char) mKey);
    serial->print(F("\", \"V\": "));
    switch (mKey) {
        case 't':
        case 'h':
        case 's':
            serial->print(mValue.toFloat());
            break;
        case 'p':
            serial->print(mValue.toInt() * 2L);
            break;
        default:
            serial->print(mValue.toInt());
            break;
    }
    serial->println(F("}"));
}

bool Notification::setValueFromSerialReader(SerialReader& reader)
{
    char type = 0;
    key_t key;
    bool res = false;

    if (reader.checkNext('"') && reader.readNext()) {
        type = toupper(reader.getChar());
    }
    if (type != 0 && reader.checkNext('"') && reader.checkNext(':')) {
        if (type == 'K' && reader.checkNext('"')) {
            if (reader.readNext()) {
                key = reader.getChar();
                if (reader.checkNext('"')) {
                    mKey = key;
                    res = true;
                }
            }
        } else {
            if (reader.readValueFromCharStream()) {
                res = true;
                switch (type) {
                    case 'K': mKey = (key_t) reader.getValue();
                        break;
                    case 'S': mSenderAddress = (address_t) reader.getValue();
                        break;
                    case 'R': mReceiverAddress = (address_t) reader.getValue();
                        break;
                    case 'A': mAcknowledge = reader.getValue();
                        break;
                    case 'V': mValue = (value_t) reader.getValue();
                        break;
                    default: res = false;
                        break;
                }
            }
        }
    }
    return res;
}

bool Notification::getJsonFromSerial(HardwareSerial* serial, time_t serialSpeed)
{
    bool res = false;
    mKey = 0;
    SerialReader reader(serial, serialSpeed);
    while (reader.readNext() && reader.getChar() != '{') {
    };
    if (reader.getChar() == '{') {
        while (setValueFromSerialReader(reader)) {
            if (!reader.checkNext(',')) {
                break;
            }
        };
        res = (reader.getChar() == '}') && mKey != 0;
    }
    return res;
}

Notification::base_t Notification::calcParity() const
{
    return mSenderAddress ^ mReceiverAddress ^ mAcknowledge ^ mKey ^ mValue.getIntPlaces() ^ mValue.getDecPlaces();
}
