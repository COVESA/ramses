//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEPERSISTATIONTEST_H
#define RAMSES_SCENEPERSISTATIONTEST_H

#include "gtest/gtest.h"
#include "ramses-utils.h"
#include "ClientTestUtils.h"
#include "SceneImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "RamsesClientImpl.h"
#include "Utils/File.h"

namespace ramses
{
    class ASceneAndAnimationSystemLoadedFromFile : public LocalTestClientWithSceneAndAnimationSystem, public ::testing::Test
    {
    public:
        explicit ASceneAndAnimationSystemLoadedFromFile(uint32_t animationSystemCreationFlags = EAnimationSystemFlags_Default)
            : LocalTestClientWithSceneAndAnimationSystem(animationSystemCreationFlags)
            , m_clientForLoading(*m_frameworkForLoader.createClient("client"))
            , m_sceneLoaded(nullptr)
            , m_animationSystemLoaded(nullptr)
        {
            m_frameworkForLoader.impl.getScenegraphComponent().setSceneRendererHandler(&sceneActionsCollector);
        }

        ~ASceneAndAnimationSystemLoadedFromFile()
        {
        }

        using ObjectTypeHistogram = ramses_internal::HashMap< ERamsesObjectType, uint32_t >;


        void fillObjectTypeHistorgramFromObjectVector(ObjectTypeHistogram& counter, const RamsesObjectVector& objects)
        {
            for (const auto obj : objects)
            {
                ERamsesObjectType type = obj->getType();
                uint32_t* pCount = counter.get(type);
                if( pCount )
                {
                    ++(*pCount);
                }
                else
                {
                    counter.put(type, 1u);
                }
            }
        }


        void fillObjectTypeHistogramFromScene( ObjectTypeHistogram& counter, const ramses::Scene& scene )
        {
            RamsesObjectVector objects;
            scene.impl.getObjectRegistry().getObjectsOfType(objects, ERamsesObjectType_SceneObject);
            fillObjectTypeHistorgramFromObjectVector( counter, objects );
        }

        ::testing::AssertionResult AssertHistogramEqual( const char* m_expr, const char* n_expr, const ObjectTypeHistogram& m, const ObjectTypeHistogram& n )
        {
            bool isEqual = true;
            ::testing::AssertionResult wrongHistogramCount = ::testing::AssertionFailure();
            wrongHistogramCount << "Histogram differs\n";

            if( m.size() != n.size() )
            {
                wrongHistogramCount << m_expr << " and " << n_expr
                                    << " have different sizes " << m.size() << " and " << n.size() << "\n";

                wrongHistogramCount << m_expr << ":\n";
                for(ObjectTypeHistogram::ConstIterator iter = m.begin(); iter != m.end(); ++iter)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter->key ) << ":\t"
                                            << iter->value << "\n";
                }

                wrongHistogramCount << n_expr << ":\n";
                for(ObjectTypeHistogram::ConstIterator iter = n.begin(); iter != n.end(); ++iter)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter->key ) << ":\t"
                                            << iter->value << "\n";
                }

                isEqual = false;
            }
            else
            {
                for(ObjectTypeHistogram::ConstIterator iter = m.begin(); iter != m.end(); ++iter)
                {
                    if(*m.get(iter->key) != *n.get(iter->key))
                    {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(iter->key) << ":\t"
                                            << m_expr << " = " << *m.get(iter->key) << " while "
                                            << n_expr << " = " << *n.get(iter->key) << "\n";
                        isEqual = false;
                    }
                }
            }

            if( isEqual )
            {
                return ::testing::AssertionSuccess();
            }
            else
            {
                return wrongHistogramCount;
            }
        }

        void checkSceneFile(const char* filename)
        {
            std::vector<ramses_internal::Byte> buffer;
            EXPECT_EQ(StatusOK, m_scene.impl.serialize(buffer, false));
            EXPECT_FALSE(buffer.empty());

            ramses_internal::File f(filename);
            EXPECT_TRUE(f.open(ramses_internal::File::Mode::ReadOnlyBinary));
            size_t numBytes = 0u;
            EXPECT_TRUE(f.getSizeInBytes(numBytes));
            EXPECT_EQ(numBytes, buffer.size());
            std::vector<ramses_internal::Byte> fileBuffer;
            fileBuffer.resize(numBytes);
            EXPECT_EQ(ramses_internal::EStatus::Ok, f.read(fileBuffer.data(), numBytes, numBytes));
            EXPECT_TRUE(f.close());
            EXPECT_EQ(buffer, fileBuffer);
        }

        void doWriteReadCycle(bool expectSameSceneSizeInfo = true, bool expectSameTypeHistogram = true)
        {
            const status_t status = m_scene.saveToFile("someTemporaryFile.ram", false);
            EXPECT_EQ(StatusOK, status);

            checkSceneFile("someTemporaryFile.ram");

            m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram");
            ASSERT_TRUE(nullptr != m_sceneLoaded);

            if (expectSameTypeHistogram)
            {
                ObjectTypeHistogram origSceneNumbers;
                ObjectTypeHistogram loadedSceneNumbers;
                fillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
                fillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
                EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);
            }

            // this can not be enabled for text serialization, because those are destroying/creating meshes
            if (expectSameSceneSizeInfo)
            {
                ramses_internal::SceneSizeInformation origSceneSizeInfo = m_scene.impl.getIScene().getSceneSizeInformation();
                ramses_internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->impl.getIScene().getSceneSizeInformation();

                EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);
            }

            m_animationSystemLoaded = RamsesUtils::TryConvert<AnimationSystem>(*m_sceneLoaded->findObjectByName("animation system"));
            ASSERT_TRUE(nullptr != m_animationSystemLoaded);
        }

        template<typename T>
        T* getObjectForTesting(const char* name)
        {
            RamsesObject* objectPerName = this->m_sceneLoaded->findObjectByName(name);
            EXPECT_TRUE(objectPerName != nullptr);
            if (!objectPerName)
                return nullptr;
            EXPECT_STREQ(name, objectPerName->getName());

            T* specificObject = RamsesUtils::TryConvert<T>(*objectPerName);
            EXPECT_TRUE(nullptr != specificObject);
            return specificObject;
        }

        template<typename T>
        T* getAnimationObjectForTesting(const char* name)
        {
            RamsesObject* objectPerName = this->m_animationSystemLoaded->findObjectByName(name);
            EXPECT_TRUE(objectPerName != nullptr);
            if (!objectPerName)
                return nullptr;
            EXPECT_STREQ(name, objectPerName->getName());

            T* specificObject = RamsesUtils::TryConvert<T>(*objectPerName);
            EXPECT_TRUE(nullptr != specificObject);
            return specificObject;
        }

        ramses::RamsesFramework m_frameworkForLoader;
        ramses::RamsesClient& m_clientForLoading;
        ramses::Scene* m_sceneLoaded;
        ramses::AnimationSystem* m_animationSystemLoaded;
    };

    class ASceneAndAnimationSystemLoadedFromFileWithDefaultRenderPass : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        ASceneAndAnimationSystemLoadedFromFileWithDefaultRenderPass()
            : ASceneAndAnimationSystemLoadedFromFile(0)
        {
        }
    };

    template <typename T>
    class ASceneAndAnimationSystemLoadedFromFileTemplated : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        ASceneAndAnimationSystemLoadedFromFileTemplated()
            : ASceneAndAnimationSystemLoadedFromFile(0)
        {
        }
    };

}

#endif
