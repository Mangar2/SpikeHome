/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      ActivityState.h
 * Purpose:   State machine for activity class
 *
 *
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * Version:   1.0
 * ---------------------------------------------------------------------------------------------------
 */

#ifndef __ACTIVITYSTATE_H
#define	__ACTIVITYSTATE_H

class ActivityState {

public:

    static const uint8_t ACTIVITY_OFF                   = 0;
    static const uint8_t ACTIVITY_ENTERED               = 2;
    static const uint8_t ACTIVITY_LEAVING               = 4;
    static const uint8_t ACTIVITY_IN_ROOM               = 10;
    static const uint8_t ACTIVITY_IN_ROOM2              = 12;
    static const uint8_t ACTIVITY_IN_ROOM3              = 14;


    static const uint8_t MOVETYPE_OFF                   = 0;
    static const uint8_t MOVETYPE_ROOM_MOVE             = 1;
    static const uint8_t MOVETYPE_NEAR_ENTRY_MOVE       = 2;
    static const uint8_t MOVETYPE_ENTRY_MOVE            = 3;

    ActivityState() : mState(ACTIVITY_OFF), mActiveMoveDetectors(0) {}

    /**
     * Signals switching off the lights due to a timeout
     */
    void lightTimeOut()
    {
        mState = ACTIVITY_OFF;
    }

    /**
     * Call to change state if move is detected
     * @param moveType type of move
     */
    void moveDetected(value_t moveType)
    {
        if (moveType != MOVETYPE_OFF) {
            mActiveMoveDetectors++;
        } else {
            if (mActiveMoveDetectors > 0) {
                mActiveMoveDetectors--;
            }
        }

        handleMove(moveType);
        if (moveType == MOVETYPE_OFF && mState != ACTIVITY_OFF) {
            // My move detectors are able to activate with a delay of up to 6 seconds. Activation with
            // delay irritates the presence detection.
            mIgnoreMoveTimer = MOVE_REPEAT_DELAY_IN_SECONDS *
                    (NotifyTarget::LOOPS_PER_SECOND / NotifyTarget::CHECKSTATE_NORMAL);
        }


        printVariableIfDebug(moveType);
        printVariableIfDebug(mIgnoreMoveTimer);
        printVariableIfDebug(mState);
        printVariableIfDebug(mActiveMoveDetectors);
    }

    void decreaseIgnoreMoveTimer()
    {
        if (mIgnoreMoveTimer > 0) {
            mIgnoreMoveTimer--;
        }
    }

    /**
     * Calculates a weight of the activity dependant of the state
     * @return activity weight
     */
    uint8_t getActivityWeight()
    {
        uint8_t result = 0;
        switch(mState) {
            case ACTIVITY_OFF: result = 0; break;
            case ACTIVITY_IN_ROOM: result =  2; break;
            case ACTIVITY_IN_ROOM2: result =  4; break;
            case ACTIVITY_IN_ROOM3: result =  10; break;
            case ACTIVITY_LEAVING: result = 0; break;
            case ACTIVITY_ENTERED: result =  1; break;
        }
        return result;
    }

    /**
     * Checks if a move sensor is currently active (level high).
     * @return true on active move sensor
     */
    bool isMoveActive()
    {
        return mActiveMoveDetectors != 0;
    }

private:

    /**
     * Sets the activity state based on a new move
     * @param moveType type of move
     */
    void handleMove(value_t moveType)
    {
        switch (mState) {
            case ACTIVITY_OFF: handleOff(moveType); break;
            case ACTIVITY_ENTERED: handleEntered(moveType); break;
            case ACTIVITY_IN_ROOM2:
            case ACTIVITY_IN_ROOM3:
            case ACTIVITY_IN_ROOM: handleInRoom(moveType); break;
            case ACTIVITY_LEAVING: handleLeaving(moveType); break;
        }
    }

    /**
     * Calculates the new state when current state is ACTIVITY_OFF
     * @param moveType type of move
     */
    void handleOff(value_t moveType)
    {
        switch(moveType) {
            case MOVETYPE_OFF: mActiveMoveDetectors = 0; break;
            case MOVETYPE_ROOM_MOVE:
                if (mIgnoreMoveTimer == 0) {
                    mState = ACTIVITY_IN_ROOM2;
                }
                break;
            case MOVETYPE_NEAR_ENTRY_MOVE: mState = ACTIVITY_ENTERED; break;
            case MOVETYPE_ENTRY_MOVE: mState = ACTIVITY_ENTERED; break;
        }
    }

    /**
     * Sets the new state when curren state is ACTIVITY_ENTERED
     * @param moveType type of move
     */
    void handleEntered(value_t moveType)
    {
        switch(moveType) {
            case MOVETYPE_OFF: break;
            case MOVETYPE_ROOM_MOVE: mState = ACTIVITY_IN_ROOM; break;
            case MOVETYPE_NEAR_ENTRY_MOVE: break;
            case MOVETYPE_ENTRY_MOVE: break;
        }
    }

    /**
     * Sets the new state when curren state is ACTIVITY_IN_ROOM
     * @param moveType type of move
     */
    void handleInRoom(value_t moveType)
    {
        switch(moveType) {
            case MOVETYPE_OFF: break;
            case MOVETYPE_ROOM_MOVE:
                mState = (mState == ACTIVITY_IN_ROOM) ? ACTIVITY_IN_ROOM2:  ACTIVITY_IN_ROOM3;
                break;
            case MOVETYPE_NEAR_ENTRY_MOVE: mState = ACTIVITY_IN_ROOM2; break;
            case MOVETYPE_ENTRY_MOVE: mState = ACTIVITY_LEAVING; break;
        }
    }

    /**
     * Sets the new state when curren state is ACTIVITY_LEAVING
     * @param moveType type of move
     */
    void handleLeaving(value_t moveType)
    {
        switch(moveType) {
            case MOVETYPE_OFF:
                if (mActiveMoveDetectors == 0) {
                    mState = ACTIVITY_OFF;
                }
                break;
            case MOVETYPE_ROOM_MOVE: break;
            case MOVETYPE_NEAR_ENTRY_MOVE: break;
            case MOVETYPE_ENTRY_MOVE: break;
        }
    }

    uint8_t mState;
    uint8_t mActiveMoveDetectors;
    uint8_t mIgnoreMoveTimer;

    static const uint8_t MOVE_REPEAT_DELAY_IN_SECONDS = 6;


};


#endif	/* __ACTIVITYSTATE_H */

