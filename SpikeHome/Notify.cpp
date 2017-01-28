/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Notify.cpp
 * Author:    Volker Böhm
 * Copyright: Volker Böhm
 * ---------------------------------------------------------------------------------------------------
 */

#include "Notify.h"

Notify::Notify()
{
    mListenerAmount = 0;
}

void Notify::change(key_t key, StateValue data)
{
    amount_t i;
    for (i = 0; i < mListenerAmount; i++) {
        mListener[i]->handleChange(key, data);
    }
}

void Notify::onChange(NotifyTarget* pTarget)
{
    if (mListenerAmount < MAX_NOTIFY_TARGETS_PER_DEVICE) {
        mListener[mListenerAmount] = pTarget;
        mListenerAmount++;
    } else {
        printlnIfDebug(F("Cannot register additional notification targets"));
    }
}
