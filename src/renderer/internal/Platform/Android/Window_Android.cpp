//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Warnings.h"

#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"
#include "internal/Platform/Android/Window_Android.h"

namespace ramses::internal
{
    Window_Android::Window_Android(const DisplayConfig& displayConfig, IWindowEventHandler &windowEventHandler, uint32_t id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_nativeWindow(static_cast<ANativeWindow*>(displayConfig.getAndroidNativeWindow().getValue()))
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Android::Window_Android");
    }

    Window_Android::~Window_Android()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Android::~Window_Android");
    }

    bool Window_Android::init()
    {
        return true;
    }

    EGLNativeDisplayType Window_Android::getNativeDisplayHandle() const
    {
        return EGL_DEFAULT_DISPLAY;
    }

    ANativeWindow* Window_Android::getNativeWindowHandle() const
    {
        return m_nativeWindow;
    }

    bool Window_Android::setFullscreen([[maybe_unused]] bool fullscreen)
    {
        return true;
    }

    bool Window_Android::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        return true;
    }

    void Window_Android::handleEvents()
    {
    }
}
