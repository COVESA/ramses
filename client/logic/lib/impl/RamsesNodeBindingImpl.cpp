//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesNodeBindingImpl.h"

#include "ramses-client-api/Node.h"

#include "ramses-logic/Property.h"

#include "impl/PropertyImpl.h"
#include "impl/LoggerImpl.h"

#include "internals/ErrorReporting.h"
#include "internals/RamsesObjectResolver.h"
#include "internals/RotationUtils.h"

#include "generated/RamsesNodeBindingGen.h"

namespace rlogic::internal
{
    RamsesNodeBindingImpl::RamsesNodeBindingImpl(ramses::Node& ramsesNode, ERotationType rotationType, std::string_view name, uint64_t id, EFeatureLevel featureLevel)
        : RamsesBindingImpl{ name, id }
        , m_ramsesNode{ ramsesNode }
        , m_rotationType{ rotationType }
        , m_featureLevel{ featureLevel }
    {
    }

    void RamsesNodeBindingImpl::createRootProperties()
    {
        // Attention! This order is important - it has to match the indices in ENodePropertyStaticIndex!
        auto inputsType = MakeStruct("", {
                TypeData{"visibility", EPropertyType::Bool},
                TypeData{"rotation", m_rotationType == ERotationType::Quaternion ? EPropertyType::Vec4f : EPropertyType::Vec3f},
                TypeData{"translation", EPropertyType::Vec3f},
                TypeData{"scaling", EPropertyType::Vec3f}
            });
        if (m_featureLevel >= EFeatureLevel_02)
            inputsType.children.push_back({ TypeData{"enabled", EPropertyType::Bool}, {} });
        auto inputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(inputsType, EPropertySemantics::BindingInput));

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesNode, m_featureLevel);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding> RamsesNodeBindingImpl::Serialize(
        const RamsesNodeBindingImpl& nodeBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        auto ramsesReference = RamsesBindingImpl::SerializeRamsesReference(nodeBinding.m_ramsesNode, builder);

        const auto logicObject = LogicObjectImpl::Serialize(nodeBinding, builder);
        const auto propertyObject = PropertyImpl::Serialize(*nodeBinding.getInputs()->m_impl, builder, serializationMap);
        auto ramsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            ramsesReference,
            propertyObject);
        builder.Finish(ramsesBinding);

        auto ramsesNodeBinding = rlogic_serialization::CreateRamsesNodeBinding(builder,
            ramsesBinding,
            static_cast<uint8_t>(nodeBinding.m_rotationType)
        );
        builder.Finish(ramsesNodeBinding);

        return ramsesNodeBinding;
    }

    std::unique_ptr<RamsesNodeBindingImpl> RamsesNodeBindingImpl::Deserialize(
        const rlogic_serialization::RamsesNodeBinding& nodeBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap,
        EFeatureLevel featureLevel)
    {
        if (!nodeBinding.base())
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: missing base class info!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(nodeBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!nodeBinding.base()->rootInput())
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*nodeBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);

        if (!deserializedRootInput)
        {
            return nullptr;
        }

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: root input has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto* boundObject = nodeBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: missing ramses object reference!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId(boundObject->objectId());

        ramses::Node* ramsesNode = ramsesResolver.findRamsesNodeInScene(name, objectId);
        if (!ramsesNode)
        {
            // TODO Violin improve error reporting for this particular error (it's reported in ramsesResolver currently): provide better message and scene/node ids
            return nullptr;
        }

        if (ramsesNode->getType() != static_cast<int>(boundObject->objectType()))
        {
            errorReporting.add("Fatal error during loading of RamsesNodeBinding from serialized data: loaded node type does not match referenced node type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto rotationType (static_cast<ERotationType>(nodeBinding.rotationType()));

        auto binding = std::make_unique<RamsesNodeBindingImpl>(*ramsesNode, rotationType, name, id, featureLevel);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::make_unique<Property>(std::move(deserializedRootInput)));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesNode, featureLevel);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RamsesNodeBindingImpl::update()
    {
        ramses::status_t status = ramses::StatusOK;

        PropertyImpl& visibility = *getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility))->m_impl;
        if (m_featureLevel == EFeatureLevel_01)
        {
            if (visibility.checkForBindingInputNewValueAndReset())
            {
                const auto visibilityMode = (visibility.getValueAs<bool>() ? ramses::EVisibilityMode::Visible : ramses::EVisibilityMode::Invisible);
                status = m_ramsesNode.get().setVisibility(visibilityMode);
                if (status != ramses::StatusOK)
                    return LogicNodeRuntimeError{ m_ramsesNode.get().getStatusMessage(status) };
            }
        }
        else // EFeatureLevel_02 and future levels
        {
            PropertyImpl& enabled = *getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled))->m_impl;

            // Ramses uses 3-state visibility mode, transform the 2 bool properties 'visibility' and 'enabled' into 3-state
            const bool visibilityChanged = visibility.checkForBindingInputNewValueAndReset();
            const bool enabledChanged = enabled.checkForBindingInputNewValueAndReset();
            if (visibilityChanged || enabledChanged)
            {
                ramses::EVisibilityMode visibilityMode = (visibility.getValueAs<bool>() ? ramses::EVisibilityMode::Visible : ramses::EVisibilityMode::Invisible);
                // if 'enabled' false it overrides visibility mode to OFF, otherwise (enabled==true) the mode is determined by 'visibility' only
                if (!enabled.getValueAs<bool>())
                    visibilityMode = ramses::EVisibilityMode::Off;

                status = m_ramsesNode.get().setVisibility(visibilityMode);
                if (status != ramses::StatusOK)
                    return LogicNodeRuntimeError{ m_ramsesNode.get().getStatusMessage(status) };
            }
        }

        PropertyImpl& rotation = *getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->m_impl;
        if (rotation.checkForBindingInputNewValueAndReset())
        {
            if (m_rotationType == ERotationType::Quaternion)
            {
                const auto& value = rotation.getValueAs<vec4f>();
                status = m_ramsesNode.get().setRotation(glm::quat(value[3], value[0], value[1], value[2]));
            }
            else
            {
                const auto& valuesEuler = rotation.getValueAs<vec3f>();
                status = m_ramsesNode.get().setRotation(valuesEuler[0], valuesEuler[1], valuesEuler[2], *RotationUtils::RotationTypeToRamsesRotationConvention(m_rotationType));
            }

            if (status != ramses::StatusOK)
            {
                return LogicNodeRuntimeError{m_ramsesNode.get().getStatusMessage(status)};
            }
        }

        PropertyImpl& translation = *getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation))->m_impl;
        if (translation.checkForBindingInputNewValueAndReset())
        {
            const auto& value = translation.getValueAs<vec3f>();
            status = m_ramsesNode.get().setTranslation(value[0], value[1], value[2]);

            if (status != ramses::StatusOK)
            {
                return LogicNodeRuntimeError{ m_ramsesNode.get().getStatusMessage(status) };
            }
        }

        PropertyImpl& scaling = *getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling))->m_impl;
        if (scaling.checkForBindingInputNewValueAndReset())
        {
            const auto& value = scaling.getValueAs<vec3f>();
            status = m_ramsesNode.get().setScaling(value[0], value[1], value[2]);

            if (status != ramses::StatusOK)
            {
                return LogicNodeRuntimeError{ m_ramsesNode.get().getStatusMessage(status) };
            }
        }

        return std::nullopt;
    }

    ramses::Node& RamsesNodeBindingImpl::getRamsesNode() const
    {
        return m_ramsesNode;
    }

    ERotationType RamsesNodeBindingImpl::getRotationType() const
    {
        return m_rotationType;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at initialization,
    // should not overwrite values unless set() or link explicitly called
    void RamsesNodeBindingImpl::ApplyRamsesValuesToInputProperties(RamsesNodeBindingImpl& binding, ramses::Node& ramsesNode, EFeatureLevel featureLevel)
    {
        // The 3-state ramses visibility mode is transformed into 2 bool properties in node binding - 'visibility' and 'enabled'.
        const ramses::EVisibilityMode visibilityMode = ramsesNode.getVisibility();
        const bool visible = (visibilityMode == ramses::EVisibilityMode::Visible);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility))->m_impl->initializeBindingInputValue(PropertyValue{ visible });

        if (featureLevel >= EFeatureLevel_02)
        {
            const bool enabled = (visibilityMode == ramses::EVisibilityMode::Visible || visibilityMode == ramses::EVisibilityMode::Invisible);
            binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled))->m_impl->initializeBindingInputValue(PropertyValue{ enabled });
        }

        vec3f translationValue;
        ramsesNode.getTranslation(translationValue[0], translationValue[1], translationValue[2]);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation))->m_impl->initializeBindingInputValue(PropertyValue{ translationValue });

        vec3f scalingValue;
        ramsesNode.getScaling(scalingValue[0], scalingValue[1], scalingValue[2]);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling))->m_impl->initializeBindingInputValue(PropertyValue{ scalingValue });

        const ramses::ERotationConvention rotationConvention = ramsesNode.getRotationConvention();
        if (binding.m_rotationType == ERotationType::Quaternion)
        {
            vec4f rotationValue = {0.f, 0.f, 0.f, 1.f};
            if (rotationConvention == ramses::ERotationConvention::Quaternion)
            {
                glm::quat quaternion;
                ramsesNode.getRotation(quaternion);
                rotationValue = {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
            }
            else
            {
                vec3f euler;
                ramsesNode.getRotation(euler[0], euler[1], euler[2]);
                // Allow special case where rotation is not set (i.e. zero) -> mismatching convention is OK in this case
                // Otherwise issue a warning
                if (euler[0] != 0.f || euler[1] != 0.f || euler[2] != 0.f)
                {
                    LOG_WARN("Initial rotation values for RamsesNodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type. Expected Quaternion, got Euler.",
                             binding.getIdentificationString());
                }
            }
            binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->m_impl->initializeBindingInputValue(rotationValue);
        }
        else
        {
            vec3f rotationValue;
            ramsesNode.getRotation(rotationValue[0], rotationValue[1], rotationValue[2]);

            std::optional<ERotationType> convertedType = RotationUtils::RamsesRotationConventionToRotationType(rotationConvention);
            if (!convertedType || binding.m_rotationType != *convertedType)
            {
                // Allow special case where rotation is not set (i.e. zero) -> mismatching convention is OK in this case
                // Otherwise issue a warning
                if (rotationValue[0] != 0.f || rotationValue[1] != 0.f || rotationValue[2] != 0.f)
                {
                    LOG_WARN("Initial rotation values for RamsesNodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type.", binding.getIdentificationString());
                }
            }
            else
            {
                binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->m_impl->initializeBindingInputValue(PropertyValue{ rotationValue });
            }
        }
    }
}
