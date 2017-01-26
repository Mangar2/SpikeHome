/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public license. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      SerialReader.h
 * Purpose:   Reads char by char from a serial device until no char is available. Timeout is calcuated
 *            from current serial speed
 *
 * Author:    Mangar
 * Copyright: Mangar
 * Version:   1.0
 * Created on 22. Januar 2017, 08:49
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __SERIALREADER_H
#define	__SERIALREADER_H

#include "StdInclude.h"

class SerialReader {

public:
    SerialReader(HardwareSerial* pSerial, time_t serialSpeed) : mpSerial(pSerial)
    {
        mTimeoutInMilliseconds = 3 + BITS_PER_CHAR * MILLISECONDS_IN_A_SECOND / serialSpeed;
        // Nothing read yet
        mTimeLastCharReadInMilliseconds = 0;
        mCur = 0;
    }

    /**
     * Reads the next byte from Serial
     * @return true, if byte read or false on timeout
     */
    bool readNext()
    {
        bool res = false;
        do {
            if (mpSerial->available())
            {
                mTimeLastCharReadInMilliseconds = millis();
                mCur = mpSerial->read();
                res = true;
            }
        } while (!res && mTimeLastCharReadInMilliseconds + mTimeoutInMilliseconds > millis());
        return res;
    }

    /**
     * Gets the char read
     * @return current char read
     */
    char getChar()
    {
        return (char) mCur;
    }

    /**
     * Check if a caracter is a space character
     * @param ch
     * @return true, if a caracter is a space character
     */
    bool isSpace(char ch)
    {
        return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
    }

    /**
     * checks if char is a digit
     * @param ch char to check
     * @return true if char is a digit
     */
    bool isDigit(char ch)
    {
        return (ch >= '0' && ch <= '9');
    }

    /**
     * returns true, if a char is a sign
     * @param ch char to check
     * @return true for + and -
     */
    bool isSign(char ch)
    {
        return (ch == '+' || ch == '-');
    }

    /**
     * Reads chars until no space/tab/newline/linefeed
     * @return true, if char available, false on timeout
     */
    bool readNextSkipSpaces()
    {
        bool charRead;
        do {
            charRead = readNext();
        } while (isSpace(getChar()) && charRead);
        return charRead;
    }

    /**
     * Reads a char skipping spaces and check if it matches
     * @param ch char to check
     * @return true, if char available and matches or false if timeout or char does not match
     */
    bool checkNext(char ch)
    {
        bool res = false;
        if (readNextSkipSpaces()) {
            res = ch == getChar();
        }
        return res;
    }

    /**
     * Reads a value in a string format (not binary)
     * @return true, if value could be read. Get Value with "getValue"
     */
    bool readValueFromCharStream()
    {
        bool res = true;
        bool negate = false;
        mValue = 0;
        if (!isSign(getChar()) && !isDigit(getChar())) {
            res = readNextSkipSpaces();
        }
        if (res && isSign(getChar())) {
            negate = getChar() == '-';
            res = readNextSkipSpaces();
        }
        if (res) {
            res = false;
            do {
                if (!isDigit(getChar())) {
                    break;
                }
                res = true;
                mValue *= 10;
                mValue += getChar() - '0';
            } while (readNext());
        }

        if (negate) {
            mValue = -mValue;
        }

        return res;
    }

    /**
     * Gets the value read with readValue
     * @return value
     */
    int32_t getValue()
    {
        return mValue;
    }

private:
    static const time_t BITS_PER_CHAR = 8;
    static const time_t MILLISECONDS_IN_A_SECOND = 1000;

    HardwareSerial* mpSerial;
    time_t mTimeoutInMilliseconds;
    time_t mTimeLastCharReadInMilliseconds;
    uint8_t mCur;
    int32_t mValue;
};


#endif	/* __SERIALREADER_H */
