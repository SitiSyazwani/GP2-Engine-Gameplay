/**
 * @file PlayerController.hpp
 * @author Syazwani (100%)
 * @brief Interface for the player movement and collision control system
 *
 * Defines the PlayerController class, which manages player input, calculates
 * movement vectors, and handles entity-to-world collision detection and resolution.
 */

#pragma once

#include <Engine.hpp>
#include "./Audio/AudioComponent.hpp"

namespace Hollows {

    /**
     * @brief Controls player movement with collision detection
     *
     * Features:
     * - WASD movement with speed control
     * - Sprint mode with Left Shift
     * - Entity collision detection and resolution
     * - Sliding collision (separate horizontal/vertical movement)
     * - Velocity tracking for debug visualization
     */
    class PlayerController {
    public:
        PlayerController() = default;

        /**
         * @brief Initialize player controller
         * @param playerEntity Entity ID of the player
         */
        void Initialize(GP2Engine::EntityID playerEntity);

        // NEW: Initialize audio components (call this in Initialize())
        void InitializeAudioComponents();

        // NEW: Generate JSON files for rubric (call once to create files)
        void SavePlayerAudioToJSON();

        /**
         * @brief Update player movement and collision
         * @param registry ECS registry
         * @param deltaTime Frame time in seconds
         */
        void Update(GP2Engine::Registry& registry, float deltaTime);

        /**
         * @brief Get current player velocity (for debug visualization)
         */
        const GP2Engine::Vector2D& GetVelocity() const { return m_playerVelocity; }

        /**
         * @brief Get player entity ID
         */
        GP2Engine::EntityID GetPlayerEntity() const { return m_playerEntity; }

        /**
         * @brief Set player movement speed
         */
        void SetSpeed(float speed) { m_playerSpeed = speed; }

        /**
         * @brief Get player movement speed
         */
        float GetSpeed() const { return m_playerSpeed; }

        /**
         * @brief Set sprite sheet textures for horizontal and vertical movement
         * @param horizontalTexture Texture for left/right movement
         * @param verticalTexture Texture for down movement
         * @param walkUpTexture Texture for up movement
         * @param idleTexture Texture for idle animation
         */
        void SetTextures(GP2Engine::TexturePtr horizontalTexture, GP2Engine::TexturePtr verticalTexture,
                        GP2Engine::TexturePtr walkUpTexture, GP2Engine::TexturePtr idleTexture) {
            m_horizontalTexture = horizontalTexture;
            m_verticalTexture = verticalTexture;
            m_walkUpTexture = walkUpTexture;
            m_idleTexture = idleTexture;
        }

    private:
        GP2Engine::EntityID m_playerEntity = GP2Engine::INVALID_ENTITY;
        GP2Engine::EntityCollisionSystem m_collisionSystem;
        float m_playerSpeed = 300.0f;
        GP2Engine::Vector2D m_playerVelocity{0.0f, 0.0f};
        bool m_spacebarWasPressed = false;
        bool m_leftMouseWasPressed = false;  // NEW
        bool m_hasPlayedCollisionSound = false;  // NEW for collision cooldown
        float m_collisionSoundCooldown = 0.0f;  // NEW

        // Animation tracking
        std::string m_currentAnimation = "idle";
        bool m_facingRight = true;

        // Sprite sheet textures
        GP2Engine::TexturePtr m_horizontalTexture = nullptr;
        GP2Engine::TexturePtr m_verticalTexture = nullptr;
        GP2Engine::TexturePtr m_walkUpTexture = nullptr;
        GP2Engine::TexturePtr m_idleTexture = nullptr;

        AudioComponent m_collisionAudioComp;
        AudioComponent m_keyboardAudioComp;
        AudioComponent m_mouseClickAudioComp;
        bool m_audioComponentsInitialized = false;

        /**
         * @brief Check if player would collide at position
         */
        bool WouldCollide(GP2Engine::Registry& registry, const GP2Engine::Vector2D& position);

        /**
         * @brief Update sprite animation based on movement
         */
        void UpdateAnimation(GP2Engine::Registry& registry, const GP2Engine::Vector2D& movement);

        /**
         * @brief Pause the current animation at its current frame
         */
        void PauseAnimation(GP2Engine::Registry& registry);
    };

} // namespace Hollows
