//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONLOGICNOTIFIER_H
#define RAMSES_ANIMATIONLOGICNOTIFIER_H

#include "Collections/Vector.h"
#include "Animation/AnimationCommon.h"
#include "Animation/AnimationTime.h"

namespace ramses_internal
{
    class AnimationLogicListener;

    class AnimationLogicNotifier
    {
    public:
        virtual ~AnimationLogicNotifier() {}
        virtual void addListener(AnimationLogicListener* pListener);
        virtual void removeListener(AnimationLogicListener* pListener);

    protected:
        void notifyAnimationStarted(AnimationHandle handle);
        void notifyAnimationFinished(AnimationHandle handle);
        void notifyAnimationPaused(AnimationHandle handle);
        void notifyAnimationResumed(AnimationHandle handle);
        void notifyAnimationPropertiesChanged(AnimationHandle handle);
        void notifyTimeChanged(const AnimationTime& time);

        using AnimationListenerVector = std::vector<AnimationLogicListener *>;

        AnimationListenerVector m_observers;
    };
}

#endif
