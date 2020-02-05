//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKACTIONCOLLECTOR_H
#define RAMSES_MOCKACTIONCOLLECTOR_H

#include "gmock/gmock.h"
#include "Components/ISceneGraphConsumerComponent.h"
#include "SceneAPI/SceneId.h"
#include "ServiceHandlerMocks.h"
#include "Scene/SceneActionCollection.h"

namespace ramses
{
    class MockActionCollector : public ramses_internal::SceneRendererServiceHandlerMock
    {
    public:
        MockActionCollector()
            : m_numReceivedActionLists(0)
            , m_sceneGraphConsumer(nullptr)
        {
        }

        void init(ramses_internal::ISceneGraphConsumerComponent& sceneGraphConsumer)
        {
            m_sceneGraphConsumer = &sceneGraphConsumer;
        }

        void resetCollecting()
        {
            m_collectedActions.clear();
            m_numReceivedActionLists = 0u;
        }

        uint32_t getNumberOfActions() const
        {
            return m_collectedActions.numberOfActions();
        }

        ramses_internal::SceneActionCollection getCopyOfCollectedActions()
        {
            return m_collectedActions.copy();
        }

        uint32_t getNumReceivedActionLists() const
        {
            return m_numReceivedActionLists;
        }

    private:
        ramses_internal::SceneActionCollection  m_collectedActions;
        uint32_t                                m_numReceivedActionLists;

        ramses_internal::ISceneGraphConsumerComponent* m_sceneGraphConsumer;

        virtual void handleInitializeScene(const ramses_internal::SceneInfo& sceneInfo, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererServiceHandlerMock::handleInitializeScene(sceneInfo, providerID);
        }

        virtual void handleSceneActionList(const ramses_internal::SceneId& sceneId, ramses_internal::SceneActionCollection&& actions, const uint64_t& counter, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererServiceHandlerMock::handleSceneActionList(sceneId, actions.copy(), counter, providerID);
            m_collectedActions.append(actions);
            ++m_numReceivedActionLists;
        }

        virtual void handleNewScenesAvailable(const ramses_internal::SceneInfoVector& newScenes, const ramses_internal::Guid& providerID, ramses_internal::EScenePublicationMode publicationMode) override
        {
            ramses_internal::SceneRendererServiceHandlerMock::handleNewScenesAvailable(newScenes, providerID, publicationMode);
            if (m_sceneGraphConsumer != nullptr)
            {
                for(const auto& newScene : newScenes)
                {
                    m_sceneGraphConsumer->subscribeScene(providerID, newScene.sceneID);
                }
            }
        }

        virtual void handleScenesBecameUnavailable(const ramses_internal::SceneInfoVector& unavailableScenes, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererServiceHandlerMock::handleScenesBecameUnavailable(unavailableScenes, providerID);
        }
    };
}

#endif
