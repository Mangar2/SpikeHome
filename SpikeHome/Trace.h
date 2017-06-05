/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Trace.h
 * Purpose:   Defines preprocessor elements to help for debugging
 *            #define DEBUG in your files to print debug output
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __TRACE_H
#define __TRACE_H

#include <arduino.h>

//#define DEBUG
//#define TRACE1
//#define LCDDEBUG

#ifdef DEBUG
#define printlnIfDebug(s) Serial.println(s)
#define printIfDebug(s) Serial.print(s)
#define printStringIfDebug(s) Serial.print(F(s))
#define printVariableIfDebug(v) Serial.print(F(" "#v": ")); Serial.println(v);
#elif defined LCDDEBUG
#include "LCD.h"
#define printlnIfDebug(s) Serial.println(s); /*delay(1000); LCD::print(3, s);*/
#define printIfDebug(s) Serial.print(s); /*delay(1000); LCD::print(3, s);*/
#define printVariableIfDebug(v); /* LCD::print(2, F(" "#v": ")); LCD::print(v); */
#else
#define printlnIfDebug(s)
#define printIfDebug(s)
#define printStringIfDebug(s)
#define printVariableIfDebug(v);
#endif

#if  defined(TRACE1) || defined(TRACE2)
#define traceln1(s) Serial.println(s)
#define trace1(s) Serial.print(s)
#define trace1str(s) Serial.print(F(s))
#define trace1Variable(v) Serial.print(F(" "#v": ")); Serial.print(v);
#else
#define traceln1(s)
#define trace1str(s)
#define trace1(s)
#define trace1Variable(v)
#endif

#ifdef TRACE2
#define traceln2(s) Serial.println(s)
#define trace2(s) Serial.print(s)
#define trace2str(s) Serial.print(F(s))
#else
#define traceln2(s)
#define trace2str(s)
#define trace2(s)
#endif


class Trace {
public:
    /**
     * Calculates the space between heap and stack. Does not count the available memory in heap holes
     * @return space between heap and stack
     */
    static uint16_t getFreeMemory() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    /**
     * Prints a 8 bit value in hex format (always two digit length)
     * @param data data to print
     */
    static void printHex(uint8_t data) {
        if (data < 16) {
            Serial.print(0);
        }
        Serial.print(data, HEX);
    }


    static void printFreeMemory() {
        printIfDebug(F("FM="));
        printIfDebug(getFreeMemory());
        printIfDebug(" ");
    }
};


#endif
