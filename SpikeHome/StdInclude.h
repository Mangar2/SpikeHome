/*
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU public licensev V3. It is furnished "as is", without any
 * support, and with no warranty, express or implied, as to its usefulness for any purpose.
 *
 * File:      StdInclude.h
 * Purpose:   Defines the basic constants and data types used in the library and includes some basic
 *            definitions
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __STDINCLUDE_H
#define __STDINCLUDE_H

#include <Arduino.h>

typedef uint8_t pin_t;
typedef uint8_t key_t;
typedef int8_t device_t;
typedef uint8_t address_t;
typedef uint16_t value_t;
typedef uint32_t time_t;
typedef int16_t pos_t;

/**
 * The following constants are limiting the amount of of sensors used in an Arduino.
 * You can safely change them if you have a look at the amount of memory available.
 * Use Trace::getFreeMemory() to view the memory left between heap and stack.
 * As long as the memory is very limited it is better to not increase space for arrays
 * dynamically because this could lead to holes in the heap and thus again less memory...
 */

/**
 * Maximal amount of devices. The amount of devices has an impact to the memory needed
 */
static const device_t MAX_DEVICE_AMOUNT = 4;

/**
 * Maximal amount of elements to notify for changes per device
 */
static const uint16_t MAX_NOTIFY_TARGETS_PER_DEVICE = 5;

/**
 * Supports tracing/debugging.
 */
#include "Trace.h"

/**
 * Basic data type to store sensor variables
 */
#include "StateValue.h"

/**
 * Basic class for all sensors
 */
#include "NotifyTarget.h"

#endif // __STDINCLUDE_H
