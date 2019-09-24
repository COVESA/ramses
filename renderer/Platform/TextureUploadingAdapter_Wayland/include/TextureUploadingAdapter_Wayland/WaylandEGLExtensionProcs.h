//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDEGLEXTENSIONPROCS_H
#define RAMSES_WAYLANDEGLEXTENSIONPROCS_H

#include "Utils/Warnings.h"

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wdeprecated-declarations)
#include "wayland-egl.h"
WARNINGS_POP

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "Collections/String.h"

#ifndef EGL_WAYLAND_BUFFER_WL
    #define EGL_WAYLAND_BUFFER_WL 0x31D5
#endif
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);

namespace ramses_internal
{
    class WaylandEGLExtensionProcs
    {
    public:
        WaylandEGLExtensionProcs(wl_display* waylandWindowDisplay);

        EGLImageKHR eglCreateImageKHR(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint* attributeList) const;
        EGLBoolean eglDestroyImageKHR(EGLImageKHR image) const;
        void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) const;
        EGLBoolean eglBindWaylandDisplayWL(wl_display* waylandDisplay) const;
        EGLBoolean eglUnbindWaylandDisplayWL(wl_display* waylandDisplay) const;

        bool areExtensionsSupported()const;

    private:

        static bool CheckExtensionAvailable(const ramses_internal::String& eglExtensions, const ramses_internal::String& extensionName);

        const EGLDisplay m_eglDisplay;

        PFNEGLCREATEIMAGEKHRPROC m_eglCreateImageKHR;
        PFNEGLDESTROYIMAGEKHRPROC m_eglDestroyImageKHR;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_glEGLImageTargetTexture2DOES;
        PFNEGLBINDWAYLANDDISPLAYWL m_eglBindWaylandDisplayWL;
        PFNEGLUNBINDWAYLANDDISPLAYWL m_eglUnbindWaylandDisplayWL;

        bool m_extensionsSupported;
    };
}

#endif
