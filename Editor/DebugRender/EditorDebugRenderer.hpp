/**
 * @file EditorDebugRenderer.hpp
 * @author Adi (100%)
 * @brief Debug visualization for editor viewport
 *
 * Renders collision boxes and entity bounds in the editor viewport
 * to help with scene layout and collision design.
 */

#pragma once

#include <Engine.hpp>

class EditorDebugRenderer {
public:
    EditorDebugRenderer() = default;
    ~EditorDebugRenderer() = default;

    /**
     * @brief Render collision boxes for all entities in the scene
     * @param registry ECS registry containing entities
     */
    void RenderCollisionBoxes(GP2Engine::Registry& registry);

    void GenerateStressTestScene();

private:
    // No state needed, uses engine's DebugRenderer
};
