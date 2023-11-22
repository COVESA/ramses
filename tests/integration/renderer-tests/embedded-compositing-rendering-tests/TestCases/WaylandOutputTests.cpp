//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandOutputTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "WaylandOutputTestParams.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "wayland-server-protocol.h"

namespace ramses::internal
{
    WaylandOutputTests::WaylandOutputTests() = default;

    void WaylandOutputTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCase(WaylandOutput_Version1, *this, "WaylandOutput_Version1");
        testFramework.createTestCase(WaylandOutput_Version2, *this, "WaylandOutput_Version2");
        testFramework.createTestCase(WaylandOutput_Version3, *this, "WaylandOutput_Version3");
    }

    bool WaylandOutputTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        switch(testCase.m_id)
        {

        case WaylandOutput_Version1:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(1);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= CheckWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, false, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case WaylandOutput_Version2:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(2);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= CheckWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, true, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case WaylandOutput_Version3:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(3);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= CheckWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, true, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        default:
            assert(false);
        }

        return testResultValue;
    }

    bool WaylandOutputTests::CheckWaylandOutputParams(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight, bool expectMode, bool expectScale, bool expectDone)
    {
        bool success = true;

        const bool geometryActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_GeometryReceived) != 0;
        const bool modeActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ModeReceived) != 0;
        const bool scaleActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ScaleReceived) != 0;
        const bool doneActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived) != 0;

        if(!geometryActuallyReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: geometry was not received!");
            success = false;
        }

        if(modeActuallyReceived != expectMode)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: expected mode received :{}, actual mode received :{}",
                expectMode, modeActuallyReceived);
            success = false;
        }

        if(scaleActuallyReceived != expectScale)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: expected scale received :{}, actual scale received :{}",
                expectScale, scaleActuallyReceived);
            success = false;
        }

        if(doneActuallyReceived != expectDone)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: expected done received :{}, actual done received :{}",
                expectDone, doneActuallyReceived);
            success = false;
        }

        if(!CheckWaylandOutputGeometry(waylandOutputParams))
            success = false;

        if(expectMode && !CheckWaylandOutputMode(waylandOutputParams, expectedWidth, expectedHeight))
            success = false;

        if(expectScale && !CheckWaylandOutputScale(waylandOutputParams))
            success = false;

        return success;
    }

    bool WaylandOutputTests::CheckWaylandOutputGeometry(const WaylandOutputTestParams& waylandOutputParams)
    {

        const int32_t xExpected = 0;
        const int32_t yExpected = 0;
        const int32_t physicalWidthExpected = 0;
        const int32_t physicalHeightExpected= 0;
        const int32_t subpixelExpected = WL_OUTPUT_SUBPIXEL_UNKNOWN;
        const int32_t transformExpected = WL_OUTPUT_TRANSFORM_NORMAL;

        if(waylandOutputParams.x != xExpected || waylandOutputParams.y != yExpected
            || waylandOutputParams.physicalWidth != physicalWidthExpected || waylandOutputParams.physicalHeight != physicalHeightExpected
            || waylandOutputParams.subpixel != subpixelExpected || waylandOutputParams.transform != transformExpected)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: wrong values received"
                ": x = {} (expected = {})"
                ", y = {} (expected = {})"
                ", physical_width = {} (expected = {})"
                ", physical_height = {} (expected = {})"
                ", subpixel format = {} (expected = {})"
                ", transform = {} (expected = {})",
                waylandOutputParams.x, xExpected,
                waylandOutputParams.y, yExpected,
                waylandOutputParams.physicalWidth, physicalWidthExpected,
                waylandOutputParams.physicalHeight, physicalHeightExpected,
                waylandOutputParams.subpixel, subpixelExpected,
                waylandOutputParams.transform, transformExpected
            );

            return false;
        }

        return true;
    }

    bool WaylandOutputTests::CheckWaylandOutputMode(const WaylandOutputTestParams &waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight)
    {
        const uint32_t flagsExpected = WL_OUTPUT_MODE_CURRENT;
        const int32_t refreshExpected= 0;

        if(waylandOutputParams.modeFlags != flagsExpected
            || waylandOutputParams.width != static_cast<int32_t>(expectedWidth)
            || waylandOutputParams.height != static_cast<int32_t>(expectedHeight)
            || waylandOutputParams.refreshRate != refreshExpected)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: wrong values received"
                ": flags = {} (expected = {})"
                ": width = {} (expected = {})"
                ": height = {} (expected = {})"
                ": refresh = {} (expected = {})",
                waylandOutputParams.modeFlags, flagsExpected,
                waylandOutputParams.width, expectedWidth,
                waylandOutputParams.height, expectedHeight,
                waylandOutputParams.refreshRate, refreshExpected
            );

            return false;
        }

        return true;
    }

    bool WaylandOutputTests::CheckWaylandOutputScale(const WaylandOutputTestParams &waylandOutputParams)
    {
        const int32_t factorExpected = 1;

        if(waylandOutputParams.factor != factorExpected)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputTests::CheckWaylandOutputParams: wrong values received"
                ": factor = {} (expected = {})", waylandOutputParams.factor, factorExpected
            );

            return false;
        }

        return true;
    }
}
