/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      NotificationV2.cpp
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * Created on 24. Dezember 2016, 07:04
 * ---------------------------------------------------------------------------------------------------
 */
#include "NotificationV2.h"
#include "SerialReader.h"
#include "CRC16.h"

NotificationV2::NotificationV2()
:NotificationV2(0, StateValue(uint16_t(0)))
{
}

NotificationV2::NotificationV2(key_t key, StateValue value)
{
    mKey = key;
    mValue = value;
    mSize = BUFFER_SIZE;
    mVersion = VERSION;
    mAcknowledge = 0;
    mSenderAddress = 0;
    mReceiverAddress = 0;
    mError = NO_ERROR;
}

NotificationV2::NotificationV2(key_t key, StateValue value, base_t senderAddress, base_t receiverAddress)
{
    mKey = key;
    mValue = value;
    mVersion = VERSION;
    mSize = BUFFER_SIZE;
    mAcknowledge = 0;
    mSenderAddress = senderAddress;
    mReceiverAddress = receiverAddress;
    mError = NO_ERROR;
}

NotificationV2::NotificationV2(buffer_t buffer, base_t bytesReceived)
{
    mSenderAddress = buffer[0];
    mReceiverAddress = buffer[1];
    mAcknowledge = buffer[2] & 1;
    mVersion = buffer[2] >> VERSION_SHIFT;

    switch (mVersion) {
        case 0: setVersion0(buffer, bytesReceived); break;
        case 1: setVersion1(buffer, bytesReceived); break;
        default:
            mError = ILLEGAL_VERSION;
    }
}

void NotificationV2::setVersion0(buffer_t buffer, base_t bytesReceived) 
{
    mSize = BUFFER_SIZE_V0;
    mKey = buffer[3];
    mValue = StateValue(buffer[4], buffer[5]);
    base_t parity = buffer[6];
    mBytesReceived = bytesReceived;
    mError = NO_ERROR;

    if (bytesReceived == 0) {
        mError = NO_DATA;
    } else if (bytesReceived != BUFFER_SIZE_V0) {
        mError = INVALID_LENGTH_ERROR;
        mKey = 0;
    } else if (calcParity() != parity) {
        mError = CHECK_ERROR;
        mKey = 0;
    }
}

void NotificationV2::setVersion1(buffer_t buffer, base_t bytesReceived) 
{
    mSize = buffer[3];
    mKey = buffer[4];
    mValue = StateValue(buffer[5], buffer[6]);
    check_t crc16 = buffer[7] + (buffer[8] << BITS_IN_BYTE);
    mBytesReceived = bytesReceived;
    mError = NO_ERROR;

    if (bytesReceived == 0) {
        mError = NO_DATA;
    } else if (bytesReceived != BUFFER_SIZE) {
        mError = INVALID_LENGTH_ERROR;
        mKey = 0;
    } else if (calcCRC16() != crc16) {
        mError = CHECK_ERROR;
        mKey = 0;
    }
}


void NotificationV2::writeToSerial(HardwareSerial* serial) const
{
    serial->write(mSenderAddress);
    serial->write(mReceiverAddress);
    serial->write(mAcknowledge + (mVersion << VERSION_SHIFT));
    serial->write(mSize);
    serial->write(mKey);
    serial->write(mValue.getIntPlaces());
    serial->write(mValue.getDecPlaces());
    check_t crc16 = calcCRC16();
    serial->write((uint8_t*) &crc16, sizeof(crc16));
}

void NotificationV2::printToSerial(HardwareSerial* serial) const
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

void NotificationV2::printJsonToSerial(HardwareSerial* serial) const
{
    check_t crc16 = calcCRC16();
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
    serial->print(F(", \"C\": \"0x"));
    Trace::printHex(uint8_t(crc16 >> 8));
    Trace::printHex(uint8_t(crc16));
    serial->println(F("\"}"));
}

bool NotificationV2::setValueFromSerialReader(SerialReader& reader)
{
    char type = 0;
    key_t key;
    bool res = false;

    if (reader.checkNext('"') && reader.readNext()) {
        type = toupper(reader.getChar());
    }
    if (type != 0 && reader.checkNext('"') && reader.checkNext(':')) {
        printVariableIfDebug(type);
        if (type == 'K' && reader.checkNext('"')) {
            if (reader.readNext()) {
                key = reader.getChar();
                if (reader.checkNext('"')) {
                    mKey = key;
                    printVariableIfDebug(mKey);
                    res = true;
                }
            }
        } else {
            if (reader.readValueFromCharStream()) {
                res = true;
                switch (type) {
                    case 'K': mKey = (key_t) reader.getValue();
                        printVariableIfDebug(mKey);
                        break;
                    case 'S': mSenderAddress = (address_t) reader.getValue();
                        printVariableIfDebug(mSenderAddress);
                        break;
                    case 'R': mReceiverAddress = (address_t) reader.getValue();
                        printVariableIfDebug(mReceiverAddress);
                        break;
                    case 'A': mAcknowledge = reader.getValue();
                        printVariableIfDebug(mAcknowledge);
                        break;
                    case 'V': mValue = (value_t) reader.getValue();
                        printVariableIfDebug(mValue.toInt());
                        break;
                    default: res = false;
                        break;
                }
            }
        }
    }
    return res;
}

bool NotificationV2::getJsonFromSerial(HardwareSerial* serial, time_t serialSpeed)
{
    bool res = false;
    mKey = 0;
    SerialReader reader(serial, serialSpeed);
    while (reader.readNext() && reader.getChar() != '{') {
    };
    if (reader.getChar() == '{') {
        while (setValueFromSerialReader(reader)) {
            if (reader.getChar() != ',' && !reader.checkNext(',')) {
                break;
            }
        };
        res = (reader.getChar() == '}') && mKey != 0;
    }
#ifdef DEBUG
    if (res) {
        printJsonToSerial(serial);
    }
#endif
    return res;
}

NotificationV2::check_t NotificationV2::calcCRC16() const
{
    const base_t CRC_BUFFER_SIZE = BUFFER_SIZE - sizeof(check_t);
    base_t buffer[CRC_BUFFER_SIZE];
    buffer[0] = mSenderAddress;
    buffer[1] = mReceiverAddress;
    buffer[2] = mAcknowledge + (mVersion << VERSION_SHIFT);
    buffer[3] = mSize;
    buffer[4] = mKey;
    buffer[5] = mValue.getIntPlaces();
    buffer[6] = mValue.getDecPlaces();
    check_t result = crc16(buffer, CRC_BUFFER_SIZE);
    return result;
}

Notification::base_t NotificationV2::calcParity() const
{
    return mSenderAddress ^ mReceiverAddress ^ mAcknowledge ^ mKey ^ mValue.getIntPlaces() ^ mValue.getDecPlaces();
}
