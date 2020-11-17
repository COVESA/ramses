//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Scene/TransformationCachedScene.h"
#include "Scene/Scene.h"
#include "TestEqualHelper.h"

using namespace testing;

namespace ramses_internal
{
    class ATransformationCachedScene : public testing::Test
    {
    public:
        ATransformationCachedScene()
            : scene(SceneInfo())
        {
            this->nodeWithoutTransform = this->scene.allocateNode();
            this->nodeWithTransform = this->scene.allocateNode();
            this->transform = this->scene.allocateTransform(this->nodeWithTransform);
        }

    protected:
        void expectIdentityMatrices(const NodeHandle nodeToCheck) const
        {
            this->expectCorrectMatrices(nodeToCheck, Matrix44f::Identity, Matrix44f::Identity);
        }

        void expectCorrectMatrices(const NodeHandle nodeToCheck, const Matrix44f& worldMatrix, const Matrix44f& objectMatrix) const
        {
            expectMatrixFloatEqual(worldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, nodeToCheck));
            expectMatrixFloatEqual(objectMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_Object, nodeToCheck));

            // should also work when state is clean (after first update)
            expectMatrixFloatEqual(worldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, nodeToCheck));
            expectMatrixFloatEqual(objectMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_Object, nodeToCheck));
        }

        TransformationCachedScene scene;
        NodeHandle nodeWithTransform;
        NodeHandle nodeWithoutTransform;
        TransformHandle transform;
    };

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForNodesWithoutTransform)
    {
        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForInvalidNodes)
    {
        const NodeHandle invalidNode = NodeHandle::Invalid();
        this->expectIdentityMatrices(invalidNode);
    }

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForNodeWithEmptyTransform)
    {
        this->expectIdentityMatrices(this->nodeWithTransform);
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsScaled)
    {
        this->scene.setScaling(this->transform, Vector3(0.5f));
        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsTranslated)
    {
        this->scene.setTranslation(this->transform, Vector3(0.5f));

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Translation({ 0.5f, 0.5f, 0.5f }), Matrix44f::Translation({ -0.5f, -0.5f, -0.5f }));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsRotated)
    {
        this->scene.setRotation(this->transform, Vector3(0.5f, 0.0f, 0.0f), ERotationConvention::Legacy_ZYX);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::RotationEuler({ 0.5f, 0.f, 0.f }, ERotationConvention::Legacy_ZYX),
            Matrix44f::RotationEuler({ -0.5f, 0.f, 0.f }, ERotationConvention::Legacy_ZYX));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSet)
    {
        const Vector3 rotation{ 5.f, 10.0f, 150.0f };
        this->scene.setRotation(this->transform, rotation, ERotationConvention::ZYX);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::RotationEuler(rotation, ERotationConvention::ZYX),
            Matrix44f::RotationEuler(-1 * rotation, ERotationConvention::XYZ));

        this->scene.setRotation(this->transform, rotation, ERotationConvention::YZX);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::RotationEuler(rotation, ERotationConvention::YZX),
            Matrix44f::RotationEuler(-1 * rotation, ERotationConvention::XZY));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSetDifferentFromParent)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const Vector3 rotationParent{ 60.f, 0.f, 0.f }; //rotate only in X-Axis
        this->scene.setRotation(this->transform, rotationParent, ERotationConvention::YXZ);

        const Vector3 rotationChild{ 0.f, 30.f, 0.f }; //rotate only in Y-Axis
        this->scene.setRotation(childTransform, rotationChild, ERotationConvention::ZYX); //use different convention

        const auto expectedChildRotation = rotationChild + rotationParent; //because both rotate only on 1 axis this is still possible
        const Matrix44f expectedWorldMatrix = Matrix44f::RotationEuler(expectedChildRotation, ERotationConvention::XYZ); //Child rotate around Y, then parent rotate around X
        const Matrix44f expectedObjectMatrix = Matrix44f::RotationEuler(-1 * expectedChildRotation, ERotationConvention::ZYX); //invert angle AND convention
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, rotationParent, ERotationConvention::XYZ);
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterAddingParentWithTransform)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, Vector3(0.5f));

        this->expectIdentityMatrices(this->nodeWithTransform);

        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterChangingTransformOfParent)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, Vector3(0.5f));
        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));

        this->scene.setScaling(parentTransform, Vector3(0.25f));

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.25f), Matrix44f::Scaling(4.f));
    }

    TEST_F(ATransformationCachedScene, ReturnsIdentityMatrixForUntransformedNodeAfterRemovingParentWithTransform)
    {
        this->scene.setScaling(this->transform, Vector3(0.25f));
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectCorrectMatrices(this->nodeWithoutTransform, Matrix44f::Scaling(0.25f), Matrix44f::Scaling(4.f));

        this->scene.removeChildFromNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, PropagatesMatricesToMultipleChildNodes)
    {
        // add parent with transform
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        this->expectIdentityMatrices(childLeft);
        this->expectIdentityMatrices(childRight);

        this->scene.setTranslation(this->transform, Vector3(0.1f, 0.2f, 1));

        const Matrix44f expectedWorldMatrix = Matrix44f::Translation(Vector3(0.1f, 0.2f, 1));
        const Matrix44f expectedObjectMatrix = Matrix44f::Translation(Vector3(-0.1f, -0.2f, -1));
        this->expectCorrectMatrices(childLeft, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childRight, expectedWorldMatrix, expectedObjectMatrix);
    }

    TEST_F(ATransformationCachedScene, PropagatesMatricesToMultipleChildNodeWhenRotationConventionIsSet)
    {
        // add parent with transform
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        this->expectIdentityMatrices(nodeWithoutTransform);
        const Vector3 rotation{ 60.f, 30.f, 30.f };
        this->scene.setRotation(this->transform, rotation, ERotationConvention::ZYX);

        const Matrix44f expectedWorldMatrix = Matrix44f::RotationEuler(rotation, ERotationConvention::ZYX);
        const Matrix44f expectedObjectMatrix = Matrix44f::RotationEuler(-1 * rotation, ERotationConvention::XYZ); //invert input angles AND convention
        this->expectCorrectMatrices(nodeWithTransform, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childLeft, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childRight, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, rotation, ERotationConvention::XYZ);

        const Matrix44f expectedWorldMatrixUpdated = Matrix44f::RotationEuler(rotation, ERotationConvention::XYZ);
        const Matrix44f expectedObjectMatrixUpdated = Matrix44f::RotationEuler(-1 * rotation, ERotationConvention::ZYX); //invert input angles AND convention
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixUpdated, expectedObjectMatrixUpdated);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixUpdated, expectedObjectMatrixUpdated);
    }

    TEST_F(ATransformationCachedScene, confidenceTest_UpdatesMatricesOfMultipleChildNodesWithTransform)
    {
        // create a small transformation graph - parent and 2 children, all transforms
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        TransformHandle childLeftTransform = this->scene.allocateTransform(childLeft);
        TransformHandle childRightTransform = this->scene.allocateTransform(childRight);

        const Vector3 leftTranslation(0.1f, 0.2f, 0.3f);
        const Vector3 rightTranslation(0.4f, 0.5f, 0.6f);
        Vector3 parentTranslation(0.7f, 0.8f, 0.9f);

        this->scene.setTranslation(childLeftTransform, leftTranslation);
        this->scene.setTranslation(childRightTransform, rightTranslation);
        this->scene.setTranslation(this->transform, parentTranslation);

        Matrix44f expectedWorldMatrixLeft = Matrix44f::Translation(parentTranslation + leftTranslation);
        Matrix44f expectedWorldMatrixRight = Matrix44f::Translation(parentTranslation + rightTranslation);
        Matrix44f expectedWorldMatrixParent = Matrix44f::Translation(parentTranslation);

        Matrix44f expectedObjectMatrixLeft = Matrix44f::Translation(-parentTranslation - leftTranslation);
        Matrix44f expectedObjectMatrixRight = Matrix44f::Translation(-parentTranslation - rightTranslation);
        Matrix44f expectedObjectMatrixParent = Matrix44f::Translation(-parentTranslation);

        // expect that transformation matrices of all 3 transforms are correct
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);

        // modify parent translation
        parentTranslation.y = 99.f;
        this->scene.setTranslation(this->transform, parentTranslation);

        expectedWorldMatrixLeft = Matrix44f::Translation(parentTranslation + leftTranslation);
        expectedWorldMatrixRight = Matrix44f::Translation(parentTranslation + rightTranslation);
        expectedWorldMatrixParent = Matrix44f::Translation(parentTranslation);

        expectedObjectMatrixLeft = Matrix44f::Translation(-parentTranslation - leftTranslation);
        expectedObjectMatrixRight = Matrix44f::Translation(-parentTranslation - rightTranslation);
        expectedObjectMatrixParent = Matrix44f::Translation(-parentTranslation);

        // expect that parent translation modification is reflected to all dependent transformation matrices
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);
    }

    TEST_F(ATransformationCachedScene, DoesNotAffectNodeWithoutTransformAfterAddingChildWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithoutTransform, this->nodeWithTransform);
        this->scene.setTranslation(this->transform, Vector3(1, 2, 3));

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, UpdatesNodeMatricesWhenItHasAParentWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        const Vector3 translation(1, 2, 3);
        const Vector3 rotation(4, 5, 6);
        const Vector3 scaling(7, 8, 9);

        this->scene.setRotation(this->transform, rotation, ERotationConvention::Legacy_ZYX);
        this->scene.setTranslation(this->transform, translation);
        this->scene.setScaling(this->transform, scaling);

        const Matrix44f expectedWorldMatrix =
            Matrix44f::Translation(translation) *
            Matrix44f::Scaling(scaling) *
            Matrix44f::RotationEuler(rotation, ERotationConvention::Legacy_ZYX);

        const Matrix44f expectedObjectMatrix =
            Matrix44f::RotationEuler(rotation, ERotationConvention::Legacy_ZYX).transpose() *
            Matrix44f::Scaling(scaling.inverse()) *
            Matrix44f::Translation(-translation);

        this->expectCorrectMatrices(this->nodeWithoutTransform, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.removeChildFromNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, UpdatesMatricesWhenAddingNodeAsChildOfOtherNode)
    {
        const NodeHandle parent = this->scene.allocateNode();
        const NodeHandle child = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        const TransformHandle childTransform = this->scene.allocateTransform(child);

        Vector3 parentTranslation(1, 2, 3);
        Vector3 childTranslation(3, 4, 5);

        this->scene.setTranslation(parentTransform, parentTranslation);
        this->scene.setTranslation(childTransform, childTranslation);

        Matrix44f expectedParentWorldMatrix =
            Matrix44f::Translation(parentTranslation);
        Matrix44f expectedChildWorldMatrix =
            Matrix44f::Translation(childTranslation);

        // force cache update
        expectMatrixFloatEqual(expectedParentWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, parent));
        expectMatrixFloatEqual(expectedChildWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, child));

        this->scene.addChildToNode(parent, child);

        Matrix44f expectedUpdatedChildWorldMatrix =
            Matrix44f::Translation(childTranslation + parentTranslation);
        Matrix44f expectedUpdatedChildObjectMatrix =
            Matrix44f::Translation(-childTranslation - parentTranslation);

        this->expectCorrectMatrices(child, expectedUpdatedChildWorldMatrix, expectedUpdatedChildObjectMatrix);
    }
}
