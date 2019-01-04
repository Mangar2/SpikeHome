
/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      CRC16.h
 * Purpose:
 * 
 * Calculate a CRC16 algorithm 
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __CRC16_H
#define	__CRC16_H

const uint16_t CCITT_CRC16_Polynome_Reversed = 0x8408;
const uint16_t CCITT_CRC16_Polynome = 0x1021;
const uint16_t BITS_IN_BYTE = 8;

#include "StdInclude.h"
#include "Notification.h"

inline bool sumOfLastBitIsOne(uint16_t crc, uint16_t data) {
    return (crc ^ data) & 0x0001;
}

inline uint16_t swapBytes(uint16_t crc) {
    return (crc << 8) | ((crc >> 8) & 0xff);
}

uint16_t crc16_reverse(uint8_t* buffer, uint8_t length)
{
    uint8_t bufferIndex;
    uint8_t shiftLoop;
    uint16_t data;
    uint16_t crc = 0xffff;
    unsigned short result;

    result = ~crc;

    if (length != 0) {
        bufferIndex = 0;
        do
        {
            data = buffer[bufferIndex];
            bufferIndex++;
            for (shiftLoop = 0; shiftLoop < BITS_IN_BYTE; shiftLoop++, data >>= 1)
            {
                bool lsbIsOne = sumOfLastBitIsOne(crc, data);
                crc >>= 1;
                if (lsbIsOne) {
                    crc ^= CCITT_CRC16_Polynome_Reversed;
                }
            }
        } while (bufferIndex < length);

        result = swapBytes(~crc); 
    }

    return result;
}

uint16_t LSByteToMSByte(uint16_t data) {
    
    return data << BITS_IN_BYTE;
}

bool isMSBSet(uint16_t data) {
    return (data & 0x8000) != 0;
}

uint16_t crc16(uint8_t* buffer, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t bufferIndex = 0; bufferIndex < length; bufferIndex++) {
        crc ^= LSByteToMSByte(buffer[bufferIndex]);
        for (uint8_t shiftLoop = 0; shiftLoop < 8; shiftLoop++) {
            if (isMSBSet(crc)) {
                crc = (crc << 1) ^ CCITT_CRC16_Polynome;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

#endif