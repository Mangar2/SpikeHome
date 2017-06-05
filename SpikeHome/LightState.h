/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      LightState.h
 * Purpose:   State machine for the light class
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __LIGHTSTATE_H
#define	__LIGHTSTATE_H

class LightState {

public:

    static const uint8_t LIGHT_OFF                      = 0;
    static const uint8_t LIGHT_ON_BRIGHT                = 1;
    static const uint8_t LIGHT_KEEP_BRIGHTNESS          = 2;
    static const uint8_t LIGHT_INC_BRIGHTNESS           = 3;
    static const uint8_t LIGHT_DEC_BRIGHTNESS           = 4;
    static const uint8_t LIGHT_MEASURE_MAX_BRIGHTNESS   = 16;
    static const uint8_t LIGHT_MEASURE_MIN_VOLTAGE      = 17;
    static const uint8_t LIGHT_MEASURE_MAX_VOLTAGE      = 18;


    LightState() : mState(LIGHT_OFF), wait(0) {}

    /**
     * Adjusts the state on light switch
     * @param lightOn true, if lights are set to on
     * @param darkEnoughToSwithOnLight tru, if it is dark enough to swithc on the lights
     */
    void setLight(bool lightOn, bool darkEnoughToSwithOnLight) {
        if ((mState == LIGHT_OFF || mState == LIGHT_ON_BRIGHT) && lightOn) {
            mState = darkEnoughToSwithOnLight ? LIGHT_INC_BRIGHTNESS : LIGHT_ON_BRIGHT;
        } else if (!lightOn) {
            mState = LIGHT_OFF;
        }
        printVariableIfDebug(mState);
    }

    /**
     * Starts light adjustments
     */
    void setAdjustLight()
    {
        mState = LIGHT_MEASURE_MAX_BRIGHTNESS;
        wait = 0;
    }

    /**
     * Adjusts the state by darkness information
     * @param darkEnoughToSwithOnLight true, if it is dark enought to switch on the lights.
     */
    void checkForDarkness(bool darkEnoughToSwithOnLight)
    {
        if (mState == LIGHT_ON_BRIGHT && darkEnoughToSwithOnLight) {
            mState = LIGHT_INC_BRIGHTNESS;
        }
        printVariableIfDebug(mState);
    }

    /**
     * Adjust the state on target brightness change
     * @param oldTarget former target
     * @param newTarget new target
     */
    void targetBrightnessChanged(value_t oldTarget, value_t newTarget)
    {
        if (isUsingLight()) {
            if (oldTarget < newTarget) {
                mState = LIGHT_INC_BRIGHTNESS;
            } else if (oldTarget > newTarget) {
                mState = LIGHT_DEC_BRIGHTNESS;
            }
        }
        printVariableIfDebug(mState);
    }

    /**
     * Call on dimming step. Calculates the voltage step direction and adjusts the state
     * @param curBrightness measured brightness in %
     * @param targetBrightness calculated target brightness in %
     * @return dimming step in PWM output
     */
    int16_t dimmingStep(int16_t curBrightness, int16_t targetBrightness)
    {
        int16_t result = 0;

        switch (mState) {
            case LIGHT_INC_BRIGHTNESS:
                if (curBrightness >= targetBrightness) {
                    setStateKeepBrightness();
                    printVariableIfDebug(mState);
                } else {
                    result = 1;
                }
                break;
            case LIGHT_DEC_BRIGHTNESS:
                if (curBrightness < targetBrightness) {
                    setStateKeepBrightness();
                    printVariableIfDebug(mState);
                } else {
                    result = -1;
                }
                break;
            case LIGHT_KEEP_BRIGHTNESS:
                result = handleKeepBrightness(curBrightness, targetBrightness);
                break;
            case LIGHT_MEASURE_MIN_VOLTAGE:
                mState = LIGHT_OFF;
                break;
            case LIGHT_OFF: result = -1; break;
            default: result = 0;
        }

        return result;
    }

    /**
     * Checks if the adjust program is currently running
     * @return true, if adjust program is running
     */
    bool isAdjustProgramRunning()
    {
        return (mState >= LIGHT_MEASURE_MAX_BRIGHTNESS && mState <= LIGHT_MEASURE_MAX_VOLTAGE);
    }

    /**
     * sets a next state
     */
    void nextAdjustLightState()
    {
        if (mState >= LIGHT_MEASURE_MAX_BRIGHTNESS && mState < LIGHT_MEASURE_MAX_VOLTAGE) {
            mState++;
        } else if (mState == LIGHT_MEASURE_MAX_VOLTAGE) {
            mState = LIGHT_OFF;
        }
    }

    /**
     * Increases an internal wait counter until the current brightness measurement is reliable.
     */
    void waitForReliableBrightness()
    {
        if (isBrightnessReliable()) {
            wait = 0;
        } else {
            wait ++;
        }
    }

    /**
     * Checks if lights are switched on
     * @return true, if lights are switched on
     */
    bool isOn() { return mState != LIGHT_OFF; }

    /**
     * Checks if the lights are on. They must be switched on and it must be dark enough
     * @return true, if lights are switched on and it is dark enough
     */
    bool isUsingLight() { return isOn() && mState != LIGHT_ON_BRIGHT; }

    /**
     * Checks if lights are long enough in the same state to safely measure brightness
     * @return true, if brightness is reliable
     */
    bool isBrightnessReliable() { return wait >= WAIT_FOR_RELIABLE_BRIGHTNESS_MEASUREMENT; }

    /**
     * Gets the current state
     * @return state
     */
    uint8_t getState() { return mState; }


private:

    /**
     * Sets the state "LIGHT_KEEP_BRIGTHNESS"
     */
    void setStateKeepBrightness()
    {
        mState = LIGHT_KEEP_BRIGHTNESS;
        wait = 0;
    }

    /**
     * On Sate LIGHT_KEEP_BRIGHTNESS: Calculates the voltage step direction and adjusts the state
     * @param curBrightness measured brightness in %
     * @param targetBrightness calculated target brightness in %
     * @return dimming step in PWM output
     */
    int16_t handleKeepBrightness(int16_t curBrightness, int16_t targetBrightness)
    {
        int16_t result = 0;
        if (curBrightness + LARGE_BRIGHTNESS_DIFFERENCE < targetBrightness) {
            mState = LIGHT_INC_BRIGHTNESS;
            result = 1;
        } else if (curBrightness - LARGE_BRIGHTNESS_DIFFERENCE > targetBrightness) {
            mState = LIGHT_DEC_BRIGHTNESS;
            result = -1;
        } else if ((curBrightness + SMALL_BRIGHTNESS_DIFFERENCE > targetBrightness) ||
                   (curBrightness - SMALL_BRIGHTNESS_DIFFERENCE > targetBrightness)){
            wait = 0;
        } else {
            if (curBrightness < targetBrightness) {
                wait = max(0, wait + 1);
            }
            if (curBrightness > targetBrightness) {
                wait = min(0, wait - 1);
            }
            if (wait >= MAX_WAIT) {
                wait = 0;
                result = 1;
            }
            if (wait <= -MAX_WAIT) {
                wait = 0;
                result = -1;
            }
        }
        return result;
    }

    uint8_t mState;
    int8_t wait;
    static const int16_t LARGE_BRIGHTNESS_DIFFERENCE = 40;
    static const int16_t SMALL_BRIGHTNESS_DIFFERENCE = 15;
    static const uint8_t MAX_WAIT = 20;
    static const uint8_t WAIT_FOR_RELIABLE_BRIGHTNESS_MEASUREMENT = 100;


};


#endif	/* __LIGHTSTATE_H */

