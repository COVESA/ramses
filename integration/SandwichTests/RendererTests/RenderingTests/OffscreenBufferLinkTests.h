//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OFFSCREENBUFFERLINKTESTS_H
#define RAMSES_OFFSCREENBUFFERLINKTESTS_H

#include "IRendererTest.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

class OffscreenBufferLinkTests : public IRendererTest
{
public:
    explicit OffscreenBufferLinkTests(bool useInterruptibleBuffers);

    void setUpTestCases(RendererTestsFramework& testFramework) final;
    bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createAndShowScene(RendererTestsFramework& testFramework, uint32_t sceneState, const ramses_internal::Vector3& camPos, uint32_t vpWidth = ramses_internal::IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = ramses_internal::IntegrationScene::DefaultViewportHeight);
    bool renderAndCompareScreenshot(RendererTestsFramework& testFramework, const ramses_internal::String& expectedImageName, uint32_t testDisplayIdx = 0u, float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel);

    enum
    {
        OffscreenBufferLinkTest_ConsumerLinkedToEmptyBuffer,
        OffscreenBufferLinkTest_ConsumersLinkedToBufferWithOneScene,
        OffscreenBufferLinkTest_OneOfTwoLinksRemoved,
        OffscreenBufferLinkTest_ConsumerLinkedToBufferWithTwoScenes,
        OffscreenBufferLinkTest_ConsumerRelinkedToAnotherBuffer,
        OffscreenBufferLinkTest_ProviderBufferDestroyed,
        OffscreenBufferLinkTest_SourceSceneHidden,
        OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown,
        OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped,
        OffscreenBufferLinkTest_ProviderSceneUsesDepthTest,
        OffscreenBufferLinkTest_ProviderSceneUsesStencilTest,
        OffscreenBufferLinkTest_SetClearColor,
        OffscreenBufferLinkTest_ConsumerLinkedToMSAABuffer,
        OffscreenBufferLinkTest_ConsumerUnlinkedMSAABuffer
    };

    ramses::sceneId_t m_sceneIdProvider;
    ramses::sceneId_t m_sceneIdProvider2;
    ramses::sceneId_t m_sceneIdConsumer;
    ramses::sceneId_t m_sceneIdConsumer2;

    const ramses_internal::Vector3 m_cameraLow{ -1.f, 2.f, 8.f };
    const ramses_internal::Vector3 m_cameraHigh{ 1.f, -2.f, 8.f };
    const ramses_internal::Vector3 m_cameraMid{ 0.f, 0.f, 8.f };

    const bool m_interruptibleBuffers;

    static constexpr uint32_t OBWidth = 256u;
    static constexpr uint32_t OBHeight = 256u;
};

#endif
