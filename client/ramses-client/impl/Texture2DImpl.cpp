//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Texture2DImpl.h"
#include "TextureUtils.h"
#include "Components/ManagedResource.h"
#include "ResourceImpl.h"
#include "ClientApplicationLogic.h"

#include <string_view>

namespace ramses
{
    Texture2DImpl::Texture2DImpl(ramses_internal::ResourceHashUsage resource,
        SceneImpl& scene,
        std::string_view name,
        ERamsesObjectType overrideType /* = ERamsesObjectType_Texture2D*/)
        : ResourceImpl(overrideType, std::move(resource), scene, name)
        , m_width(0)
        , m_height(0)
        , m_textureFormat(ETextureFormat::Invalid)
    {
    }

    Texture2DImpl::~Texture2DImpl()
    {
    }

    void Texture2DImpl::initializeFromFrameworkData(uint32_t width, uint32_t height, ETextureFormat textureFormat, const TextureSwizzle& swizzle)
    {
        m_width = width;
        m_height = height;
        m_textureFormat = textureFormat;
        m_swizzle = swizzle;
    }

    status_t Texture2DImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ResourceImpl::serialize(outStream, serializationContext));

        outStream << m_width;
        outStream << m_height;
        outStream << static_cast<uint32_t>(m_textureFormat);
        outStream << m_swizzle.channelRed;
        outStream << m_swizzle.channelGreen;
        outStream << m_swizzle.channelBlue;
        outStream << m_swizzle.channelAlpha;

        return StatusOK;
    }

    status_t Texture2DImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ResourceImpl::deserialize(inStream, serializationContext));

        inStream >> m_width;
        inStream >> m_height;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_textureFormat = static_cast<ETextureFormat>(enumInt);
        inStream >> m_swizzle.channelRed;
        inStream >> m_swizzle.channelGreen;
        inStream >> m_swizzle.channelBlue;
        inStream >> m_swizzle.channelAlpha;

        return StatusOK;
    }

    uint32_t Texture2DImpl::getWidth() const
    {
        return m_width;
    }

    uint32_t Texture2DImpl::getHeight() const
    {
        return m_height;
    }

    ETextureFormat Texture2DImpl::getTextureFormat() const
    {
        return m_textureFormat;
    }

    const TextureSwizzle& Texture2DImpl::getTextureSwizzle() const
    {
        return m_swizzle;
    }

}

