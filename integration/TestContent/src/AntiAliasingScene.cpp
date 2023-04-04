//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/AntiAliasingScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-framework-api/DataTypes.h"
#include "PlatformAbstraction/PlatformMath.h"
#include <vector>


namespace ramses_internal
{
    AntiAliasingScene::AntiAliasingScene(ramses::Scene& scene, UInt32 /*state*/, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        std::vector<ramses::vec3f> pos;
        std::vector<uint16_t> id;
        std::vector<ramses::vec3f> colors;

        Float zValue = -6.5f;

        pos.push_back(ramses::vec3f{ 0.f, 0.f, zValue });
        colors.push_back(ramses::vec3f{ 1.f, 0.f, 0.f });

        pos.push_back(ramses::vec3f{ 0.f, 0.f, zValue });
        colors.push_back(ramses::vec3f{ 1.f, 1.f, 1.f });

        const UInt32 slices = 28;
        const Float sliceAngle = 2.f * PlatformMath::PI_f / slices;
        for (UInt16 i = 0; i <= slices; i++)
        {
            Float angle = sliceAngle * i;
            Float x = cosf(angle);
            Float y = sinf(angle);

            pos.push_back(ramses::vec3f{ x, y, zValue });
            colors.push_back(ramses::vec3f{ 1.f, 0.f, 0.f });

            pos.push_back(ramses::vec3f{ x, y, zValue });
            colors.push_back(ramses::vec3f{ 1.f, 1.f, 1.f });

            if (i > 0)
            {
                id.push_back(i % 2);
                id.push_back(2 * i + i % 2);
                id.push_back(2 * i + 2 + i % 2);
            }
        }

        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(uint32_t(pos.size()), pos.data());
        const ramses::ArrayResource* indices = m_scene.createArrayResource(uint32_t(id.size()), id.data());
        const ramses::ArrayResource* vertexColors = m_scene.createArrayResource(uint32_t(colors.size()), colors.data());

        ramses::Effect* effectTex = getTestEffect("ramses-test-client-simple-color");
        ramses::Appearance* appearance = m_scene.createAppearance(*effectTex, "disk appearance");

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput colorsInput;
        effectTex->findAttributeInput("a_position", positionsInput);
        effectTex->findAttributeInput("a_color", colorsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effectTex, "disk geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(colorsInput, *vertexColors);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("disk mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
    }
}
