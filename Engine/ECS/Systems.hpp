/**
 * @file Systems.hpp
 * @author Adi (100%)
 * @brief ECS Systems for GP2Engine
 *
 * Systems contain game logic that operates on entities with specific components.
 * Uses existing engine subsystems (Graphics, Physics).
 *
 * Current systems:
 * - RenderSystem: Renders all entities with Transform2D + SpriteComponent
 * - EntityCollisionSystem: AABB collision detection for entities
 * - AISystem: A* pathfinding and chase behavior (see AI/AISystem.hpp)
 *
 */

#pragma once

#include "Registry.hpp"
#include "Component.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Graphics/Camera.hpp"
#include "../Physics/PhysicsSystem.hpp"
#include "../AI/AISystem.hpp"

namespace GP2Engine {

    /**
     * @brief System for rendering entities
     *
     * Renders all entities that have Transform2D + SpriteComponent.
     * Uses the Graphics/Renderer subsystem for actual drawing.
     *
     * Features:
     * - Automatic batching for entities tagged "StressTest" (performance optimization)
     * - Supports two render modes: Sprite objects, colored quads
     * - Skips invisible entities (sprite->visible = false)
     *
     * Called once per frame from game's Render() method.
     */
    class RenderSystem {
    public:
        RenderSystem() = default;

        /**
         * @brief Render all visible entities
         *
         * @param registry ECS registry containing entities and components
         * @param camera Camera for view/projection transformation
         */
        void Render(Registry& registry, Camera& camera);
    };

    /**
     * @brief System for entity collision detection
     *
     * Tests if an entity would collide with other entities at a given position.
     * Uses AABB (Axis-Aligned Bounding Box) from Physics/PhysicsSystem.
     *
     * Features:
     * - Tests collision against all active entities
     * - Skips Background and StressTest entities (no collision)
     * - Uses entity scale for accurate collision boxes
     *
     * Typically used for movement validation before updating Transform2D.
     */
    class EntityCollisionSystem {
    public:
        EntityCollisionSystem() = default;

        /**
         * @brief Test if entity would collide at a position
         *
         * @param registry ECS registry
         * @param entity Entity to test
         * @param testPosition Position to test for collision
         * @return true if collision would occur, false otherwise
         */
        bool WouldCollide(Registry& registry, EntityID entity, const Vector2D& testPosition);
    };

    /**
     * @brief System for interactive button logic
     * @author Asri (100%)
     *
     * Processes all entities with ButtonComponent for hover detection and click handling.
     * Any entity can become a button by attaching ButtonComponent - pure ECS approach.
     *
     * Features:
     * - Automatic hitbox calculation from TextComponent (if autoSizeFromText enabled)
     * - Hover state detection with visual feedback (color changes)
     * - Click detection and action triggering
     * - Works with both text and sprite buttons
     *
     * Requires Transform2D component on button entities.
     * Optionally uses TextComponent for auto-sizing and color feedback.
     *
     * Called once per frame from game's Update() method.
     */
    class ButtonSystem {
    public:
        ButtonSystem() = default;

        /**
         * @brief Update all button entities (hover detection, click handling)
         *
         * @param registry ECS registry containing entities and components
         * @param deltaTime Frame time in seconds (unused currently)
         */
        void Update(Registry& registry, float deltaTime);

        /**
         * @brief Check if any button was clicked and get its action
         *
         * @param registry ECS registry
         * @param outAction Output parameter for button action
         * @return EntityID of clicked button, or INVALID_ENTITY if none
         */
        EntityID GetClickedButton(Registry& registry, ButtonComponent::Action& outAction);

    private:
        /**
         * @brief Calculate button hitbox from transform and text (if applicable)
         *
         * @param transform Entity transform
         * @param buttonComp Button component with hitbox configuration
         * @param textComp Optional text component for auto-sizing
         * @return Hitbox bounds {x, y, width, height} in screen coordinates
         */
        glm::vec4 CalculateHitbox(
            const Transform2D* transform,
            const ButtonComponent* buttonComp,
            const TextComponent* textComp
        );

        /**
         * @brief Check if point is inside hitbox
         *
         * @param point Screen position (x, y)
         * @param hitbox Hitbox bounds {x, y, width, height}
         * @return true if point is inside hitbox
         */
        bool IsPointInHitbox(const glm::vec2& point, const glm::vec4& hitbox);
    };

} // namespace GP2Engine


