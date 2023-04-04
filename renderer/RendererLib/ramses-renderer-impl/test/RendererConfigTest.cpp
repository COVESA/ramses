//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/BinaryShaderCache.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "RendererConfigImpl.h"

TEST(ARendererConfig, hasDefaultValuesUponConstruction)
{
    const ramses_internal::RendererConfig defaultConfig;
    ramses::RendererConfig config;
    EXPECT_EQ(nullptr, config.impl.getBinaryShaderCache());

    const ramses_internal::RendererConfig& internalConfig = config.impl.getInternalRendererConfig();

    EXPECT_EQ(defaultConfig.getFrameCallbackMaxPollTime(), internalConfig.getFrameCallbackMaxPollTime());
    EXPECT_EQ(defaultConfig.getRenderThreadLoopTimingReportingPeriod(), internalConfig.getRenderThreadLoopTimingReportingPeriod());
    EXPECT_EQ(defaultConfig.getSystemCompositorControlEnabled(), internalConfig.getSystemCompositorControlEnabled());
}

TEST(ARendererConfig, canEnableSystemCompositor)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.enableSystemCompositorControl());
    EXPECT_TRUE(config.impl.getInternalRendererConfig().getSystemCompositorControlEnabled());
}

TEST(ARendererConfig, canBeCopyConstructed)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setFrameCallbackMaxPollTime(333u));
    ramses::RendererConfig configOther(config);
    EXPECT_EQ(333u, configOther.impl.getInternalRendererConfig().getFrameCallbackMaxPollTime().count());
}

TEST(ARendererConfig, canSetBinaryShaderCache)
{
    ramses::RendererConfig config;

    ramses::BinaryShaderCache cache;
    config.setBinaryShaderCache(cache);
    EXPECT_EQ(&cache, config.impl.getBinaryShaderCache());
}

TEST(ARendererConfig, defaultRendererConfigValidates)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST(ARendererConfig, setsAndGetsWaylandDisplay)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setSystemCompositorWaylandDisplay("xxx"));
    EXPECT_EQ("xxx", config.getSystemCompositorWaylandDisplay());
    EXPECT_EQ("xxx", config.impl.getInternalRendererConfig().getWaylandDisplayForSystemCompositorController());
}

TEST(ARendererConfig, setsAndGetsLoopCountPeriod)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds(1234)));
    EXPECT_EQ(std::chrono::milliseconds(1234), config.getRenderThreadLoopTimingReportingPeriod());
}

