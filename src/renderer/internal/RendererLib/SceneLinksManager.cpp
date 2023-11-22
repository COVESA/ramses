//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SceneLinksManager.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/DataLinkUtils.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    SceneLinksManager::SceneLinksManager(RendererScenes& rendererScenes, RendererEventCollector& rendererEventCollector)
        : m_rendererScenes(rendererScenes)
        , m_rendererEventCollector(rendererEventCollector)
        , m_transformationLinkManager(rendererScenes)
        , m_dataReferenceLinkManager(rendererScenes)
        , m_textureLinkManager(rendererScenes)
    {
    }

    void SceneLinksManager::createDataLink(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_rendererScenes.hasScene(providerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createDataLink failed: provider scene {} is not known to the renderer!", providerSceneId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        if (!m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createDataLink failed: consumer scene {} is not known to the renderer!", consumerSceneId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        const DataSlotHandle providerSlotHandle = DataLinkUtils::GetDataSlotHandle(providerSceneId, providerId, m_rendererScenes);
        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);

        if (!providerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createDataLink failed: provider data id {} is invalid! (Provider scene: {})", providerId, providerSceneId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createDataLink failed: consumer data id {} is invalid! (Consumer scene: {})", consumerId, consumerSceneId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        // remove any existing data link to establish a new one
        removeAnyDataLinkFromConsumer(consumerSceneId, consumerSlotHandle);

        const EDataSlotType providerSlotType = DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_rendererScenes).type;
        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        bool linkSuccess = false;

        if (providerSlotType == EDataSlotType::TransformationProvider && consumerSlotType == EDataSlotType::TransformationConsumer)
        {
            linkSuccess = m_transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else if (providerSlotType == EDataSlotType::DataProvider && consumerSlotType == EDataSlotType::DataConsumer)
        {
            linkSuccess = m_dataReferenceLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else if (providerSlotType == EDataSlotType::TextureProvider && consumerSlotType == EDataSlotType::TextureConsumer)
        {
            linkSuccess = m_textureLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createDataLink failed: data slot type mismatch, provider and consumer slot types must match! (Provider scene: {}, consumer scene: {})", providerSceneId, consumerSceneId);
        }

        m_rendererEventCollector.addDataLinkEvent((linkSuccess ? ERendererEventType::SceneDataLinked : ERendererEventType::SceneDataLinkFailed), providerSceneId, consumerSceneId, providerId, consumerId);
    }

    void SceneLinksManager::createBufferLink(StreamBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        createBufferLinkInternal(providerBuffer, consumerSceneId, consumerId);
    }

    void SceneLinksManager::createBufferLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        createBufferLinkInternal(providerBuffer, consumerSceneId, consumerId);
    }

    void SceneLinksManager::createBufferLink(ExternalBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        createBufferLinkInternal(providerBuffer, consumerSceneId, consumerId);
    }

    template <typename BUFFERHANDLE>
    void SceneLinksManager::createBufferLinkInternal(BUFFERHANDLE providerBuffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        assert(m_rendererScenes.hasScene(consumerSceneId));

        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);
        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createBufferLink failed: consumer data id is invalid! (consumer scene: {})", consumerSceneId);
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, providerBuffer, consumerSceneId, consumerId);
            return;
        }

        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        if (consumerSlotType != EDataSlotType::TextureConsumer)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::createBufferLink failed: consumer data {} refers to a slot that is not of texture type! (consumer scene: {})", consumerId, consumerSceneId);
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, providerBuffer, consumerSceneId, consumerId);
            return;
        }

        // remove any existing data link to establish a new one
        removeAnyDataLinkFromConsumer(consumerSceneId, consumerSlotHandle);

        m_textureLinkManager.createBufferLink(providerBuffer, consumerSceneId, consumerSlotHandle);
        m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinked, providerBuffer, consumerSceneId, consumerId);
    }

    void SceneLinksManager::removeDataLink(SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::removeDataLink failed: consumer scene {} is not known to the renderer!", consumerSceneId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);

        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::removeDataLink failed: consumer data id {} is invalid!", consumerId);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        bool unlinkSuccess = false;
        SceneId providerSceneId;

        if (consumerSlotType == EDataSlotType::TransformationConsumer)
        {
            unlinkSuccess = m_transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle, &providerSceneId);
        }
        else if (consumerSlotType == EDataSlotType::DataConsumer)
        {
            unlinkSuccess = m_dataReferenceLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle, &providerSceneId);
        }
        else if (consumerSlotType == EDataSlotType::TextureConsumer)
        {
            unlinkSuccess = m_textureLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle, &providerSceneId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneLinksManager::removeDataLink failed: given slot {} is not a consumer slot or there was no link!", consumerSlotHandle);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        m_rendererEventCollector.addDataLinkEvent((unlinkSuccess ? ERendererEventType::SceneDataUnlinked : ERendererEventType::SceneDataUnlinkFailed), providerSceneId, consumerSceneId, DataSlotId(0u), consumerId);
    }

    void SceneLinksManager::handleSceneRemoved(SceneId sceneId)
    {
        m_transformationLinkManager.removeSceneLinks(sceneId);
        m_dataReferenceLinkManager.removeSceneLinks(sceneId);
        m_textureLinkManager.removeSceneLinks(sceneId);
    }

    void SceneLinksManager::handleSceneUnmapped(SceneId sceneId)
    {
        m_textureLinkManager.removeSceneLinks(sceneId);
    }

    void SceneLinksManager::handleDataSlotCreated(SceneId sceneId, DataSlotHandle dataSlotHandle)
    {
        const IScene& scene = m_rendererScenes.getScene(sceneId);
        const DataSlot& dataSlot = scene.getDataSlot(dataSlotHandle);

        switch (dataSlot.type)
        {
        case EDataSlotType::DataProvider:
        case EDataSlotType::TransformationProvider:
        case EDataSlotType::TextureProvider:
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderCreated, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;
        case EDataSlotType::DataConsumer:
        case EDataSlotType::TransformationConsumer:
        case EDataSlotType::TextureConsumer:
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;
        default:
            assert(false);
            break;
        }
    }

    void SceneLinksManager::handleDataSlotDestroyed(SceneId sceneId, DataSlotHandle dataSlotHandle)
    {
        const IScene& scene = m_rendererScenes.getScene(sceneId);
        const DataSlot& dataSlot = scene.getDataSlot(dataSlotHandle);

        switch (dataSlot.type)
        {
        case EDataSlotType::TransformationProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_transformationLinkManager);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType::TransformationConsumer:
            if (m_transformationLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_transformationLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        case EDataSlotType::DataProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_dataReferenceLinkManager);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType::DataConsumer:
            if (m_dataReferenceLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_dataReferenceLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        case EDataSlotType::TextureProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_textureLinkManager);
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType::TextureConsumer:
            if (m_textureLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle) ||
                m_textureLinkManager.getOffscreenBufferLinks().hasLinkedProvider(sceneId, dataSlotHandle) ||
                m_textureLinkManager.getStreamBufferLinks().hasLinkedProvider(sceneId, dataSlotHandle) ||
                m_textureLinkManager.getExternalBufferLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_textureLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        default:
            assert(false);
            break;
        }
    }

    void SceneLinksManager::handleBufferDestroyed(OffscreenBufferHandle providerBuffer)
    {
        OffscreenBufferLinkVector links;
        m_textureLinkManager.getOffscreenBufferLinks().getLinkedConsumers(providerBuffer, links);
        for (const auto& link : links)
        {
            assert(link.providerBuffer == providerBuffer);
            m_textureLinkManager.removeDataLink(link.consumerSceneId, link.consumerSlot);
        }
    }

    void SceneLinksManager::handleBufferDestroyedOrSourceUnavailable(StreamBufferHandle providerBuffer)
    {
        StreamBufferLinkVector links;
        m_textureLinkManager.getStreamBufferLinks().getLinkedConsumers(providerBuffer, links);
        for (const auto& link : links)
        {
            assert(link.providerBuffer == providerBuffer);
            m_textureLinkManager.removeDataLink(link.consumerSceneId, link.consumerSlot);
        }
    }

    void SceneLinksManager::handleBufferDestroyed(ExternalBufferHandle providerBuffer)
    {
        ExternalBufferLinkVector links;
        m_textureLinkManager.getExternalBufferLinks().getLinkedConsumers(providerBuffer, links);
        for (const auto& link : links)
        {
            assert(link.providerBuffer == providerBuffer);
            m_textureLinkManager.removeDataLink(link.consumerSceneId, link.consumerSlot);
        }
    }

    template <typename LINKMANAGER>
    void SceneLinksManager::removeLinksToProvider(SceneId sceneId, DataSlotHandle providerSlotHandle, LINKMANAGER& manager) const
    {
        const SceneLinks& sceneLinks = manager.getSceneLinks();
        SceneLinkVector links;
        sceneLinks.getLinkedConsumers(sceneId, providerSlotHandle, links);

        for (const auto& link : links)
        {
            assert(link.providerSceneId == sceneId);
            assert(link.providerSlot == providerSlotHandle);
            if (manager.removeDataLink(link.consumerSceneId, link.consumerSlot))
            {
                const IScene& consumerScene = m_rendererScenes.getScene(link.consumerSceneId);
                const DataSlotId consumerId = consumerScene.getDataSlot(link.consumerSlot).id;
                m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), link.consumerSceneId, DataSlotId(0u), consumerId);
            }
        }
    }

    void SceneLinksManager::removeAnyDataLinkFromConsumer(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        if (m_transformationLinkManager.getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            m_transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
        else if (m_dataReferenceLinkManager.getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            m_dataReferenceLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
        else if (m_textureLinkManager.getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle) ||
            m_textureLinkManager.getOffscreenBufferLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle) ||
            m_textureLinkManager.getStreamBufferLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            m_textureLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
    }

    const TransformationLinkManager& SceneLinksManager::getTransformationLinkManager() const
    {
        return m_transformationLinkManager;
    }

    const DataReferenceLinkManager& SceneLinksManager::getDataReferenceLinkManager() const
    {
        return m_dataReferenceLinkManager;
    }

    const TextureLinkManager& SceneLinksManager::getTextureLinkManager() const
    {
        return m_textureLinkManager;
    }
}
