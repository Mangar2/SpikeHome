/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      StateValue.h
 * Purpose:   Stores sensor values in a two byte integer either from float or from integer
 *            Declares basic types.
 *            Floats are not used due to save memory. If memory will not be relevant in future this
 *            could be changed
 *            This implementation is not good, but memory saving. Do not:
 *            Implement operator==, operator!= or conversion operations because that might lead to
 *            comparing integers with floats with false results
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
*/

#ifndef __STATEVALUE_H
#define __STATEVALUE_H


class StateValue {
public:

  /**
   * Creates a value with a predecimal part and decimal places
   * @param intPlaces predecimal part of the value
   * @param decPlaces decimal places des zu übermittelnden wertes
   */
  StateValue(uint16_t intPlaces, uint16_t decPlaces)
  {
      setValue(intPlaces, decPlaces);
  }

  StateValue (uint16_t value)
      :valueStore(value)
  {
  }

  StateValue()
      :valueStore(0)
  {
  }

  StateValue (int16_t value)
      :valueStore(uint16_t(value))
  {
  }

  StateValue (uint8_t value)
      :valueStore(uint16_t (value))
  {
  }

  /**
   * Creates a state value from a float. The intplaces are put in the first byte
   * the dec places in the second byte. Negative values or values > 255 are not supported
   * @param value float value
   */
  StateValue(double value)
  {
      uint16_t intPlaces = floor(value);
      uint16_t decPlaces = round((value - intPlaces) * 100.0);
      setValue(intPlaces, decPlaces);
  }

  /**
   * Creates a state value from a byte array. The first of the array byte is copyied to the
   * most significant byte of the int, the second byte to the least significant byte.
   * @param data array of two bytes
   */
  StateValue(uint8_t data[2])
  {
      setValue((uint16_t) data[0], (uint16_t) data[1]);
  }

  /**
   * Checks if the value is zero
   * @return true, if the value is zero
   */
  bool isZero() const
  {
      return valueStore == 0;
  }

  /**
   * Converts the state value to an integer. Implicit conversion is explicitely not supported as
   * the value could either be an int or a float
   * @return value in integer format
   */
  uint16_t toInt() const
  {
      return valueStore;
  }

  /**
   * Gets the least significant byte of the value
   * @return least significant byte
   */
  uint8_t getDecPlaces() const {
      return valueStore & 0x00FF;
  }

  /**
   * Gets the most significant byte of the value
   * @return most significant byte
   */
  uint8_t getIntPlaces() const {
      return valueStore >> 8;
  }

  /**
   * Converts the value to a float using the most significant byte for the int places and
   * the least significant byte for the dec places
   * @return
   */
  float toFloat() const
  {
      float decPlaces = (float) (getDecPlaces()) / 100.0;
      return getIntPlaces() + decPlaces;
  }

  private:
      void setValue(uint16_t intPlaces, uint16_t decPlaces)
      {
          valueStore = (uint16_t (intPlaces) << 8) + uint16_t (decPlaces);
      }
      uint16_t valueStore;
};

#endif // __STATEVALUE_H
