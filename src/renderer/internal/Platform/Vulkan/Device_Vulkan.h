//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Device_Base.h"
#include "vulkan/vulkan.h"

namespace ramses::internal
{
    class IContext;

    class Device_Vulkan final : public Device_Base
    {
    public:
        explicit Device_Vulkan(IContext& context, VkInstance instance, VkSurfaceKHR surface);
        ~Device_Vulkan() override = default;

        bool init();

        void drawIndexedTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount) override;
        void drawTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount) override;

        void clear(ClearFlags clearFlags) override;
        void colorMask(bool r, bool g, bool b, bool a) override;
        void clearColor(const glm::vec4& clearColor) override;
        void clearDepth(float d) override;
        void clearStencil(int32_t s) override;
        void depthFunc(EDepthFunc func) override;
        void depthWrite(EDepthWrite flag) override;
        void scissorTest(EScissorTest state, const RenderState::ScissorRegion& region) override;
        void blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        void blendColor(const glm::vec4& color) override;
        void blendOperations(EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void cullMode(ECullMode mode) override;
        void stencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask) override;
        void stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void drawMode(EDrawMode mode) override;
        void setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        bool setConstant(DataFieldHandle field, uint32_t count, const float* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec2* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec3* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec4* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const bool* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const int32_t* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec2* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec3* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec4* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat2* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat3* value) override;
        bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat4* value) override;

        void readPixels(uint8_t* buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        DeviceResourceHandle    allocateUniformBuffer(uint32_t totalSizeInBytes) override;
        void                    uploadUniformBufferData(DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize) override;
        void                    activateUniformBuffer(DeviceResourceHandle handle, DataFieldHandle field) override;
        void                    deleteUniformBuffer(DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateVertexBuffer(uint32_t totalSizeInBytes) override;
        void                    uploadVertexBufferData(DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize) override;
        void                    deleteVertexBuffer(DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateVertexArray(const VertexArrayInfo& vertexArrayInfo) override;
        void                    activateVertexArray(DeviceResourceHandle handle) override;
        void                    deleteVertexArray(DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateIndexBuffer(EDataType dataType, uint32_t sizeInBytes) override;
        void                    uploadIndexBufferData(DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize) override;
        void                    deleteIndexBuffer(DeviceResourceHandle handle) override;

        std::unique_ptr<const GPUResource> uploadShader(const EffectResource& shader) override;
        DeviceResourceHandle    registerShader(std::unique_ptr<const GPUResource> shaderResource) override;
        DeviceResourceHandle    uploadBinaryShader(const EffectResource& shader, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) override;
        bool                    getBinaryShader(DeviceResourceHandle handleconst, std::vector<std::byte>& binaryShader, BinaryShaderFormatID& binaryShaderFormat) override;
        void                    deleteShader(DeviceResourceHandle handle) override;
        void                    activateShader(DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateTexture2D(uint32_t width, uint32_t height, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes) override;
        DeviceResourceHandle    allocateTexture3D(uint32_t width, uint32_t height, uint32_t depth, EPixelStorageFormat textureFormat, uint32_t mipLevelCount, uint32_t totalSizeInBytes) override;
        DeviceResourceHandle    allocateTextureCube(uint32_t faceSize, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes) override;
        DeviceResourceHandle    allocateExternalTexture() override;
        [[nodiscard]] DeviceResourceHandle getEmptyExternalTexture() const override;

        void                    bindTexture(DeviceResourceHandle handle) override;
        void                    generateMipmaps(DeviceResourceHandle handle) override;
        void                    uploadTextureData(DeviceResourceHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const std::byte* data, uint32_t dataSize, uint32_t stride) override;
        DeviceResourceHandle    uploadStreamTexture2D(DeviceResourceHandle handle, uint32_t width, uint32_t height, EPixelStorageFormat format, const std::byte* data, const TextureSwizzleArray& swizzle) override;
        void                    deleteTexture(DeviceResourceHandle handle) override;
        void                    activateTexture(DeviceResourceHandle handle, DataFieldHandle field) override;
        [[nodiscard]] uint32_t  getTextureAddress(DeviceResourceHandle handle) const override;

        DeviceResourceHandle    uploadRenderBuffer(uint32_t width, uint32_t height, EPixelStorageFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount) override;
        void                    deleteRenderBuffer(DeviceResourceHandle handle) override;

        DeviceResourceHandle    uploadDmaRenderBuffer(uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) override;
        int                     getDmaRenderBufferFD(DeviceResourceHandle handle) override;
        uint32_t                getDmaRenderBufferStride(DeviceResourceHandle handle) override;
        void                    destroyDmaRenderBuffer(DeviceResourceHandle handle) override;

        void                    activateTextureSamplerObject(const TextureSamplerStates& samplerStates, DataFieldHandle field) override;

        [[nodiscard]] DeviceResourceHandle getFramebufferRenderTarget() const override;
        DeviceResourceHandle    uploadRenderTarget(const DeviceHandleVector& renderBuffers) override;
        void                    activateRenderTarget(DeviceResourceHandle handle) override;
        void                    deleteRenderTarget(DeviceResourceHandle handle) override;
        void                    discardDepthStencil() override;

        void                    pairRenderTargetsForDoubleBuffering(const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers) override;
        void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        void                    blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, bool colorOnly) override;

        void                    validateDeviceStatusHealthy() const override;
        [[nodiscard]] bool      isDeviceStatusHealthy() const override;
        void                    getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const override;
        [[nodiscard]] bool      isExternalTextureExtensionSupported() const override;


        [[nodiscard]] uint32_t  getTotalGpuMemoryUsageInKB() const override;

        void                    flush() override;

    private:
        VkInstance m_instance;
        VkSurfaceKHR m_surface;
    };
}
