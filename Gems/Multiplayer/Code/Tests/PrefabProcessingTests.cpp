/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <AzToolsFramework/Prefab/PrefabSystemComponentInterface.h>
#include <AzToolsFramework/Prefab/Spawnable/PrefabCatchmentProcessor.h>
#include <AzToolsFramework/UnitTest/AzToolsFrameworkTestHelpers.h>
#include <Prefab/PrefabDomTypes.h>
#include <Prefab/Spawnable/PrefabProcessorContext.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/MultiplayerConstants.h>
#include <Source/NetworkEntity/NetworkEntityManager.h>
#include <Source/Pipeline/NetworkPrefabProcessor.h>

namespace UnitTest
{
    class TestPrefabProcessorContext
        : public AzToolsFramework::Prefab::PrefabConversionUtils::PrefabProcessorContext
    {
    public:
        AZ_CLASS_ALLOCATOR(TestPrefabProcessorContext, AZ::SystemAllocator, 0);
        AZ_RTTI(TestPrefabProcessorContext, "{2FFFAA06-BA78-4CB3-AE0E-6532822A9B69}",
            AzToolsFramework::Prefab::PrefabConversionUtils::PrefabProcessorContext);

        explicit TestPrefabProcessorContext(const AZ::Uuid& sourceUuid)
            : AzToolsFramework::Prefab::PrefabConversionUtils::PrefabProcessorContext(sourceUuid)
        {
        }

        const AZStd::vector<AzToolsFramework::Prefab::PrefabConversionUtils::EntityAliasStore>& GetEntityAliases() const
        {
            return m_entityAliases;
        }
    };

    class PrefabProcessingTestFixture : public ::testing::Test
    {

    public:
        void SetUp() override
        {
            using AzToolsFramework::Prefab::PrefabConversionUtils::PrefabProcessorContext;

            // Create test entities: 1 networked and 1 static
            AZStd::vector<AZ::Entity*> entities;
            entities.emplace_back(CreateSourceEntity(m_staticEntityName.c_str(), false, AZ::Transform::CreateIdentity()));
            entities.emplace_back(CreateSourceEntity(m_netEntityName.c_str(), true, AZ::Transform::CreateIdentity()));

            // Convert the entities into prefab. Note: This will transfer the ownership of AZ::Entity* into Prefab
            ConvertEntitiesToPrefab(entities, m_prefabDom);
        }

        void TearDown() override
        {
            AZ::Interface<AzToolsFramework::Prefab::PrefabSystemComponentInterface>::Get()->RemoveAllTemplates();
        }

        static void ConvertEntitiesToPrefab(const AZStd::vector<AZ::Entity*>& entities, AzToolsFramework::Prefab::PrefabDom& prefabDom)
        {
            auto* prefabSystem = AZ::Interface<AzToolsFramework::Prefab::PrefabSystemComponentInterface>::Get();
            AZStd::unique_ptr<AzToolsFramework::Prefab::Instance> sourceInstance(prefabSystem->CreatePrefab(entities, {}, "test/path"));
            ASSERT_TRUE(sourceInstance);

            auto& prefabTemplateDom = prefabSystem->FindTemplateDom(sourceInstance->GetTemplateId());
            prefabDom.CopyFrom(prefabTemplateDom, prefabDom.GetAllocator());
        }

        static AZ::Entity* CreateSourceEntity(const char* name, bool networked, const AZ::Transform& tm, AZ::Entity* parent = nullptr)
        {
            AZ::Entity* entity = aznew AZ::Entity(name);
            auto* transformComponent = entity->CreateComponent<AzFramework::TransformComponent>();

            if (parent)
            {
                transformComponent->SetParent(parent->GetId());
                transformComponent->SetLocalTM(tm);
            }
            else
            {
                transformComponent->SetWorldTM(tm);
            }

            if(networked)
            {
                entity->CreateComponent<Multiplayer::NetBindComponent>();
            }

            return entity;
        }

        const AZStd::string m_staticEntityName = "static_floor";
        const AZStd::string m_netEntityName = "networked_entity";

        AzToolsFramework::Prefab::PrefabDom m_prefabDom;
    };

    TEST_F(PrefabProcessingTestFixture, NetworkPrefabProcessor_ProcessPrefabTwoEntities_NetEntityGoesToNetSpawnable)
    {
        // Add the prefab into the Prefab Processor Context
        const AZStd::string prefabName = "testPrefab";
        TestPrefabProcessorContext prefabProcessorContext{AZ::Uuid::CreateRandom()};
        AzToolsFramework::Prefab::PrefabConversionUtils::PrefabDocument document(prefabName);
        ASSERT_TRUE(document.SetPrefabDom(AZStd::move(m_prefabDom)));
        prefabProcessorContext.AddPrefab(AZStd::move(document));

        // Request NetworkPrefabProcessor and PrefabCatchmentProcessor to process the prefab
        Multiplayer::NetworkPrefabProcessor processor;
        processor.Process(prefabProcessorContext);
        AzToolsFramework::Prefab::PrefabConversionUtils::PrefabCatchmentProcessor prefabCatchmentProcessor;
        prefabCatchmentProcessor.Process(prefabProcessorContext);

        // Validate results
        EXPECT_TRUE(prefabProcessorContext.HasCompletedSuccessfully());

        // Should be 1 spawnable and 1 networked spawnable
        const auto& processedObjects = prefabProcessorContext.GetProcessedObjects();
        EXPECT_EQ(processedObjects.size(), 2);

        // Verify the name and the type of the spawnable asset 
        const AZ::Data::AssetData& spawnableAsset = processedObjects[1].GetAsset();
        EXPECT_EQ(prefabName + Multiplayer::NetworkSpawnableFileExtension.data(), processedObjects[1].GetId());
        EXPECT_EQ(spawnableAsset.GetType(), azrtti_typeid<AzFramework::Spawnable>());

        // Verify we have only the networked entity in the network spawnable and not the static one
        const AzFramework::Spawnable* netSpawnable = azrtti_cast<const AzFramework::Spawnable*>(&spawnableAsset);
        const AzFramework::Spawnable::EntityList& entityList = netSpawnable->GetEntities();
        auto countEntityCallback = [](const auto& name)
        {
            return [name](const auto& entity)
            {
                return entity->GetName() == name;
            };
        };

        EXPECT_EQ(0, AZStd::count_if(entityList.begin(), entityList.end(), countEntityCallback(m_staticEntityName)));
        EXPECT_EQ(1, AZStd::count_if(entityList.begin(), entityList.end(), countEntityCallback(m_netEntityName)));
    }

    TEST_F(PrefabProcessingTestFixture, NetworkPrefabProcessor_ProcessPrefabTwoEntities_AliasesInsertedIntoContext)
    {
        using AzToolsFramework::Prefab::PrefabConversionUtils::EntityAliasStore;
        using AzToolsFramework::Prefab::PrefabConversionUtils::PrefabDocument;

        // Add the prefab into the Prefab Processor Context
        TestPrefabProcessorContext prefabProcessorContext{ AZ::Uuid::CreateRandom() };
        PrefabDocument prefabDocument("testPrefab");
        prefabDocument.SetPrefabDom(AZStd::move(m_prefabDom));

        prefabProcessorContext.AddPrefab(AZStd::move(prefabDocument));

        // Request NetworkPrefabProcessor to process the prefab
        Multiplayer::NetworkPrefabProcessor processor;
        processor.Process(prefabProcessorContext);

        const AZStd::vector<EntityAliasStore>& aliases = prefabProcessorContext.GetEntityAliases();

        // Only 1 networked entity, so should be 1 alias inserted.
        EXPECT_EQ(aliases.size(), 1);

        // Verify alias metadata
        const EntityAliasStore& alias = aliases[0];
        EXPECT_EQ(alias.m_aliasType, AzFramework::Spawnable::EntityAliasType::Replace);
        EXPECT_EQ(alias.m_tag, Multiplayer::NetworkEntityManager::NetworkEntityTag);
    }
} // namespace UnitTest
