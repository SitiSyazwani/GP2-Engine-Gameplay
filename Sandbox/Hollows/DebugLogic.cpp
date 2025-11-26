 /**
  * @file DebugLogic.cpp
  * @author Fauzan (100%)
  * @brief Implementation of the physics debug visualization layer
  *
  * Implements functions to render debugging overlays for physics entities,
  * including AABB collision boxes, velocity vectors, acceleration vectors, and entity center points.
  */

#include "DebugLogic.hpp"
#include <iostream>
#include <algorithm>

namespace Hollows {

    void DebugLogic::RenderDebugPhysics(
        GP2Engine::Registry& registry,
        GP2Engine::EntityID playerEntity,
        const GP2Engine::Vector2D& playerVelocity,
        bool showCollisionBoxes,
        bool showVelocityVectors
    ) {
        // Only render if at least one visualization option is enabled
        if (!showCollisionBoxes && !showVelocityVectors) return;

        auto& renderer = GP2Engine::Renderer::GetInstance();
        auto& debugRenderer = renderer.GetDebugRenderer();

        // Debug output
        static int debugCallCount = 0;
        if (debugCallCount < 3) {
            std::cout << "RenderDebugPhysics called! CollisionBoxes=" << showCollisionBoxes
                      << ", VelocityVectors=" << showVelocityVectors
                      << ", Entity count=" << registry.GetEntityCount() << std::endl;
            debugCallCount++;
        }

        int shapesDrawn = 0;

        // Iterate through all entities and draw their debug info
        for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
            GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
            GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(entity);
            GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);
            GP2Engine::PhysicsComponent* physicsComp = registry.GetComponent<GP2Engine::PhysicsComponent>(entity);

            if (!transform || !spriteComp) continue;

            // Skip background entity
            if (tag && tag->name == "Background") continue;

            // 1. Draw AABB Collision Boxes
            if (showCollisionBoxes) {
                GP2Engine::Color boxColor = (tag && tag->name == "Character")
                    ? GP2Engine::Color::GetGreen()
                    : GP2Engine::Color::GetRed();

                // Override collision box size for debug visualization (must match Systems.cpp)
                const float overrideWidth = 0.0f;  // Set to 0 to use actual sprite size
                const float overrideHeight = 0.0f; // Set to 0 to use actual sprite size
                GP2Engine::Vector2D scaledSize(
                    (overrideWidth > 0 ? overrideWidth : spriteComp->size.x) * transform->scale.x,
                    (overrideHeight > 0 ? overrideHeight : spriteComp->size.y) * transform->scale.y
                );

                debugRenderer.DrawRectangle(
                    transform->position,
                    scaledSize,
                    boxColor,
                    false  // Outline only
                );
                shapesDrawn++;

                if (debugCallCount < 3) {
                    std::cout << "  Drawing collision box for entity " << entity
                              << " at (" << transform->position.x << ", " << transform->position.y << ")"
                              << " scaled size (" << scaledSize.x << ", " << scaledSize.y << ")"
                              << " tag=" << (tag ? tag->name : "none") << std::endl;
                }
            }

            // 2. Draw Velocity Vectors
            if (showVelocityVectors) {
                GP2Engine::Vector2D velocity(0.0f, 0.0f);

                // Check if this is the player entity (use tracked velocity)
                if (entity == playerEntity) {
                    velocity = playerVelocity;
                }
                // Check if entity has physics component
                else if (physicsComp && physicsComp->body) {
                    velocity = physicsComp->body->velocity;
                }

                // Draw velocity vector if moving
                if (velocity.length() > 0.1f) {
                    GP2Engine::Vector2D start = transform->position;
                    GP2Engine::Vector2D velocityDir = velocity.normalized();
                    float velocityLength = std::min(velocity.length() * 0.3f, 100.0f); // Scale for visibility
                    GP2Engine::Vector2D end = start + velocityDir * velocityLength;

                    debugRenderer.DrawLine(start, end, GP2Engine::Color::GetYellow(), 2.0f);

                    // Draw arrowhead
                    debugRenderer.DrawCircle(end, 4.0f, GP2Engine::Color::GetYellow(), true);
                }

                // 3. Draw Acceleration Vectors (only for physics bodies)
                if (physicsComp && physicsComp->body && physicsComp->body->acceleration.length() > 0.01f) {
                    GP2Engine::Vector2D start = transform->position;
                    GP2Engine::Vector2D end = start + physicsComp->body->acceleration * 20.0f; // Scale for visibility

                    debugRenderer.DrawLine(start, end, GP2Engine::Color::GetMagenta(), 2.0f);
                }
            }

            // 4. Draw center point
            if (showCollisionBoxes) {
                debugRenderer.DrawPoint(transform->position, GP2Engine::Color::GetWhite(), 4.0f);
            }
        }

        if (debugCallCount < 3) {
            std::cout << "  Total shapes drawn: " << shapesDrawn << std::endl;
        }
    }

} // namespace Hollows
