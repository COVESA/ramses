//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IVICONTROLLERSCREEN_H
#define RAMSES_IVICONTROLLERSCREEN_H

#include "PlatformAbstraction/PlatformTypes.h"

#include <string>

struct ivi_controller_screen;

namespace ramses_internal
{
    class IVIControllerScreen
    {
    public:
        IVIControllerScreen(ivi_controller_screen& controllerScreen, uint32_t screenId);
        ~IVIControllerScreen();

        void takeScreenshot(const std::string& fileName);
        [[nodiscard]] uint32_t getScreenId() const;

    private:
        ivi_controller_screen& m_controllerScreen;
        const uint32_t         m_screenId;
    };
}

#endif
