//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class Guid;

    class IConnectionStatusListener
    {
    public:
        virtual ~IConnectionStatusListener() = default;

        virtual void newParticipantHasConnected(const Guid& guid) = 0;
        virtual void participantHasDisconnected(const Guid& guid) = 0;
    };
}
