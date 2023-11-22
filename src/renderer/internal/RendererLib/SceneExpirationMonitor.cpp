//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    SceneExpirationMonitor::SceneExpirationMonitor(const RendererScenes& scenes, RendererEventCollector& eventCollector, RendererStatistics& statistics)
        : m_scenes(scenes)
        , m_eventCollector(eventCollector)
        , m_statistics(statistics)
    {
    }

    SceneExpirationMonitor::~SceneExpirationMonitor()
    {
        assert(m_monitoredScenes.empty());
    }

    void SceneExpirationMonitor::onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp, SceneVersionTag versionTag, uint64_t flushIndex)
    {
        if (expirationTimestamp != FlushTime::InvalidTimestamp)
        {
            if (m_monitoredScenes.count(sceneId) == 0)
            {
                LOG_INFO(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} enabled", sceneId);
                m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringEnabled, sceneId);
            }

            TimeStampTag& ts = m_monitoredScenes[sceneId].expirationTSOfLastAppliedFlush;
            ts.ts = expirationTimestamp;
            ts.tag = versionTag;
            ts.internalIndex = flushIndex;
        }
        else if (m_monitoredScenes.count(sceneId) != 0)
        {
            LOG_INFO(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} disabled, last state expired={}", sceneId, m_monitoredScenes[sceneId].inExpiredState);
            m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringDisabled, sceneId);
            m_monitoredScenes.erase(sceneId);
        }
    }

    void SceneExpirationMonitor::onRendered(SceneId sceneId)
    {
        auto it = m_monitoredScenes.find(sceneId);
        if (it != m_monitoredScenes.end())
            it->second.expirationTSOfRenderedScene = it->second.expirationTSOfLastAppliedFlush;
    }

    void SceneExpirationMonitor::onHidden(SceneId sceneId)
    {
        auto it = m_monitoredScenes.find(sceneId);
        if (it != m_monitoredScenes.end())
            it->second.expirationTSOfRenderedScene = {};
    }

    void SceneExpirationMonitor::checkAndTriggerExpirationEvent(SceneId sceneId, SceneTimestamps& timestamps, bool expired)
    {
        if (expired != timestamps.inExpiredState)
            m_eventCollector.addSceneExpirationEvent(expired ? ERendererEventType::SceneExpired : ERendererEventType::SceneRecoveredFromExpiration, sceneId);

        timestamps.inExpiredState = expired;
    }

    void SceneExpirationMonitor::checkExpiredScenes(FlushTime::Clock::time_point currentTime)
    {
        if (m_monitoredScenes.empty()) // early out if there are no monitored scenes
            return;

        if (currentTime == FlushTime::InvalidTimestamp) // early out if current time is invalid
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Current time is invalid. This is an error. Marking all monitored scenes as expired.");
            for (auto& it : m_monitoredScenes)
                checkAndTriggerExpirationEvent(it.first, it.second, true);

            return;
        }

        for (auto& it : m_monitoredScenes)
        {
            const SceneId sceneId = it.first;
            bool expired = false;
            auto expDelayRendered = FlushTime::Clock::milliseconds::min();
            auto expDelayApplied = FlushTime::Clock::milliseconds::min();
            auto expDelayPending = FlushTime::Clock::milliseconds::min();
            auto& timestamps = it.second;

            if (timestamps.expirationTSOfRenderedScene.ts != FlushTime::InvalidTimestamp)
            {
                expDelayRendered = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - timestamps.expirationTSOfRenderedScene.ts);
                if (expDelayRendered > FlushTime::Clock::milliseconds::zero())
                {
                    const uint64_t expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfRenderedScene.ts.time_since_epoch()).count();
                    LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Content of rendered scene {} is expired (version tag of rendered scene {}). "
                        "Expiration time stamp {} ms, expired by {} ms. "
                        "Internal flush index {}",
                        sceneId, timestamps.expirationTSOfRenderedScene.tag, expirationTimestamp, expDelayRendered.count(), timestamps.expirationTSOfRenderedScene.internalIndex);
                    expired = true;
                }
            }

            if (timestamps.expirationTSOfLastAppliedFlush.ts != FlushTime::InvalidTimestamp)
            {
                expDelayApplied = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - timestamps.expirationTSOfLastAppliedFlush.ts);
                if (expDelayApplied > FlushTime::Clock::milliseconds::zero())
                {
                    const uint64_t expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfLastAppliedFlush.ts.time_since_epoch()).count();
                    LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Flush applied to scene {} is expired (version tag of applied flush {}). "
                        "Expiration time stamp {} ms, expired by {} ms."
                        "Internal flush index {}",
                        sceneId, timestamps.expirationTSOfLastAppliedFlush.tag, expirationTimestamp, expDelayApplied.count(), timestamps.expirationTSOfLastAppliedFlush.internalIndex);
                    expired = true;
                }
            }

            const auto& pendingFlushes = m_scenes.getStagingInfo(sceneId).pendingData.pendingFlushes;
            if (!pendingFlushes.empty()) // early out if no pending flushes
            {
                const auto& lastPendingFlush = pendingFlushes.back();
                const auto& lastPendingTimeInfo = lastPendingFlush.timeInfo;

                if (lastPendingTimeInfo.expirationTimestamp != FlushTime::InvalidTimestamp)
                {
                    expDelayPending = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - lastPendingTimeInfo.expirationTimestamp);
                    const bool lastPendingFlushExpired = expDelayPending > FlushTime::Clock::milliseconds::zero();
                    if (expired || lastPendingFlushExpired) // early out if nothing expired
                    {
                        const uint64_t expirationTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.expirationTimestamp);
                        const uint64_t internalTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.internalTimestamp);

                        if (lastPendingFlushExpired)
                        {
                            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Latest pending flush of scene {} is expired (version tag of this flush {}). "
                                "Expiration time stamp {} ms, "
                                "expired by {} ms. "
                                "Timestamp of flush creation on client side: {} ms. "
                                "Internal flush index {}. "
                                "There is {} pending flushes in total, only latest was checked.",
                                sceneId, lastPendingFlush.versionTag, expirationTimestampPendingFlush, expDelayPending.count(), internalTimestampPendingFlush, lastPendingFlush.flushIndex, pendingFlushes.size());
                            expired = true;
                        }
                        else if (expired)
                        {
                            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: for the expired scene {} there is pending flush which is not expired."
                                "Expiration time stamp {} ms, timestamp of flush creation on client side: {} ms. "
                                "Internal flush index {}. "
                                "There is {} pending flushes in total, only latest was checked.",
                                sceneId, expirationTimestampPendingFlush, internalTimestampPendingFlush, timestamps.expirationTSOfLastAppliedFlush.internalIndex, pendingFlushes.size());
                        }

                        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& logStream)
                            {
                                logStream << "Pending flushes for expired scene " << sceneId << "[internalIndex, expirationTS, versionTag] : ";
                                for (const auto& pendingFlush : pendingFlushes)
                                    logStream << "[" << pendingFlush.flushIndex << ", " << asMilliseconds(pendingFlush.timeInfo.expirationTimestamp) << ", " << pendingFlush.versionTag << "] ";
                            }));
                    }
                }
            }
            else if (expired)
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: there are no pending flushes for the expired scene {}", sceneId);

            checkAndTriggerExpirationEvent(sceneId, timestamps, expired);

            // log the worst case to statistics
            auto maxDelay = std::max(expDelayApplied, std::max(expDelayPending, expDelayRendered));
            m_statistics.addExpirationOffset(sceneId, maxDelay.count());
        }
    }

    void SceneExpirationMonitor::onDestroyed(SceneId sceneId)
    {
        if (m_monitoredScenes.erase(sceneId) != 0)
        {
            LOG_INFO(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} disabled because scene was unsubscribed from renderer", sceneId);
            m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringDisabled, sceneId);
        }
    }

    FlushTime::Clock::time_point SceneExpirationMonitor::getExpirationTimestampOfRenderedScene(SceneId sceneId) const
    {
        const auto it = m_monitoredScenes.find(sceneId);
        return it != m_monitoredScenes.cend() ? it->second.expirationTSOfRenderedScene.ts : FlushTime::InvalidTimestamp;
    }
}
