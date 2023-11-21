//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/DataTypesImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneUtils/ISceneDataArrayAccessor.h"
#include "internal/PlatformAbstraction/VariantWrapper.h"

namespace ramses::internal
{
    class IScene;

    using DataInstanceValueVariant = std::variant<
        std::monostate,
        float,
        bool,
        int32_t,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::ivec2,
        glm::ivec3,
        glm::ivec4,
        glm::mat2,
        glm::mat3,
        glm::mat4>;

    class DataInstanceHelper
    {
    public:
        static void GetInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, DataInstanceValueVariant& value);
        static void SetInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const DataInstanceValueVariant& value);

        template <typename T>
        static const T& GetReferencedData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField = DataFieldHandle{ 0u });
    };

    template <typename T>
    const T& DataInstanceHelper::GetReferencedData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField)
    {
        const auto dataRefHandle = scene.getDataReference(dataInstance, dataField);
        const T* dataArray = ISceneDataArrayAccessor::GetDataArray<T>(&scene, dataRefHandle, DataFieldHandle{ 0u });
        return *dataArray;
    }
}
