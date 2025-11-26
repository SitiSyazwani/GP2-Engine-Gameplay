/**
 * @file AnimationHelpers.hpp
 * @brief Helper functions for setting up sprite animations
 * @author Asri (100%)
 *
 * Provides inline utility functions for configuring sprite sheet animations
 * specific to Hollows game assets. Includes setup for player walk/idle animations
 * and monster movement animations using 6-frame, 512x512 sprite sheets.
 */

#pragma once

#include <Engine.hpp>

namespace Hollows {

    /**
     * @brief Setup walk left animation for sprite
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk left animation from a 3072x512 sprite sheet
     * where each frame is 512x512 pixels arranged horizontally.
     */
    inline void SetupWalkLeftAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        // Create animation frames for 6-frame walk cycle
        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        // Add all 6 frames (arranged horizontally left to right)
        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),           // sourcePosition (pixel coordinates)
                glm::vec2(frameWidth, frameHeight),  // sourceSize (pixel coordinates)
                frameDuration                        // duration
            );
        }

        // Add the animation to the sprite with looping enabled
        sprite.AddAnimation("walk_left", frames, true);
    }

    /**
     * @brief Setup walk right animation for sprite
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk right animation from a 3072x512 sprite sheet
     * where each frame is 512x512 pixels arranged horizontally.
     */
    inline void SetupWalkRightAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        // Create animation frames for 6-frame walk cycle
        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        // Add all 6 frames (arranged horizontally left to right)
        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),           // sourcePosition (pixel coordinates)
                glm::vec2(frameWidth, frameHeight),  // sourceSize (pixel coordinates)
                frameDuration                        // duration
            );
        }

        // Add the animation to the sprite with looping enabled
        sprite.AddAnimation("walk_right", frames, true);
    }

    /**
     * @brief Setup walk up animation for sprite
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk up animation from a 3072x512 sprite sheet
     * where each frame is 512x512 pixels arranged horizontally.
     */
    inline void SetupWalkUpAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        // Create animation frames for 6-frame walk cycle
        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        // Add all 6 frames (arranged horizontally left to right)
        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),           // sourcePosition (pixel coordinates)
                glm::vec2(frameWidth, frameHeight),  // sourceSize (pixel coordinates)
                frameDuration                        // duration
            );
        }

        // Add the animation to the sprite with looping enabled
        sprite.AddAnimation("walk_up", frames, true);
    }

    /**
     * @brief Setup walk down animation for sprite
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk down animation from a 3072x512 sprite sheet
     * where each frame is 512x512 pixels arranged horizontally.
     */
    inline void SetupWalkDownAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        // Create animation frames for 6-frame walk cycle
        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        // Add all 6 frames (arranged horizontally left to right)
        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),           // sourcePosition (pixel coordinates)
                glm::vec2(frameWidth, frameHeight),  // sourceSize (pixel coordinates)
                frameDuration                        // duration
            );
        }

        // Add the animation to the sprite with looping enabled
        sprite.AddAnimation("walk_down", frames, true);
    }

    /**
     * @brief Setup idle animation for sprite
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame idle animation from a 3072x512 sprite sheet
     * where each frame is 512x512 pixels arranged horizontally.
     */
    inline void SetupIdleAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        // Create animation frames for 6-frame idle cycle
        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.15f; // Slightly slower than walk for idle

        // Add all 6 frames (arranged horizontally left to right)
        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),           // sourcePosition (pixel coordinates)
                glm::vec2(frameWidth, frameHeight),  // sourceSize (pixel coordinates)
                frameDuration                        // duration
            );
        }

        // Add the animation to the sprite with looping enabled
        sprite.AddAnimation("idle", frames, true);
    }

    /**
     * @brief Setup monster walk left animation for horizontal sprite sheet
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk left animation for monster horizontal sprite.
     */
    inline void SetupMonsterWalkLeftAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),
                glm::vec2(frameWidth, frameHeight),
                frameDuration
            );
        }

        sprite.AddAnimation("monster_walk_left", frames, true);
    }

    /**
     * @brief Setup monster walk right animation for horizontal sprite sheet
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk right animation for monster horizontal sprite.
     */
    inline void SetupMonsterWalkRightAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),
                glm::vec2(frameWidth, frameHeight),
                frameDuration
            );
        }

        sprite.AddAnimation("monster_walk_right", frames, true);
    }

    /**
     * @brief Setup monster walk up animation for vertical sprite sheet
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk up animation for monster vertical sprite.
     */
    inline void SetupMonsterWalkUpAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),
                glm::vec2(frameWidth, frameHeight),
                frameDuration
            );
        }

        sprite.AddAnimation("monster_walk_up", frames, true);
    }

    /**
     * @brief Setup monster walk down animation for vertical sprite sheet
     * @param sprite Sprite to add animation to
     *
     * Configures a 6-frame walk down animation for monster vertical sprite.
     */
    inline void SetupMonsterWalkDownAnimation(GP2Engine::Sprite& sprite) {
        using namespace GP2Engine;

        std::vector<AnimationFrame> frames;

        const float frameWidth = 512.0f;
        const float frameHeight = 512.0f;
        const float frameDuration = 0.1f; // 10 FPS

        for (int i = 0; i < 6; ++i) {
            float frameX = i * frameWidth;
            float frameY = 0.0f;

            frames.emplace_back(
                glm::vec2(frameX, frameY),
                glm::vec2(frameWidth, frameHeight),
                frameDuration
            );
        }

        sprite.AddAnimation("monster_walk_down", frames, true);
    }

} // namespace Hollows
