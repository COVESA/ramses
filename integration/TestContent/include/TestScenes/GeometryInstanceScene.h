//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GEOMETRYINSTANCESCENE_H
#define RAMSES_GEOMETRYINSTANCESCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"


namespace ramses_internal
{
    class GeometryInstanceScene : public IntegrationScene
    {
    public:
        GeometryInstanceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            GEOMETRY_INSTANCE_UNIFORM = 0,
            GEOMETRY_INSTANCE_VERTEX,
            GEOMETRY_INSTANCE_AND_NOT_INSTANCE
        };

    private:
        void setInstancedUniforms(ramses::Appearance& appearance);
        void setInstancedAttributes(const ramses::Effect& effect, ramses::GeometryBinding* geometry, uint32_t instancingDivisor);
        ramses::GeometryBinding* createGeometry(const ramses::Effect& effect);

        static constexpr uint32_t NumInstances = 3u;
    };
}

#endif
