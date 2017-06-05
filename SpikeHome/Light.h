/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:        Light.h
 * Purpose:     Controls LED Light based lightning system steered by a brightness sensor and a PWM output
 *
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */


#include "StdInclude.h"
#include "State.h"
#include "BrightnessSensor.h"
#include "LightState.h"

class Brightness;

class Light : public BrightnessSensor {
public:

    /**
     * Initializes the light class
     * @param deviceNo number of the device the object belongs to
     * @param brightnessPin Analog pin number measuring the brightness
     * @param lightOutputPin Name/Number of the digital light output pin (PWM)
     */
    Light(device_t deviceNo, pin_t brightnessPin, pin_t lightOutputPin);

    /*
     * Initializes the configuration values
     */
    void initConfig();

    /**
     * Checks the sensor state. This function must be called regularily in the loop.
     * @param scheduleLoops number of checkState loops since reboot
     */
    virtual void checkState(time_t scheduleLoops);

    /**
     * Signal changes to the light.
     * @param key type of the change
     * @param value new value
     */
    virtual void handleChange(address_t senderAddress, key_t key, StateValue value);

    /**
     * Sends a notification to the server
     * @param value value to notify
     */
    virtual bool notifyServer(StateValue value);


    /**
     * Sets the target brightness in percent of the maximal achievable brightness
     * @param brightnessInPercent
     */
    void setTargetBrightness(value_t brightnessInPercent);

private:

    /**
     * Returns true, if it is dark enough to switch on the lights
     * @return true, if brightness is below a configured threshold
     */
    bool isDarkEnoughToSwitchOnLight();

    /**
     * handles a command coming from an fs20 remote control
     * @param command fs20 command to dim light
     */
    void handleFS20Command(value_t command);

    /**
     * Dims the light up or down until it reached a brightness target
     * @retun true, if currently dimming up or down. Fals if light is not changing
     */
    bool dimLight();

    /**
     * Adjusts light settings
     * @param curVoltage voltage to set to output pin to follow the adjust voltage program
     * @param curBrightness current brightness sensor value
     * @return new light voltage or 0 if adjust program is not running
     */
    int16_t adjustLight(int16_t curVoltage, int16_t curBrightness);

    /**
     * Calculates the next voltage value for the adjust light process
     * @param curVoltage current voltage
     * @param higher true, if voltage should be higher, else false
     * @return new voltage
     */
    int16_t calcNextVoltage(int16_t curVoltage, bool higher);

    /**
     * Sets voltage to the output pin by analogWrite
     * @param voltage new voltage for the PWM pin
     */
    void setLightVoltageToOutputPin(int16_t voltage);

    /**
     * Calculates the new voltage for the PWM pin
     * @param voltageDiff change of voltage
     * @return new Voltage
     */
    int16_t calcNewVoltage(int16_t voltageDiff);

    /**
     * Change the light voltage of the output pin
     * @param voltageDiff change of voltage
     * @return true, if light voltage has been changed
     */
    bool changeLightVoltage(int16_t voltageDiff);

    /**
     * Checks current system temperature and adjust lights if it gets too hot
     * @param value current system temperature of the light steering transistor
     */
    void handleSystemTemperature(StateValue value);

    /**
     * @return true, if the current PWM output voltage is the maximal possible voltage
     */
    bool hasMaxVoltage();

    /**
     * Sets a new startVoltage
     * @param startVoltage voltage in pwm units from 0..255
     */
    void setStartVoltage(value_t startVoltage);

    /**
     * Sets the delay between two dimming steps in milliseconds
     * @param dimmingDelayInMilliseconds delay between two dimming steps (10 .. 999)
     */
    void setDimmingDelay(value_t dimmingDelayInMilliseconds);

    /**
     * Sets the minimal voltage where lights are 100% on
     * @param fullOnVoltage voltage in pwm units from 0..255
     */
    void setFullOnVoltage(value_t fullOnVoltage);

    /**
     * Sets the brightness level in percent where the lamp will not turn on because it is alreay bright enough.
     * @param brightnessInPercent maximum brightness in percent when the lamp will not turn on. (30 .. 200)
     */
    void setMaximumBrightness(value_t brightnessInPercent);

    static const time_t TICKS_IN_MS_UNTIL_NEXT_INFO = 1000L * 60L * 10L;
    static const int16_t MAX_VOLTAGE = 255;
    static const uint16_t DELAY_IN_MILLISECONDS_BETWEEN_BRIGHTNESS_MEASURES = 1000;

    static const uint16_t HEAT_ALARM_OFF = 0;
    static const uint16_t HEAT_ALARM_WARNING = 1;
    static const uint16_t HEAT_ALARM_CRITICAL = 2;
    static constexpr float HEAT_HYSTERESE = 1.1;
    static constexpr float HEAT_WARNING_VALUE = 70;
    static constexpr float HEAT_CRITICAL_VALUE = 80;

    pin_t mLightOutputPin;
    uint16_t mHeatAlarm;

    LightState mState;
    int16_t mLightVoltage;
    int16_t mOldLightVoltage;
    int16_t mMaxLightVoltage;

    value_t mMaximumBrightness;
    value_t mTargetBrightness;
    value_t mStartVoltage;
    value_t mFullOnVoltage;
    value_t mDimmingDelay;
};
