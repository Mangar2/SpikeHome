/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      YahaMaster.h
 * Purpose:
 * Lan Kable Pin usage
 *            1   MovementSensor1
 *            2   Water
 *            3   DHT
 *            4   Window
 *            5   VCC
 *            6   MovementSensor0
 *            7   Ground
 *            8   BrightnessDevice
 *            
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef _YAHAMASTER_H
#define	_YAHAMASTER_H



class YahaMaster {

public:

    /**
     * Initializes the elements of the first device
     */
    static void setupDevice1()
    {
        const int16_t DEVICE_INDEX = 0;
        SpikeHome::addActivity(DEVICE_INDEX);
        SpikeHome::addLight(DEVICE_INDEX, BRIGHTNES_DEVICE1, LIGHTPWM_DEVICE1);

        SpikeHome::addMovementSensor(DEVICE_INDEX, MOVEMENT_DEVICE1_1, 1);
        SpikeHome::addMovementSensor(DEVICE_INDEX, MOVEMENT_DEVICE1_2, 3);
        SpikeHome::addDHTSensor(DEVICE_INDEX, DHT_DEVICE1);
        SpikeHome::addWaterSensor(DEVICE_INDEX, ANALOG_DEVICE1);
        SpikeHome::addWindowSensor(DEVICE_INDEX, WINDOW_DEVICE1);

        SpikeSensors::addDTSensor(DEVICE_INDEX, DALLAS_TEMPERATURE_PIN, DT_DEVICE1_INDEX);
      
    }

    /**
     * Initializes the elements of the second device
     */
    static void setupDevice2()
    {
        const int16_t DEVICE_INDEX = 1;
        SpikeHome::addActivity(DEVICE_INDEX);
        SpikeHome::addLight(DEVICE_INDEX, BRIGHTNES_DEVICE2, LIGHTPWM_DEVICE2);
        
        SpikeHome::addMovementSensor(DEVICE_INDEX, MOVEMENT_DEVICE2_1, 1);
        SpikeHome::addMovementSensor(DEVICE_INDEX, MOVEMENT_DEVICE2_2, 1);
        SpikeHome::addDHTSensor(DEVICE_INDEX, DHT_DEVICE2);
        SpikeHome::addWaterSensor(DEVICE_INDEX, ANALOG_DEVICE2);
        SpikeHome::addWindowSensor(DEVICE_INDEX, WINDOW_DEVICE1);
        
        SpikeSensors::addDTSensor(DEVICE_INDEX, DALLAS_TEMPERATURE_PIN, DT_DEVICE2_INDEX);
    }

    /**
     * Initializes the elements of the third device
     */
    static void setupDevice3()
    {
        const int16_t DEVICE_INDEX = 2;

        SpikeHome::addLight(DEVICE_INDEX, BRIGHTNES_DEVICE2, LIGHTPWM_DEVICE2);
        SpikeHome::addMovementSensor(DEVICE_INDEX, MOVEMENT_DEVICE3_1, 1);
        SpikeSensors::addDTSensor(DEVICE_INDEX, DALLAS_TEMPERATURE_PIN, DT_DEVICE3_INDEX);
      
    }


    /**
     * Initializes all sensors including the serial connection over RS485_HARDWARE_SERIAL
     */
    static void setupSensors(value_t softwareVersion)
    {
        //SpikeHome::initTextIO(softwareVersion, DEVICE_AMOUNT, SERIAL_SPEED);
        SpikeHome::initRS485(softwareVersion, DEVICE_AMOUNT, SERIAL_SPEED, SERIAL_TX_CONTROL_PIN, &Serial);

        YahaMaster::setupDevice1();
        YahaMaster::setupDevice2();
        YahaMaster::setupDevice3();
    }

private:
    static const device_t   DEVICE_AMOUNT           = 3;
    static const time_t     SERIAL_SPEED            = 57600;

    static const pin_t      LIGHTPWM_DEVICE1        = 3;
    static const pin_t      BRIGHTNES_DEVICE1       = A3;
    static const pin_t      ANALOG_DEVICE1          = A6;
    static const pin_t      WINDOW_DEVICE1          = A1;
    static const pin_t      DHT_DEVICE1             = 11;
    static const pin_t      MOVEMENT_DEVICE1_1      = 4;
    static const pin_t      MOVEMENT_DEVICE1_2      = 7;

    static const pin_t      LIGHTPWM_DEVICE2        = 5;
    static const pin_t      BRIGHTNES_DEVICE2       = A4;
    static const pin_t      ANALOG_DEVICE2          = A7;
    static const pin_t      WINDOW_DEVICE2          = A2;
    static const pin_t      DHT_DEVICE2             = 12;
    static const pin_t      MOVEMENT_DEVICE2_1      = 9;
    static const pin_t      MOVEMENT_DEVICE2_2      = 8;

    static const pin_t      LIGHTPWM_DEVICE3        = 6;
    static const pin_t      BRIGHTNES_DEVICE3       = A5;
    static const pin_t      MOVEMENT_DEVICE3_1      = 4;

    static const pin_t      SERIAL_TX_CONTROL_PIN   = 10;
    static const pin_t      DALLAS_TEMPERATURE_PIN  = 2;
    static const pin_t      RS485_HARDWARE_SERIAL   = 1;

    static const pin_t      DT_DEVICE1_INDEX        = 0;
    static const pin_t      DT_DEVICE2_INDEX        = 1;
    static const pin_t      DT_DEVICE3_INDEX        = 2;

};


#endif	/* _YAHAMASTER_H */
