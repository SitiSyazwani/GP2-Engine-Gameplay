/**
 * @file GameInitializer.cpp
 * @author Adi(100%)
 * @brief Implementation of core game initialization routines
 *
 * Implements static methods for setting up the game environment, including
 * ECS component registration, ImGui initialization, audio asset loading,
 * scene loading, and configuring the player and camera.
 */

#include "GameInitializer.hpp"
#include "PlayerController.hpp"
#include <Graphics/AnimationHelpers.hpp>
#include <Resources/ResourceManager.hpp>
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Hollows {

    bool GameInitializer::Initialize(
        GP2Engine::Registry& registry,
        GLFWwindow* window,
        PlayerController& playerController,
        GP2Engine::Camera& camera,
        float playerSpeed,
        GP2Engine::AISystem& aiSystem
    ) {
        std::cout << "=== Initializing Hollows Game ===" << std::endl;

        // Register ECS components
        RegisterComponents(registry);

        // Initialize ImGui
        InitializeImGui(window);

        // Load audio assets
        LoadAudioAssets();

        // Load scene
        if (!LoadTestScene(registry)) {
            std::cout << "Warning: Scene failed to load" << std::endl;
            return false;
        }

        // Setup player and camera
        SetupPlayer(registry, playerController, playerSpeed);

        // Setup monster (needs player entity)
        GP2Engine::EntityID playerEntity = playerController.GetPlayerEntity();
        SetupMonster(registry, aiSystem, playerEntity);

        SetupCamera(camera);

        std::cout << "Game initialized with " << registry.GetEntityCount() << " entities" << std::endl;
        return true;
    }

    void GameInitializer::RegisterComponents(GP2Engine::Registry& registry) {
        registry.RegisterComponent<GP2Engine::Transform2D>();
        registry.RegisterComponent<GP2Engine::SpriteComponent>();
        registry.RegisterComponent<GP2Engine::PhysicsComponent>();
        registry.RegisterComponent<GP2Engine::AudioComponent>();
        registry.RegisterComponent<GP2Engine::Tag>();
        registry.RegisterComponent<GP2Engine::TileMapComponent>();
        registry.RegisterComponent<GP2Engine::TextComponent>();
        registry.RegisterComponent<GP2Engine::AIComponent>();

        std::cout << "Components registered successfully" << std::endl;
    }

    void GameInitializer::InitializeImGui(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        std::cout << "ImGui debug overlay initialized" << std::endl;
    }

    void GameInitializer::LoadAudioAssets() {
        auto& resMgr = GP2Engine::ResourceManager::GetInstance();
        std::cout << "Loading sound via ResourceManager..." << std::endl;
        resMgr.LoadAudio("audio_files/test.wav", false, false, false);
    }

    bool GameInitializer::LoadTestScene(GP2Engine::Registry& registry) {
        std::cout << "Loading test scene..." << std::endl;

        bool success = GP2Engine::JsonSerializer::LoadScene(registry, "assets/scenes/test1.json");

        if (success) {
            std::cout << "Scene loaded successfully! Entity count: " << registry.GetEntityCount() << std::endl;
        } else {
            std::cout << "Failed to load scene" << std::endl;
        }

        return success;
    }

    void GameInitializer::SetupPlayer(GP2Engine::Registry& registry, PlayerController& playerController, float playerSpeed) {
        // Find player entity by tag
        GP2Engine::EntityID playerEntity = GP2Engine::INVALID_ENTITY;
        for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
            GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);

            if (tag && tag->name == "Character") {
                playerEntity = entity;
                std::cout << "Found player entity: " << entity << std::endl;
                break;
            }
        }

        if (playerEntity == GP2Engine::INVALID_ENTITY) {
            std::cout << "Warning: No player entity found!" << std::endl;
        } else {
            playerController.Initialize(playerEntity);
            playerController.SetSpeed(playerSpeed);

            // Load sprite sheet textures via ResourceManager
            std::cout << "Loading sprite sheet textures..." << std::endl;
            auto& resMgr = GP2Engine::ResourceManager::GetInstance();
            auto horizontalTexture = resMgr.LoadTexture("textures/SS_Walk_Horizontal.png");
            auto verticalTexture = resMgr.LoadTexture("textures/SS_Walk_Vertical.png");
            auto walkUpTexture = resMgr.LoadTexture("textures/SS_Walk_Up_01.png");
            auto idleTexture = resMgr.LoadTexture("textures/SS_Idle.png");

            if (!horizontalTexture || !horizontalTexture->IsValid()) {
                std::cout << "Error: Failed to load horizontal sprite sheet!" << std::endl;
            }
            if (!verticalTexture || !verticalTexture->IsValid()) {
                std::cout << "Error: Failed to load vertical sprite sheet!" << std::endl;
            }
            if (!walkUpTexture || !walkUpTexture->IsValid()) {
                std::cout << "Error: Failed to load walk up sprite sheet!" << std::endl;
            }
            if (!idleTexture || !idleTexture->IsValid()) {
                std::cout << "Error: Failed to load idle sprite sheet!" << std::endl;
            }

            // Pass textures to player controller
            playerController.SetTextures(horizontalTexture, verticalTexture, walkUpTexture, idleTexture);

            // Setup sprite animations
            GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(playerEntity);
            if (spriteComp && spriteComp->sprite) {
                std::cout << "Setting up player animations..." << std::endl;

                // Set initial texture to idle
                spriteComp->sprite->SetTexture(idleTexture);

                // Setup animations
                SetupWalkRightAnimation(*spriteComp->sprite);
                SetupWalkUpAnimation(*spriteComp->sprite);
                SetupWalkDownAnimation(*spriteComp->sprite);
                SetupIdleAnimation(*spriteComp->sprite);

                // Start with idle animation
                spriteComp->sprite->PlayAnimation("idle");
                std::cout << "Player animations configured successfully" << std::endl;
            } else {
                std::cout << "Warning: Player has no sprite component!" << std::endl;
            }
        }
    }

    void GameInitializer::SetupMonster(GP2Engine::Registry& registry, GP2Engine::AISystem& aiSystem, GP2Engine::EntityID playerEntity) {
        std::cout << "Setting up monster entity with AIComponent..." << std::endl;

        // Find existing monster entity from scene
        GP2Engine::EntityID monsterEntity = GP2Engine::INVALID_ENTITY;
        for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
            GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);
            if (tag && (tag->name == "Monster1" || tag->group == "monsters")) {
                monsterEntity = entity;
                std::cout << "Found existing monster entity from scene: " << entity << std::endl;
                break;
            }
        }

        // If no monster found in scene, create one
        if (monsterEntity == GP2Engine::INVALID_ENTITY) {
            std::cout << "No monster found in scene, creating new monster entity..." << std::endl;
            monsterEntity = registry.CreateEntity();

            // Add Transform component - spawn at a position away from player
            registry.AddComponent(monsterEntity, GP2Engine::Transform2D(GP2Engine::Vector2D(200.0f, 200.0f)));

            // Add Tag component
            registry.AddComponent(monsterEntity, GP2Engine::Tag("Monster1", "monsters"));
        }

        // Load monster sprite sheet textures
        auto& resMgr = GP2Engine::ResourceManager::GetInstance();
        auto monsterHorizontalTexture = resMgr.LoadTexture("textures/SS_Monster_Side_Horizontal.png");
        auto monsterVerticalTexture = resMgr.LoadTexture("textures/SS_Monster_Vertical.png");

        // Validate texture loading
        bool texturesValid = true;
        if (!monsterHorizontalTexture || !monsterHorizontalTexture->IsValid()) {
            std::cerr << "ERROR: Failed to load monster horizontal sprite sheet!" << std::endl;
            texturesValid = false;
        }
        if (!monsterVerticalTexture || !monsterVerticalTexture->IsValid()) {
            std::cerr << "ERROR: Failed to load monster vertical sprite sheet!" << std::endl;
            texturesValid = false;
        }

        if (!texturesValid) {
            std::cerr << "ERROR: Monster textures failed to load. Monster will not be functional!" << std::endl;
            return;
        }

        // Get or create sprite component
        GP2Engine::SpriteComponent* existingSpriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(monsterEntity);

        // Create or get the sprite
        std::shared_ptr<GP2Engine::Sprite> monsterSprite;
        if (existingSpriteComp && existingSpriteComp->sprite) {
            // Use existing sprite from scene
            monsterSprite = existingSpriteComp->sprite;
            std::cout << "Using existing sprite from scene entity" << std::endl;
        } else {
            // Create new sprite
            monsterSprite = std::make_shared<GP2Engine::Sprite>();
            std::cout << "Created new sprite for monster" << std::endl;
        }

        // Set texture and setup animations
        if (monsterVerticalTexture) {
            monsterSprite->SetTexture(monsterVerticalTexture);
        }

        // Setup monster animations
        SetupMonsterWalkLeftAnimation(*monsterSprite);
        SetupMonsterWalkRightAnimation(*monsterSprite);
        SetupMonsterWalkUpAnimation(*monsterSprite);
        SetupMonsterWalkDownAnimation(*monsterSprite);

        // Start with walk down animation
        monsterSprite->PlayAnimation("monster_walk_down");

        // Update or add sprite component
        if (existingSpriteComp) {
            existingSpriteComp->sprite = monsterSprite;
            existingSpriteComp->visible = true;
            existingSpriteComp->renderLayer = 1;
            std::cout << "Updated existing sprite component with animations" << std::endl;
        } else {
            GP2Engine::SpriteComponent spriteComp;
            spriteComp.sprite = monsterSprite;
            spriteComp.size = GP2Engine::Vector2D(128.0f, 128.0f);
            spriteComp.visible = true;
            spriteComp.renderLayer = 1;
            registry.AddComponent(monsterEntity, spriteComp);
            std::cout << "Added new sprite component to monster entity" << std::endl;
        }

        // ===== ECS APPROACH: Add AIComponent to entity =====
        GP2Engine::AIComponent aiComp(playerEntity);
        aiComp.chaseSpeed = 200.0f;
        aiComp.detectionRange = 200.0f;
        aiComp.usePathfinding = true;
        aiComp.horizontalTexture = monsterHorizontalTexture.get();
        aiComp.verticalTexture = monsterVerticalTexture.get();
        aiComp.currentAnimation = "monster_walk_down";

        registry.AddComponent(monsterEntity, aiComp);
        std::cout << "Added AIComponent to monster entity" << std::endl;

        // Setup pathfinding grid in AISystem (shared by all AI entities)
        int gridWidth = 32;
        int gridHeight = 24;
        float tileSize = 64.0f;
        aiSystem.SetupPathfindingGrid(gridWidth, gridHeight, tileSize);

        // Mark all cells as walkable by default
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                aiSystem.SetWalkable(x, y, true);
            }
        }

        std::cout << "Monster entity setup complete with ECS AIComponent!" << std::endl;
    }

    void GameInitializer::SetupCamera(GP2Engine::Camera& camera) {
        float zoomLevel = 3.0f;
        float halfWidth = (1024.0f / zoomLevel) / 2.0f;
        float halfHeight = (768.0f / zoomLevel) / 2.0f;

        camera.SetOrthographic(-halfWidth, halfWidth, halfHeight, -halfHeight);
        camera.SetPosition(GP2Engine::Vector2D(512.0f, 384.0f));

        
    }

} // namespace Hollows
