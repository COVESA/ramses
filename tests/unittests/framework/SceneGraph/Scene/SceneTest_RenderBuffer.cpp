//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, hasNoRenderBuffersInitially)
    {
        EXPECT_EQ(0u, this->m_scene.getRenderBufferCount());
        EXPECT_FALSE(this->m_scene.isRenderBufferAllocated(RenderBufferHandle(0u)));
    }

    TYPED_TEST(AScene, canCreateRenderBuffer)
    {
        const RenderBufferHandle buffer(13u);
        EXPECT_EQ(buffer, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer));
        EXPECT_NE(0u, this->m_scene.getRenderBufferCount());
        EXPECT_TRUE(this->m_scene.isRenderBufferAllocated(buffer));
    }

    TYPED_TEST(AScene, canGetRenderBufferProperties)
    {
        const RenderBufferHandle buffer(13u);
        EXPECT_EQ(buffer, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer));

        const RenderBuffer& renderBuffer = this->m_scene.getRenderBuffer(buffer);
        EXPECT_EQ(1u, renderBuffer.width);
        EXPECT_EQ(2u, renderBuffer.height);
        EXPECT_EQ(EPixelStorageFormat::Depth16, renderBuffer.format);
        EXPECT_EQ(ERenderBufferAccessMode::ReadWrite, renderBuffer.accessMode);
        EXPECT_EQ(0u, renderBuffer.sampleCount);
    }

    TYPED_TEST(AScene, canReleaseRenderBuffer)
    {
        const RenderBufferHandle buffer(13u);
        EXPECT_EQ(buffer, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer));

        this->m_scene.releaseRenderBuffer(buffer);

        EXPECT_FALSE(this->m_scene.isRenderBufferAllocated(buffer));
    }

    TYPED_TEST(AScene, canSetRenderBufferProperties)
    {
        const RenderBufferHandle buffer(13u);
        EXPECT_EQ(buffer, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer));

        this->m_scene.setRenderBufferProperties(buffer, 3u, 4u, 8u);

        const RenderBuffer& renderBuffer = this->m_scene.getRenderBuffer(buffer);
        EXPECT_EQ(3u, renderBuffer.width);
        EXPECT_EQ(4u, renderBuffer.height);
        EXPECT_EQ(EPixelStorageFormat::Depth16, renderBuffer.format);
        EXPECT_EQ(ERenderBufferAccessMode::ReadWrite, renderBuffer.accessMode);
        EXPECT_EQ(8u, renderBuffer.sampleCount);
    }

    TYPED_TEST(AScene, canCreateMultipleRenderBuffers)
    {
        const RenderBufferHandle buffer1(13u);
        EXPECT_EQ(buffer1, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer1));
        ASSERT_TRUE(this->m_scene.isRenderBufferAllocated(buffer1));

        const RenderBufferHandle buffer2(17u);
        EXPECT_EQ(buffer2, this->m_scene.allocateRenderBuffer({ 3u, 4u, EPixelStorageFormat::RGBA5551, ERenderBufferAccessMode::WriteOnly, 1u }, buffer2));
        ASSERT_TRUE(this->m_scene.isRenderBufferAllocated(buffer2));

        const RenderBuffer& renderBuffer1 = this->m_scene.getRenderBuffer(buffer1);
        EXPECT_EQ(1u, renderBuffer1.width);
        EXPECT_EQ(2u, renderBuffer1.height);
        EXPECT_EQ(EPixelStorageFormat::Depth16, renderBuffer1.format);
        EXPECT_EQ(ERenderBufferAccessMode::ReadWrite, renderBuffer1.accessMode);
        EXPECT_EQ(0u, renderBuffer1.sampleCount);

        const RenderBuffer& renderBuffer2 = this->m_scene.getRenderBuffer(buffer2);
        EXPECT_EQ(3u, renderBuffer2.width);
        EXPECT_EQ(4u, renderBuffer2.height);
        EXPECT_EQ(EPixelStorageFormat::RGBA5551, renderBuffer2.format);
        EXPECT_EQ(ERenderBufferAccessMode::WriteOnly, renderBuffer2.accessMode);
        EXPECT_EQ(1u, renderBuffer2.sampleCount);
    }

    TYPED_TEST(AScene, deletingRenderBufferDoesNotAffectOtherRenderBuffers)
    {
        const RenderBufferHandle buffer1(13u);
        EXPECT_EQ(buffer1, this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, buffer1));
        EXPECT_TRUE(this->m_scene.isRenderBufferAllocated(buffer1));

        const RenderBufferHandle buffer2(17u);
        EXPECT_EQ(buffer2, this->m_scene.allocateRenderBuffer({ 3u, 4u, EPixelStorageFormat::RGBA5551, ERenderBufferAccessMode::WriteOnly, 1u }, buffer2));
        EXPECT_TRUE(this->m_scene.isRenderBufferAllocated(buffer2));

        this->m_scene.releaseRenderBuffer(buffer1);
        EXPECT_FALSE(this->m_scene.isRenderBufferAllocated(buffer1));
        ASSERT_TRUE(this->m_scene.isRenderBufferAllocated(buffer2));

        const RenderBuffer& renderBuffer = this->m_scene.getRenderBuffer(buffer2);
        EXPECT_EQ(3u, renderBuffer.width);
        EXPECT_EQ(4u, renderBuffer.height);
        EXPECT_EQ(EPixelStorageFormat::RGBA5551, renderBuffer.format);
        EXPECT_EQ(ERenderBufferAccessMode::WriteOnly, renderBuffer.accessMode);
        EXPECT_EQ(1u, renderBuffer.sampleCount);
    }
}
