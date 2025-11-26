/**
 * @file HollowsPlayMode.hpp
 * @author Adi (100%)
 * @brief Encapsulates Hollows play mode logic for the editor
 *
 * This class handles all Hollows game behavior when the editor
 * enters play mode.
 */

#pragma once

#include <Engine.hpp>
#include "../Sandbox/Hollows/PlayerController.hpp"

class HollowsPlayMode {
public:
    HollowsPlayMode() = default;
    ~HollowsPlayMode() = default;

    /**
     * @brief Start play mode - initialize Hollows game systems
     * @param registry ECS registry containing the scene
     * @param camera Editor camera to control during play
     * @param sceneWidth Width of the scene viewport
     * @param sceneHeight Height of the scene viewport
     * @return true if play mode started successfully, false otherwise
     */
    bool Start(GP2Engine::Registry& registry, GP2Engine::Camera& camera, int sceneWidth, int sceneHeight);

    /**
     * @brief Update play mode - run Hollows game logic each frame
     * @param registry ECS registry
     * @param deltaTime Time since last frame
     * @param camera Editor camera
     * @param sceneWidth Width of the scene viewport
     * @param sceneHeight Height of the scene viewport
     */
    void Update(GP2Engine::Registry& registry, float deltaTime, GP2Engine::Camera& camera, int sceneWidth, int sceneHeight);

    /**
     * @brief Stop play mode - cleanup Hollows game systems
     */
    void Stop();

private:
    // Player controller for handling input and character movement
    Hollows::PlayerController m_playerController;

    // AI system for enemy behavior and pathfinding (ECS-based)
    GP2Engine::AISystem m_aiSystem;

    // Timestamp for frame time tracking
    float m_lastFrameTime = 0.0f;
};
