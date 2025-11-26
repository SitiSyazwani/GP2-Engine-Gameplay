/**
 * @file DebugLogic.hpp
 * @author Fauzan(100%)
 * @brief Interface for the physics debug visualization system
 *
 * Defines the DebugLogic class for controlling and rendering physics overlays
 * such as collision boxes and velocity/acceleration vectors.
 */


#pragma once

#include <Engine.hpp>

namespace Hollows {

    /**
     * @brief Handles debug physics visualization
     *
     * Features:
     * - AABB collision box rendering (green for player, red for others)
     * - Velocity vector visualization (yellow arrows)
     * - Acceleration vector visualization (magenta arrows)
     * - Entity center point markers
     *
     * Toggle options controlled externally (F2, F3 keys in main game).
     */
    class DebugLogic {
    public:
        DebugLogic() = default;

        /**
         * @brief Render debug physics visualization
         *
         * @param registry ECS registry
         * @param playerEntity Player entity ID (for special coloring)
         * @param playerVelocity Player velocity (for velocity vector)
         * @param showCollisionBoxes Draw AABB boxes
         * @param showVelocityVectors Draw velocity/acceleration arrows
         */
        void RenderDebugPhysics(
            GP2Engine::Registry& registry,
            GP2Engine::EntityID playerEntity,
            const GP2Engine::Vector2D& playerVelocity,
            bool showCollisionBoxes,
            bool showVelocityVectors
        );

        /**
         * @brief Get/set collision box visibility
         */
        bool GetShowCollisionBoxes() const { return m_showCollisionBoxes; }
        void SetShowCollisionBoxes(bool show) { m_showCollisionBoxes = show; }

        /**
         * @brief Get/set velocity vector visibility
         */
        bool GetShowVelocityVectors() const { return m_showVelocityVectors; }
        void SetShowVelocityVectors(bool show) { m_showVelocityVectors = show; }

    private:
        bool m_showCollisionBoxes = false;
        bool m_showVelocityVectors = false;
    };

} // namespace Hollows
