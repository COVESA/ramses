//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREXECUTORINTERNALSTATE_H
#define RAMSES_RENDEREXECUTORINTERNALSTATE_H

#include "Math3d/CameraMatrixHelper.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/Viewport.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "RendererAPI/RenderingContext.h"
#include "RendererLib/FrameTimer.h"
#include "RenderExecutorInternalRenderStates.h"

namespace ramses_internal
{
    class IDevice;
    class RendererCachedScene;

    template <typename STATETYPE> inline static STATETYPE DefaultStateValue()
    {
        return STATETYPE();
    }

    template <> inline glm::vec4 DefaultStateValue<glm::vec4>()
    {
        return glm::vec4(std::numeric_limits<float>::max());
    }

    template <> inline ColorWriteMask DefaultStateValue<ColorWriteMask>()
    {
        return std::numeric_limits<ColorWriteMask>::max();
    }

    template <> inline EDepthFunc DefaultStateValue<EDepthFunc>()
    {
        return EDepthFunc::NUMBER_OF_ELEMENTS;
    }

    template <> inline EDepthWrite DefaultStateValue<EDepthWrite>()
    {
        return EDepthWrite::NUMBER_OF_ELEMENTS;
    }

    template <> inline ECullMode DefaultStateValue<ECullMode>()
    {
        return ECullMode::NUMBER_OF_ELEMENTS;
    }

    template <typename STATETYPE>
    struct CachedState
    {
    public:
        explicit CachedState(const STATETYPE& initialState = DefaultStateValue<STATETYPE>())
            : m_state(initialState)
            , m_changed(true)
        {
        }

        void setState(const STATETYPE& state)
        {
            m_changed = false;
            if (state != m_state)
            {
                m_state = state;
                m_changed = true;
            }
        }

        [[nodiscard]] const STATETYPE& getState() const
        {
            return m_state;
        }

        [[nodiscard]] bool hasChanged() const
        {
            return m_changed;
        }

        void reset()
        {
            m_changed = true;
            m_state = DefaultStateValue<STATETYPE>();
        }

    private:
        STATETYPE m_state;
        bool      m_changed;
    };

    // RenderExecutor is _stateless_, this only keeps an internal state during execution
    // to avoid copying it around
    class RenderExecutorInternalState
    {
    public:
        RenderExecutorInternalState(IDevice& device, RenderingContext& renderContext, const FrameTimer* frameTimer = nullptr);

        [[nodiscard]] IDevice&                   getDevice() const;

        void                       setScene(const RendererCachedScene& scene);
        [[nodiscard]] const RendererCachedScene& getScene() const;
        RenderingContext&          getRenderingContext();

        [[nodiscard]] const glm::mat4&           getProjectionMatrix() const;
        [[nodiscard]] const glm::vec3&           getCameraWorldPosition() const;
        [[nodiscard]] const glm::mat4&           getViewMatrix() const;
        [[nodiscard]] const glm::mat4&           getModelMatrix() const;
        [[nodiscard]] const glm::mat4&           getModelViewMatrix() const;
        [[nodiscard]] const glm::mat4&           getModelViewProjectionMatrix() const;

        void                       setCamera(CameraHandle camera);

        void                       setRenderable(RenderableHandle renderable);
        [[nodiscard]] RenderableHandle           getRenderable() const;

        [[nodiscard]] bool hasExceededTimeBudgetForRendering() const;

        CachedState < DeviceResourceHandle >    shaderDeviceHandle;
        DeviceResourceHandle                    vertexArrayDeviceHandle;
        bool                                    vertexArrayUsesIndices = false;
        CachedState < ScissorState >            scissorState;
        CachedState < EDepthFunc >              depthFuncState;
        CachedState < EDepthWrite >             depthWriteState;
        CachedState < StencilState >            stencilState;
        CachedState < BlendOperationsState >    blendOperationsState;
        CachedState < BlendFactorsState >       blendFactorsState;
        CachedState < glm::vec4 >               blendColorState;
        CachedState < ColorWriteMask >          colorWriteMaskState;
        CachedState < ECullMode >               cullModeState;
        CachedState < RenderTargetHandle >      renderTargetState;
        CachedState < RenderPassHandle >        renderPassState;
        CachedState < Viewport >                viewportState;
        EDrawMode                               drawMode = EDrawMode::NUMBER_OF_ELEMENTS;

        SceneRenderExecutionIterator            m_currentRenderIterator;

    private:
        IDevice&                    m_device;
        const RendererCachedScene*  m_scene;
        RenderingContext&           m_renderContext;

        RenderableHandle            m_renderable;

        glm::mat4                   m_projectionMatrix;
        glm::mat4                   m_viewMatrix;
        glm::mat4                   m_modelMatrix;
        glm::mat4                   m_modelViewMatrix;
        glm::mat4                   m_modelViewProjectionMatrix;
        glm::vec3                   m_cameraWorldPosition;

        CachedState < CameraHandle >       m_camera;

        const FrameTimer* const            m_frameTimer;
    };

    inline RenderableHandle RenderExecutorInternalState::getRenderable() const
    {
        return m_renderable;
    }

    inline IDevice& RenderExecutorInternalState::getDevice() const
    {
        return m_device;
    }

    inline void RenderExecutorInternalState::setScene(const RendererCachedScene& scene)
    {
        m_scene = &scene;
    }

    inline const RendererCachedScene& RenderExecutorInternalState::getScene() const
    {
        assert(nullptr != m_scene);
        return *m_scene;
    }

    inline RenderingContext& RenderExecutorInternalState::getRenderingContext()
    {
        return m_renderContext;
    }

    inline const glm::mat4& RenderExecutorInternalState::getProjectionMatrix() const
    {
        return m_projectionMatrix;
    }

    inline const glm::vec3& RenderExecutorInternalState::getCameraWorldPosition() const
    {
        return m_cameraWorldPosition;
    }

    inline const glm::mat4& RenderExecutorInternalState::getViewMatrix() const
    {
        return m_viewMatrix;
    }

    inline const glm::mat4& RenderExecutorInternalState::getModelMatrix() const
    {
        return m_modelMatrix;
    }

    inline const glm::mat4& RenderExecutorInternalState::getModelViewMatrix() const
    {
        return m_modelViewMatrix;
    }

    inline const glm::mat4& RenderExecutorInternalState::getModelViewProjectionMatrix() const
    {
        return m_modelViewProjectionMatrix;
    }

    inline bool RenderExecutorInternalState::hasExceededTimeBudgetForRendering() const
    {
        return m_frameTimer != nullptr ? m_frameTimer->isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender) : false;
    }
}

#endif
