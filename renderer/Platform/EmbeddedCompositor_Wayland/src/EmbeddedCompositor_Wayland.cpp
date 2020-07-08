//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufGlobal.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufBuffer.h"
#include "EmbeddedCompositor_Wayland/TextureUploadingAdapter_Wayland.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabuf.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/RendererLogContext.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"
#include "PlatformAbstraction/PlatformTime.h"
#include <unistd.h>

namespace ramses_internal
{
    EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(const RendererConfig& config, IContext& context)
        : m_waylandEmbeddedSocketName(config.getWaylandSocketEmbedded())
        , m_waylandEmbeddedSocketGroup(config.getWaylandSocketEmbeddedGroup())
        , m_waylandEmbeddedSocketFD(config.getWaylandSocketEmbeddedFD())
        , m_context(context)
        , m_compositorGlobal(*this)
        , m_iviApplicationGlobal(*this)
        , m_linuxDmabufGlobal(*this)
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(): Created EmbeddedCompositor_Wayland...(not initialized yet)");
    }

    EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland(): Destroying EmbeddedCompositor_Wayland");

        m_compositorGlobal.destroy();
        m_shellGlobal.destroy();
        m_iviApplicationGlobal.destroy();
        m_linuxDmabufGlobal.destroy();
    }

    Bool EmbeddedCompositor_Wayland::init()
    {
        if (!m_serverDisplay.init(m_waylandEmbeddedSocketName, m_waylandEmbeddedSocketGroup, m_waylandEmbeddedSocketFD))
        {
            return false;
        }

        if (!m_compositorGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if (!m_shellGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if (!m_iviApplicationGlobal.init(m_serverDisplay))
        {
            return false;
        }

        // Not all EGL implementations support the extensions necessary for dmabuf import
        if (!m_linuxDmabufGlobal.init(m_serverDisplay, m_context))
        {
            LOG_WARN(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): EGL_EXT_image_dma_buf_import not supported, skipping zwp_linux_dmabuf_v1.");
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): Embedded compositor created successfully!");

        return true;
    }

    wl_display* EmbeddedCompositor_Wayland::getEmbeddedCompositingDisplay() const
    {
        return m_serverDisplay.get();
    }

    void EmbeddedCompositor_Wayland::handleRequestsFromClients()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleRequestsFromClients(): handling pending events and requests from clients");

        m_serverDisplay.dispatchEventLoop();
    }

    Bool EmbeddedCompositor_Wayland::hasUpdatedStreamTextureSources() const
    {
        return 0u != m_updatedStreamTextureSourceIds.size();
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds(): count of pending updates for dispatching :" << m_updatedStreamTextureSourceIds.size());
        StreamTextureSourceIdSet result = m_updatedStreamTextureSourceIds;
        m_updatedStreamTextureSourceIds.clear();
        return result;
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Wayland::dispatchNewStreamTextureSourceIds()
    {
        const auto result = m_newStreamTextureSourceIds;
        m_newStreamTextureSourceIds.clear();
        return result;
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Wayland::dispatchObsoleteStreamTextureSourceIds()
    {
        const auto result = m_obsoleteStreamTextureSourceIds;
        m_obsoleteStreamTextureSourceIds.clear();
        return result;
    }

    void EmbeddedCompositor_Wayland::logInfos(RendererLogContext& context) const
    {
        context << m_surfaces.size() << " connected wayland client(s)" << RendererLogContext::NewLine;
        context.indent();
        for (auto surface: m_surfaces)
        {
            surface->logInfos(context);
        }
        context.unindent();
    }

    void EmbeddedCompositor_Wayland::addWaylandSurface(IWaylandSurface& waylandSurface)
    {
        m_surfaces.push_back(&waylandSurface);
    }

    void EmbeddedCompositor_Wayland::removeWaylandSurface(IWaylandSurface& waylandSurface)
    {
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface destroyed");
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::removeWaylandSurface() Client destroyed surface, showing fallback texture for ivi surface " << waylandSurface.getIviSurfaceId().getValue());

        // It's safe to call remove even if surface has not been mapped and
        // therefore not been added into any list, since link got initialized at
        // construction in compositor_create_surface().
        for(auto surface = m_surfaces.begin(); surface != m_surfaces.end(); ++surface)
        {
            if(*surface == &waylandSurface)
            {
                m_surfaces.erase(surface);
                break;
            }
        }
    }

    void EmbeddedCompositor_Wayland::addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        m_compositorConnections.put(&waylandCompositorConnection);
    }

    IWaylandSurface* EmbeddedCompositor_Wayland::findWaylandSurfaceByIviSurfaceId(WaylandIviSurfaceId iviSurfaceId) const
    {
        for (auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == iviSurfaceId)
            {
                return surface;
            }
        }
        return nullptr;
    }

    void EmbeddedCompositor_Wayland::endFrame(Bool notifyClients)
    {
        if (notifyClients)
        {
            LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): will send surface frame callbacks to clients");
            const UInt32 time = static_cast<UInt32>(PlatformTime::GetMillisecondsAbsolute());

            for (auto surface: m_surfaces)
            {
                surface->sendFrameCallbacks(time);
                surface->resetNumberOfCommitedFrames();
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): flusing clients");

        m_serverDisplay.flushClients();
    }

    UInt32 EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture(StreamTextureSourceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        assert(streamTextureSourceId.isValid());
        IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        assert(nullptr != waylandClientSurface);

        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture(): Stream texture with source Id " << streamTextureSourceId.getValue());
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface found for existing streamtexture: " << streamTextureSourceId.getValue());

        uploadCompositingContentForWaylandSurface(waylandClientSurface, textureHandle, textureUploadingAdapter);
        return waylandClientSurface->getNumberOfCommitedFrames();
    }

    void EmbeddedCompositor_Wayland::uploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        IWaylandBuffer* waylandBuffer = waylandSurface->getWaylandBuffer();
        assert(nullptr != waylandBuffer);

        WaylandBufferResource& waylandBufferResource = waylandBuffer->getResource();

        const UInt8* sharedMemoryBufferData = static_cast<const UInt8*>(waylandBufferResource.bufferGetSharedMemoryData());
        LinuxDmabufBufferData* linuxDmabufBuffer = LinuxDmabufBuffer::fromWaylandBufferResource(waylandBufferResource);

        if (nullptr != sharedMemoryBufferData)
        {
            const TextureSwizzleArray swizzle = {ETextureChannelColor::Blue, ETextureChannelColor::Green, ETextureChannelColor::Red, ETextureChannelColor::Alpha};
            textureUploadingAdapter.uploadTexture2D(textureHandle, waylandBufferResource.bufferGetSharedMemoryWidth(), waylandBufferResource.bufferGetSharedMemoryHeight(), ETextureFormat_RGBA8, sharedMemoryBufferData, swizzle);
        }
        else if (nullptr != linuxDmabufBuffer)
        {
            static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromLinuxDmabuf(textureHandle, linuxDmabufBuffer);
        }
        else
        {
            static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromWaylandResource(textureHandle, waylandBufferResource.getWaylandNativeResource());
        }
    }

    Bool EmbeddedCompositor_Wayland::isContentAvailableForStreamTexture(StreamTextureSourceId streamTextureSourceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        if(waylandClientSurface)
        {
            return nullptr != waylandClientSurface->getWaylandBuffer();
        }

        return false;
    }

    UInt64 EmbeddedCompositor_Wayland::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getNumberOfCommitedFramesSinceBeginningOfTime();
        }
        else
        {
            return 0;
        }
    }

    Bool EmbeddedCompositor_Wayland::isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->hasPendingBuffer();
        }
        else
        {
            return false;
        }
    }

    UInt32 EmbeddedCompositor_Wayland::getNumberOfCompositorConnections() const
    {
        return m_compositorConnections.size();
    }

    Bool EmbeddedCompositor_Wayland::hasSurfaceForStreamTexture(StreamTextureSourceId streamTextureSourceId) const
    {
        for (const auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == streamTextureSourceId)
            {
                return true;
            }
        }

        return false;
    }

    String EmbeddedCompositor_Wayland::getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getSurfaceTitle();
        }
        else
        {
            return String();
        }
    }

    IWaylandBuffer* EmbeddedCompositor_Wayland::findWaylandBuffer(WaylandBufferResource& bufferResource)
    {
        for (auto i: m_waylandBuffers)
        {
            if (i->getResource().getWaylandNativeResource() == bufferResource.getWaylandNativeResource())
            {
                return i;
            }
        }
        return nullptr;
    }

    IWaylandBuffer& EmbeddedCompositor_Wayland::getOrCreateBuffer(WaylandBufferResource& bufferResource)
    {
        IWaylandBuffer* buffer = findWaylandBuffer(bufferResource);
        if (nullptr == buffer)
        {
            buffer = new WaylandBuffer(bufferResource, *this);
            m_waylandBuffers.put(buffer);
        }
        return *buffer;
    }

    Bool EmbeddedCompositor_Wayland::isRealCompositor() const
    {
        return true;
    }

    void EmbeddedCompositor_Wayland::handleBufferDestroyed(IWaylandBuffer& buffer)
    {
        if (!m_waylandBuffers.remove(&buffer))
        {
            LOG_ERROR(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleBufferDestroyed m_waylandBuffers.remove failed");
            assert(false);
        }

        for (auto surface: m_surfaces)
        {
            surface->bufferDestroyed(buffer);
        }
    }

    void EmbeddedCompositor_Wayland::removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        const bool removed = m_compositorConnections.remove(&waylandCompositorConnection);
        UNUSED(removed)
        assert(removed);
    }

    void EmbeddedCompositor_Wayland::removeFromUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        if(m_newStreamTextureSourceIds.contains(id))
        {
            m_newStreamTextureSourceIds.remove(id);
        }
        else if (m_knownStreamTextureSoruceIds.contains(id))
        {
            m_obsoleteStreamTextureSourceIds.put(id);
        }

        m_updatedStreamTextureSourceIds.remove(id);
        m_knownStreamTextureSoruceIds.remove(id);
    }

    void EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds: new texture data for stream texture with source id " << id.getValue());
        m_updatedStreamTextureSourceIds.put(id);

        if(!m_knownStreamTextureSoruceIds.contains(id))
        {
            m_newStreamTextureSourceIds.put(id);
            m_knownStreamTextureSoruceIds.put(id);
        }
    }

    void EmbeddedCompositor_Wayland::addWaylandRegion(IWaylandRegion& waylandRegion)
    {
        m_regions.put(&waylandRegion);
    }

    void EmbeddedCompositor_Wayland::removeWaylandRegion(IWaylandRegion& waylandRegion)
    {
        const bool removed = m_regions.remove(&waylandRegion);
        UNUSED(removed)
        assert(removed);
    }
}
