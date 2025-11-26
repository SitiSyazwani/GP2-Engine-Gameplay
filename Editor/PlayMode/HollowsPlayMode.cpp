/**
 * @file HollowsPlayMode.cpp
 * @author Adi (100%)
 * @brief Implementation of Hollows play mode logic
 *
 * Handles Hollows game initialization in play mode: finds player/monster entities,
 * loads sprite sheets, configures animations, and initializes AI behavior.
 */

#include "HollowsPlayMode.hpp"
#include <Graphics/AnimationHelpers.hpp>

bool HollowsPlayMode::Start(GP2Engine::Registry& registry, GP2Engine::Camera& /*camera*/, int /*sceneWidth*/, int /*sceneHeight*/) {
    std::cout << "=== Starting Hollows Play Mode ===" << std::endl;

    // Find player entity by searching for Tag component with name "Character"
    GP2Engine::EntityID playerEntity = GP2Engine::INVALID_ENTITY;
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        if (auto* tag = registry.GetComponent<GP2Engine::Tag>(entity)) {
            if (tag->name == "Character") {
                playerEntity = entity;
                break;
            }
        }
    }

    if (playerEntity == GP2Engine::INVALID_ENTITY) {
        std::cout << "WARNING: No player entity found (looking for Tag name 'Character')" << std::endl;
        std::cout << "Play mode will run without player controls" << std::endl;
        return true;  // Still allow play mode, just without player
    }

    // Load player sprite sheet textures for different movement directions
    auto& resMgr = GP2Engine::ResourceManager::GetInstance();
    auto horizontalTexture = resMgr.LoadTexture("textures/SS_Walk_Horizontal.png");
    auto verticalTexture = resMgr.LoadTexture("textures/SS_Walk_Vertical.png");
    auto walkUpTexture = resMgr.LoadTexture("textures/SS_Walk_Up_01.png");
    auto idleTexture = resMgr.LoadTexture("textures/SS_Idle.png");

    if (!horizontalTexture || !horizontalTexture->IsValid()) {
        std::cerr << "ERROR: Failed to load SS_Walk_Horizontal.png!" << std::endl;
        return false;
    }
    if (!verticalTexture || !verticalTexture->IsValid()) {
        std::cerr << "ERROR: Failed to load SS_Walk_Vertical.png!" << std::endl;
        return false;
    }
    if (!walkUpTexture || !walkUpTexture->IsValid()) {
        std::cerr << "ERROR: Failed to load SS_Walk_Up_01.png!" << std::endl;
        return false;
    }
    if (!idleTexture || !idleTexture->IsValid()) {
        std::cerr << "ERROR: Failed to load SS_Idle.png!" << std::endl;
        return false;
    }

    // Initialize player controller and assign loaded textures
    m_playerController.Initialize(playerEntity);
    m_playerController.SetTextures(horizontalTexture, verticalTexture, walkUpTexture, idleTexture);

    std::cout << "PlayerController initialized with entity: " << playerEntity << std::endl;

    // Configure player sprite animations using Hollows animation helpers
    GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(playerEntity);
    if (spriteComp && spriteComp->sprite) {
        // Set initial texture to idle state
        spriteComp->sprite->SetTexture(idleTexture);

        // Register all directional walk animations and idle animation
        Hollows::SetupWalkRightAnimation(*spriteComp->sprite);
        Hollows::SetupWalkUpAnimation(*spriteComp->sprite);
        Hollows::SetupWalkDownAnimation(*spriteComp->sprite);
        Hollows::SetupIdleAnimation(*spriteComp->sprite);

        // Start with walk_right animation in paused state
        spriteComp->sprite->PlayAnimation("walk_right");
        spriteComp->sprite->PauseAnimation();

        std::cout << "Player animations configured successfully" << std::endl;
    } else {
        std::cout << "WARNING: Player has no sprite component!" << std::endl;
    }

    // Find monster entity by searching for Tag component containing "Monster"
    GP2Engine::EntityID monsterEntity = GP2Engine::INVALID_ENTITY;
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        if (auto* tag = registry.GetComponent<GP2Engine::Tag>(entity)) {
            if (tag->name.find("Monster") != std::string::npos) {
                monsterEntity = entity;
                break;
            }
        }
    }

    if (monsterEntity != GP2Engine::INVALID_ENTITY && playerEntity != GP2Engine::INVALID_ENTITY) {
        std::cout << "Found monster entity: " << monsterEntity << std::endl;

        // Load monster sprite sheet textures
        auto monsterHorizontalTexture = resMgr.LoadTexture("textures/SS_Monster_Side_Horizontal.png");
        auto monsterVerticalTexture = resMgr.LoadTexture("textures/SS_Monster_Vertical.png");

        // Validate texture loading
        bool monsterTexturesValid = true;
        if (!monsterHorizontalTexture || !monsterHorizontalTexture->IsValid()) {
            std::cerr << "WARNING: Failed to load monster horizontal sprite sheet!" << std::endl;
            monsterTexturesValid = false;
        }
        if (!monsterVerticalTexture || !monsterVerticalTexture->IsValid()) {
            std::cerr << "WARNING: Failed to load monster vertical sprite sheet!" << std::endl;
            monsterTexturesValid = false;
        }

        if (monsterTexturesValid) {
            // Configure monster sprite animations using Hollows animation helpers
            GP2Engine::SpriteComponent* monsterSpriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(monsterEntity);
            if (monsterSpriteComp && monsterSpriteComp->sprite) {
                // Set initial texture to vertical sprite sheet
                monsterSpriteComp->sprite->SetTexture(monsterVerticalTexture);

                // Register all directional walk animations for monster
                Hollows::SetupMonsterWalkLeftAnimation(*monsterSpriteComp->sprite);
                Hollows::SetupMonsterWalkRightAnimation(*monsterSpriteComp->sprite);
                Hollows::SetupMonsterWalkUpAnimation(*monsterSpriteComp->sprite);
                Hollows::SetupMonsterWalkDownAnimation(*monsterSpriteComp->sprite);

                // Start playing walk down animation
                monsterSpriteComp->sprite->PlayAnimation("monster_walk_down");

                std::cout << "Monster animations configured successfully" << std::endl;
            }

            // Add AIComponent to monster entity (ECS approach)
            GP2Engine::AIComponent aiComp(playerEntity);
            aiComp.chaseSpeed = 200.0f;  // Slightly slower than player
            aiComp.detectionRange = 200.0f;
            aiComp.usePathfinding = true;
            aiComp.horizontalTexture = monsterHorizontalTexture.get();
            aiComp.verticalTexture = monsterVerticalTexture.get();
            aiComp.currentAnimation = "monster_walk_down";

            registry.AddComponent(monsterEntity, aiComp);

            // Setup pathfinding grid in AISystem
            int gridWidth = 32;
            int gridHeight = 24;
            float tileSize = 64.0f;
            m_aiSystem.SetupPathfindingGrid(gridWidth, gridHeight, tileSize);

            // Mark all cells as walkable by default
            for (int y = 0; y < gridHeight; ++y) {
                for (int x = 0; x < gridWidth; ++x) {
                    m_aiSystem.SetWalkable(x, y, true);
                }
            }

            std::cout << "AIComponent added to monster entity: " << monsterEntity << std::endl;
            std::cout << "Monster AI ready to chase player!" << std::endl;
        } else {
            std::cerr << "Monster textures failed to load. Monster AI will not be initialized!" << std::endl;
        }
    } else {
        std::cout << "WARNING: No monster entity found in scene" << std::endl;
    }

    m_lastFrameTime = static_cast<float>(glfwGetTime());

    std::cout << "=== Hollows Play Mode Active ===" << std::endl;
    return true;
}

void HollowsPlayMode::Update(GP2Engine::Registry& registry, float deltaTime, GP2Engine::Camera& camera, int sceneWidth, int sceneHeight) {
    // Update player controller (handles input, movement, collision, and animation)
    m_playerController.Update(registry, deltaTime);

    // Update AI system (processes all entities with AIComponent)
    m_aiSystem.Update(registry, deltaTime);

    // Update audio engine (positional audio, sound cleanup, etc.)
    DKAudioEngine::Update();

    // Make camera follow the player entity during play mode
    GP2Engine::EntityID playerEntity = m_playerController.GetPlayerEntity();
    if (playerEntity != GP2Engine::INVALID_ENTITY) {
        auto* transform = registry.GetComponent<GP2Engine::Transform2D>(playerEntity);
        if (transform) {
            // Center camera on player by offsetting by half viewport dimensions
            // (Camera origin is at top-left corner)
            GP2Engine::Vector2D playerPos = transform->position;
            float cameraX = playerPos.x - (sceneWidth / 2.0f);
            float cameraY = playerPos.y - (sceneHeight / 2.0f);
            camera.SetPosition(GP2Engine::Vector2D(cameraX, cameraY));
        }
    }
}

void HollowsPlayMode::Stop() {
    std::cout << "=== Stopping Hollows Play Mode ===" << std::endl;

    // Reset player controller and AI system to clean state
    // This clears entity references and game-specific state
    m_playerController = Hollows::PlayerController();
    m_aiSystem = GP2Engine::AISystem();
}
