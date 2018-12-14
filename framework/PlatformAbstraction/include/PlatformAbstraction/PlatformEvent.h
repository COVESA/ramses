//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMEVENT_H
#define RAMSES_PLATFORMEVENT_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformError.h>
#include <PlatformAbstraction/PlatformConditionVariable.h>
#include <PlatformAbstraction/PlatformLock.h>


namespace ramses_internal
{
    class PlatformEvent
    {
    public:
        PlatformEvent();

        void signal();
        EStatus wait(UInt32 millisec = 0);
        void broadcast();

    private:
        PlatformLightweightLock mMutex;
        PlatformConditionVariable mCondVar;
        bool mBool;
    };

    inline PlatformEvent::PlatformEvent()
        : mBool(false)
    {
    }

    inline void PlatformEvent::signal()
    {
        mMutex.lock();
        mBool = true;
        mCondVar.signal();
        mMutex.unlock();
    }

    inline void PlatformEvent::broadcast()
    {
        mMutex.lock();
        mBool = true;
        mCondVar.broadcast();
        mMutex.unlock();
    }

    inline EStatus PlatformEvent::wait(UInt32 millisec)
    {
        mMutex.lock();
        EStatus s = EStatus_RAMSES_OK;
        if (!mBool)
        {
            s = mCondVar.wait(&mMutex, millisec);
        }
        mBool = false;
        mMutex.unlock();
        return s;
    }
}

#endif
