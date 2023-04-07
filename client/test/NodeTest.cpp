//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-utils.h"

#include "NodeImpl.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"

using namespace testing;

namespace ramses
{
    template <typename NodeType>
    class NodeTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        NodeType& createNode(const char* name)
        {
            return (this->template createObject<NodeType>(name));
        }
    };

    TYPED_TEST_SUITE(NodeTest, NodeTypes);

    TYPED_TEST(NodeTest, hasChild)
    {
        Node& parent = this->createNode("parent");
        EXPECT_EQ(0u, parent.getChildCount());
        EXPECT_FALSE(parent.hasChild());

        EXPECT_EQ(StatusOK, parent.addChild(this->createNode("child")));
        EXPECT_TRUE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, hasParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_FALSE(child.hasParent());

        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_TRUE(child.hasParent());
    }

    TYPED_TEST(NodeTest, shouldAddChildrenToNodeAndCountThemCorrectly)
    {
        Node& parent = this->createNode("parent");
        EXPECT_EQ(0u, parent.getChildCount());

        for (uint32_t i = 0; i < 10; i++)
        {
            EXPECT_EQ(StatusOK, parent.addChild(this->createNode("child")));
            EXPECT_EQ(i + 1, parent.getChildCount());
        }

        EXPECT_TRUE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldNotAddNodeFromOneSceneAsChildToNodeInOtherScene)
    {
        Scene& otherScene = *this->client.createScene(sceneId_t(1234u));
        CreationHelper otherSceneCreationHelper(&otherScene, &this->client);
        Node& parent = *otherSceneCreationHelper.template createObjectOfType<TypeParam>("parent");
        EXPECT_EQ(0u, parent.getChildCount());

        EXPECT_NE(StatusOK, parent.addChild(this->createNode("child")));
        EXPECT_EQ(0u, parent.getChildCount());

        EXPECT_FALSE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldAddChildToNodeAndGetParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        EXPECT_EQ(&parent, child.getParent());
    }

    TYPED_TEST(NodeTest, constGetChildAndGetParentBehaveAsNonConstVersion)
    {
        Node& parent = this->createNode("parent");
        const Node& constParent = parent;
        Node& child = this->createNode("child");
        const Node& constChild = child;

        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_EQ(constParent.getChild(0), parent.getChild(0));
        EXPECT_EQ(constParent.getParent(), parent.getParent());
        EXPECT_EQ(constChild.getParent(), child.getParent());
    }

    TYPED_TEST(NodeTest, shouldRemoveChildOnDestruction)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        this->m_scene.destroy(child);

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_FALSE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldRemoveParentOnItsDestruction)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        this->m_scene.destroy(parent);

        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_FALSE(child.hasParent());
    }

    TYPED_TEST(NodeTest, shouldRemoveChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        EXPECT_EQ(StatusOK, parent.removeChild(child));

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_EQ(nullptr, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldAddParentToNodeAndGetChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_EQ(&child, parent.getChild(0));
    }

    TYPED_TEST(NodeTest, shouldRemoveParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));

        ASSERT_TRUE(child.getParent() != nullptr);
        ASSERT_TRUE(parent.getChild(0) != nullptr);

        EXPECT_EQ(StatusOK, child.removeParent());

        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_EQ(nullptr, parent.getChild(0));
    }

    TYPED_TEST(NodeTest, confidenceTest_shouldRemoveAllChildren)
    {
        Node& parent = this->createNode("parent");
        Node& child1 = this->createNode("child1");
        Node& child2 = this->createNode("child2");
        Node& child3 = this->createNode("child3");
        EXPECT_EQ(StatusOK, parent.addChild(child1));
        EXPECT_EQ(StatusOK, parent.addChild(child2));
        EXPECT_EQ(StatusOK, parent.addChild(child3));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(parent.getChild(1) != nullptr);
        ASSERT_TRUE(parent.getChild(2) != nullptr);

        EXPECT_EQ(StatusOK, parent.removeAllChildren());

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_EQ(nullptr, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, child1.getParent());
        EXPECT_EQ(nullptr, child2.getParent());
        EXPECT_EQ(nullptr, child3.getParent());
        EXPECT_TRUE(child1.impl.isDirty());
        EXPECT_TRUE(child2.impl.isDirty());
        EXPECT_TRUE(child3.impl.isDirty());
    }

    TYPED_TEST(NodeTest, reportsErrorWhenRemovinNonExistingParent)
    {
        Node& nodeWithNoParent = this->createNode("node");
        EXPECT_NE(StatusOK, nodeWithNoParent.removeParent());
    }

    TYPED_TEST(NodeTest, shouldReplaceParentWithaddChild)
    {
        Node& parent1 = this->createNode("parent1");
        Node& parent2 = this->createNode("parent2");
        Node& child = this->createNode("child");

        EXPECT_EQ(StatusOK, parent1.addChild(child));
        EXPECT_EQ(StatusOK, parent2.addChild(child));
        EXPECT_EQ(0u, parent1.getChildCount());
        EXPECT_EQ(&parent2, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldReplaceParentWithSetParent)
    {
        Node& parent1 = this->createNode("parent1");
        Node& parent2 = this->createNode("parent2");
        Node& child = this->createNode("child");

        EXPECT_EQ(StatusOK, child.setParent(parent1));
        EXPECT_EQ(StatusOK, child.setParent(parent2));
        EXPECT_EQ(0u, parent1.getChildCount());
        EXPECT_EQ(&parent2, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldOrderChildrenAccordingToTheirAddingToTheirParent)
    {
        Node& parent = this->createNode("parent");
        Node& child1 = this->createNode("child1");
        Node& child3 = this->createNode("child3");
        Node& child2 = this->createNode("child2");
        Node& child4 = this->createNode("child4");
        ASSERT_EQ(StatusOK, parent.addChild(child1));
        ASSERT_EQ(StatusOK, parent.addChild(child2));
        ASSERT_EQ(StatusOK, parent.addChild(child3));
        ASSERT_EQ(StatusOK, parent.addChild(child4));

        // children -> 1,2,3,4

        EXPECT_EQ(&child1, parent.getChild(0));
        EXPECT_EQ(&child2, parent.getChild(1));
        EXPECT_EQ(&child3, parent.getChild(2));
        EXPECT_EQ(&child4, parent.getChild(3));
    }

    TYPED_TEST(NodeTest, shouldAdaptChildOrderWhenChildrenAreRemovedFromParent)
    {
        Node& child1 = this->createNode("child1");
        Node& child3 = this->createNode("child3");
        Node& parent = this->createNode("parent");
        Node& child2 = this->createNode("child2");
        Node& child4 = this->createNode("child4");
        ASSERT_EQ(StatusOK, parent.addChild(child1));
        ASSERT_EQ(StatusOK, parent.addChild(child2));
        ASSERT_EQ(StatusOK, parent.addChild(child3));
        ASSERT_EQ(StatusOK, parent.addChild(child4));

        // children -> 1,2,3,4 (tested in other test)

        parent.removeChild(child2);

        // children -> 1,3,4

        EXPECT_EQ(&child1, parent.getChild(0));
        EXPECT_EQ(&child3, parent.getChild(1));
        EXPECT_EQ(&child4, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));

        parent.removeChild(child1);

        // children -> 3,4

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(&child4, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));

        parent.removeChild(child4);

        // children -> 3

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(nullptr, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));
    }

    TYPED_TEST(NodeTest, canBeConvertedToNode)
    {
        RamsesObject& obj = this->createNode("node");
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(obj) != nullptr);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(constObj) != nullptr);
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParentToItself)
    {
        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.setParent(node));
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParaentFromAnotherScene)
    {
        Scene& anotherScene = *this->client.createScene(sceneId_t(12u));

        CreationHelper otherSceneCreationHelper(&anotherScene, &this->client);
        Node& parent = *otherSceneCreationHelper.template createObjectOfType<TypeParam>("parent");

        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.setParent(parent));
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingChildToItself)
    {
        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.addChild(node));
    }

    TYPED_TEST(NodeTest, isNotDirtyInitially)
    {
        Node& node = this->createNode("node");
        EXPECT_FALSE(node.impl.isDirty());
    }

    TYPED_TEST(NodeTest, canBeMarkedDirty)
    {
        Node& node = this->createNode("node");
        node.impl.markDirty();
        EXPECT_TRUE(node.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenSettingParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenAddingToParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenRemovedFromParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        EXPECT_EQ(StatusOK, parent.removeChild(child));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenSettingChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_FALSE(parent.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenParentDestroyed)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        this->m_scene.destroy(parent);
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenRemovingNonExistingParent)
    {
        Node& nodeWithNoParent = this->createNode("node");
        EXPECT_NE(StatusOK, nodeWithNoParent.removeParent());
        EXPECT_FALSE(nodeWithNoParent.impl.isDirty());
    }

    void expectMatricesEqual(const float mat1[16], const float mat2[16])
    {
        for (uint32_t i = 0u; i < 16u; ++i)
        {
            EXPECT_FLOAT_EQ(mat1[i], mat2[i]);
        }
    }

    TYPED_TEST(NodeTest, getsIdentityModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(ramses_internal::Matrix44f::Identity.data, modelMat);
    }

    TYPED_TEST(NodeTest, setsRotationVectorAndConvention)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setRotation(1.f, 2.f, 3.f, ERotationConvention::Euler_XZX));

        const auto transformHandle = node.impl.getTransformHandle();

        float x;
        float y;
        float z;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(x, 1.f);
        EXPECT_EQ(y, 2.f);
        EXPECT_EQ(z, 3.f);
        EXPECT_EQ(ERotationConvention::Euler_XZX, node.getRotationConvention());
        EXPECT_EQ(ramses_internal::Vector3(1.f, 2.f, 3.f), ramses_internal::Vector3(this->m_scene.impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationConvention::Euler_XZX, this->m_scene.impl.getIScene().getRotationConvention(transformHandle));

        EXPECT_EQ(ramses::StatusOK, node.setRotation(11.f, 12.f, 13.f, ERotationConvention::Euler_XYZ));

        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(x, 11.f);
        EXPECT_EQ(y, 12.f);
        EXPECT_EQ(z, 13.f);
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());
        EXPECT_EQ(ramses_internal::Vector3(11.f, 12.f, 13.f), ramses_internal::Vector3(this->m_scene.impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationConvention::Euler_XYZ, this->m_scene.impl.getIScene().getRotationConvention(transformHandle));
    }

    TYPED_TEST(NodeTest, returnsErrorWhenSettingQuaternionConvention)
    {
        Node& node = this->createNode("node");

        EXPECT_NE(StatusOK, node.setRotation(10.f, 20.f, 30.f, ERotationConvention::Quaternion));

        // default value is still set
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());

        glm::quat quaternion;
        EXPECT_EQ(StatusOK, node.getRotation(quaternion));
        EXPECT_EQ(quaternion, glm::identity<glm::quat>());

        float x;
        float y;
        float z;
        EXPECT_EQ(StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(x, 0.f);
        EXPECT_EQ(y, 0.f);
        EXPECT_EQ(z, 0.f);

        const auto transformHandle = node.impl.getTransformHandle();
        EXPECT_FALSE(transformHandle.isValid());
    }

    TYPED_TEST(NodeTest, setsRotationQuaternion)
    {
        Node& node = this->createNode("node");

        const glm::quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));

        const auto transformHandle = node.impl.getTransformHandle();

        glm::quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));
        EXPECT_EQ(qOut.x, q.x);
        EXPECT_EQ(qOut.y, q.y);
        EXPECT_EQ(qOut.z, q.z);
        EXPECT_EQ(qOut.w, q.w);
        EXPECT_EQ(ramses_internal::Vector4(q.x, q.y, q.z, q.w), this->m_scene.impl.getIScene().getRotation(transformHandle));
        EXPECT_EQ(ramses_internal::ERotationConvention::Quaternion, this->m_scene.impl.getIScene().getRotationConvention(transformHandle));
    }

    TYPED_TEST(NodeTest, setsRotationQuaternionThenEuler)
    {
        Node& node = this->createNode("node");

        const glm::quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));
        glm::quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));

        const auto transformHandle = node.impl.getTransformHandle();
        float x;
        float y;
        float z;
        EXPECT_EQ(ramses::StatusOK, node.setRotation(11.f, 12.f, 13.f, ERotationConvention::Euler_XYZ));

        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());
        EXPECT_EQ(x, 11.f);
        EXPECT_EQ(y, 12.f);
        EXPECT_EQ(z, 13.f);
        EXPECT_EQ(ramses_internal::Vector3(11.f, 12.f, 13.f), ramses_internal::Vector3(this->m_scene.impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationConvention::Euler_XYZ, this->m_scene.impl.getIScene().getRotationConvention(transformHandle));
        EXPECT_NE(ramses::StatusOK, node.getRotation(qOut));
    }

    TYPED_TEST(NodeTest, setsRotationEulerThenQuaternion)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setRotation(1.f, 2.f, 3.f, ERotationConvention::Euler_XZX));

        float x;
        float y;
        float z;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(ERotationConvention::Euler_XZX, node.getRotationConvention());

        const glm::quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));

        const auto transformHandle = node.impl.getTransformHandle();

        glm::quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));
        EXPECT_EQ(qOut.x, q.x);
        EXPECT_EQ(qOut.y, q.y);
        EXPECT_EQ(qOut.z, q.z);
        EXPECT_EQ(qOut.w, q.w);
        EXPECT_EQ(ramses_internal::Vector4(q.x, q.y, q.z, q.w), this->m_scene.impl.getIScene().getRotation(transformHandle));
        EXPECT_EQ(ramses_internal::ERotationConvention::Quaternion, this->m_scene.impl.getIScene().getRotationConvention(transformHandle));

        EXPECT_NE(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(ERotationConvention::Quaternion, node.getRotationConvention());
    }

    TYPED_TEST(NodeTest, getRotationReturnsErrorIfQuaternion)
    {
        Node& node = this->createNode("node");
        glm::quat  q{0.5f, 0.5f, 0.5f, -0.5f};
        EXPECT_EQ(StatusOK, node.setRotation(q));
        float x;
        float y;
        float z;
        EXPECT_NE(StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(ERotationConvention::Quaternion, node.getRotationConvention());
    }

    TYPED_TEST(NodeTest, getRotationQuaternionReturnsErrorIfEuler)
    {
        Node& node = this->createNode("node");
        glm::quat  q;
        EXPECT_EQ(StatusOK, node.setRotation(90.f, 0.f, 0.f));
        EXPECT_NE(StatusOK, node.getRotation(q));
    }

    TYPED_TEST(NodeTest, getRotationReturnsDefaultValuesWithoutSetBefore)
    {
        Node& node = this->createNode("node");

        float x;
        float y;
        float z;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(0.f, x);
        EXPECT_EQ(0.f, y);
        EXPECT_EQ(0.f, z);
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());
    }


    TYPED_TEST(NodeTest, getRotationQuaternionReturnsDefaultValuesWithoutSetBefore)
    {
        Node& node = this->createNode("node");
        glm::quat q;
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());
        EXPECT_EQ(ramses::StatusOK, node.getRotation(q));
        EXPECT_EQ(0.f, q.x);
        EXPECT_EQ(0.f, q.y);
        EXPECT_EQ(0.f, q.z);
        EXPECT_EQ(1.f, q.w);
    }

    TYPED_TEST(NodeTest, settingScaleOrTranlationMayNotBreakGetRotation)
    {
        Node& node = this->createNode("node");

        float x;
        float y;
        float z;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));
        EXPECT_EQ(0.f, x);
        EXPECT_EQ(0.f, y);
        EXPECT_EQ(0.f, z);
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());

        EXPECT_EQ(ramses::StatusOK, node.setTranslation(2, 2, 2));
        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));

        EXPECT_EQ(0.f, x);
        EXPECT_EQ(0.f, y);
        EXPECT_EQ(0.f, z);
        EXPECT_EQ(ERotationConvention::Euler_XYZ, node.getRotationConvention());
    }

    TYPED_TEST(NodeTest, canSetAndGetRotationAfterSettingScaleOrTranslation)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setTranslation(2, 2, 2));

        EXPECT_EQ(ramses::StatusOK, node.setRotation(1.f, 2.f, 3.f, ERotationConvention::Euler_ZYZ));

        float x;
        float y;
        float z;

        EXPECT_EQ(ramses::StatusOK, node.getRotation(x, y, z));

        EXPECT_EQ(1.f, x);
        EXPECT_EQ(2.f, y);
        EXPECT_EQ(3.f, z);
        EXPECT_EQ(ERotationConvention::Euler_ZYZ, node.getRotationConvention());
    }

    TYPED_TEST(NodeTest, getsIdentityInverseModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(modelMat));

        expectMatricesEqual(ramses_internal::Matrix44f::Identity.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsModelMatrixComputedFromTransformationChain_SingleNode)
    {
        Node& node = this->template createObject<Node>();

        node.setTranslation(1.f, 2.f, 3.f);
        node.setScaling(4.f, 5.f, 6.f);
        node.setRotation(7.f, 8.f, 9.f, ERotationConvention::Euler_ZYX);

        const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation({ 1.f, 2.f, 3.f });
        const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling({ 4.f, 5.f, 6.f });
        const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationConvention::Euler_ZYX);
        const ramses_internal::Matrix44f expectedModelMat = transMat * rotMat * scaleMat;

        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsModelMatrixComputedFromTransformationChain_MultipleNodes)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation(1.f, 2.f, 3.f);
        scaleNode.setScaling(4.f, 5.f, 6.f);
        rotateNode.setRotation(7.f, 8.f, 9.f, ERotationConvention::Euler_ZYX);

        translationNode.addChild(rotateNode);
        rotateNode.addChild(scaleNode);

        Node& node = this->createNode("node");
        scaleNode.addChild(node);

        const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation({ 1.f, 2.f, 3.f });
        const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling({ 4.f, 5.f, 6.f });
        const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationConvention::Euler_ZYX);
        const ramses_internal::Matrix44f expectedModelMat = transMat * rotMat * scaleMat;

        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsInverseModelMatrixComputedFromTransformationChain_SingleNode)
    {
        Node& node = this->template createObject<Node>();

        node.setTranslation(1.f, 2.f, 3.f);
        node.setScaling(4.f, 5.f, 6.f);
        node.setRotation(7.f, 8.f, 9.f, ERotationConvention::Euler_YZX);

        const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation({ -1.f, -2.f, -3.f });
        const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling({ 4.f, 5.f, 6.f }).inverse();
        const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationConvention::Euler_YZX).transpose();
        const ramses_internal::Matrix44f expectedInverseModelMat = scaleMat * rotMat * transMat;

        float inverseModelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(inverseModelMat));

        expectMatricesEqual(expectedInverseModelMat.data, inverseModelMat);
    }

    TYPED_TEST(NodeTest, getsInverseModelMatrixComputedFromTransformationChain_MultipleNodes)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation(1.f, 2.f, 3.f);
        scaleNode.setScaling(4.f, 5.f, 6.f);
        rotateNode.setRotation(7.f, 8.f, 9.f, ERotationConvention::Euler_YXY);

        translationNode.addChild(rotateNode);
        rotateNode.addChild(scaleNode);

        Node& node = this->createNode("node");
        scaleNode.addChild(node);

        const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation({ -1.f, -2.f, -3.f });
        const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling({ 4.f, 5.f, 6.f }).inverse();
        const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationConvention::Euler_YXY).transpose();
        const ramses_internal::Matrix44f expectedInverseModelMat = scaleMat * rotMat * transMat;

        float inverseModelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(inverseModelMat));

        expectMatricesEqual(expectedInverseModelMat.data, inverseModelMat);
    }

    TYPED_TEST(NodeTest, instantiationDoesNotCreateMultipleLLNodes)
    {
        const Node& node = this->createNode("node");

        // This is to take in account any additional objects(derived from Node) created by helper creation util
        // that the currently tested node depends on.
        // E.g. PickableObject depends on Camera which is a node as well and would be wrongly counted here.
        const size_t additionalAllocatedNodeCount = this->m_creationHelper.getAdditionalAllocatedNodeCount();

        EXPECT_EQ(1u, node.impl.getIScene().getNodeCount() - additionalAllocatedNodeCount);
        EXPECT_TRUE(this->m_internalScene.isNodeAllocated(node.impl.getNodeHandle()));
    }

    TYPED_TEST(NodeTest, destructionDestroysLLNode)
    {
        Node& node = this->createNode("node");
        const auto handle = node.impl.getNodeHandle();
        this->m_scene.destroy(node);
        EXPECT_FALSE(this->m_internalScene.isNodeAllocated(handle));
    }
}
