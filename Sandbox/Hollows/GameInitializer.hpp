/**
 * @file GameInitializer.hpp
 * @author Adi (100%)
 * @brief Interface for the static game initialization utility
 *
 * Defines the GameInitializer class, which contains static methods to orchestrate
 * the game's startup process, ensuring all systems (ECS, rendering, audio, input) are
 * properly configured before the main game loop begins.
 */
#pragma once

#include <Engine.hpp>

namespace Hollows {

    /**
     * @brief Handles game initialization and setup
     *
     * Separates initialization logic from main game loop.
     * Called during OnStart() to set up all game systems.
     */
    class GameInitializer {
    public:
        /**
         * @brief Initialize all game systems
         *
         * @param registry ECS registry
         * @param window GLFW window for ImGui
         * @param playerController Player controller to initialize
         * @param camera Camera to set up
         * @param playerSpeed Initial player speed
         * @param aiSystem AI system for pathfinding
         * @return true if initialization succeeded
         */
        static bool Initialize(
            GP2Engine::Registry& registry,
            GLFWwindow* window,
            class PlayerController& playerController,
            GP2Engine::Camera& camera,
            float playerSpeed,
            GP2Engine::AISystem& aiSystem
        );

    private:
        static void RegisterComponents(GP2Engine::Registry& registry);
        static void InitializeImGui(GLFWwindow* window);
        static void LoadAudioAssets();
        static bool LoadTestScene(GP2Engine::Registry& registry);
        static void SetupPlayer(GP2Engine::Registry& registry, class PlayerController& playerController, float playerSpeed);
        static void SetupMonster(GP2Engine::Registry& registry, GP2Engine::AISystem& aiSystem, GP2Engine::EntityID playerEntity);
        static void SetupCamera(GP2Engine::Camera& camera);
    };

} // namespace Hollows
