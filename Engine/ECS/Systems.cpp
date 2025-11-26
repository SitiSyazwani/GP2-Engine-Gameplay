/**
 * @file Systems.cpp
 * @author Adi (100%)
 * @brief ECS Systems implementation
 *
 * Implements the logic for RenderSystem, EntityCollisionSystem, and ButtonSystem.
 * All systems use existing engine subsystems (Graphics/Renderer, Physics/AABB, Core/Input)
 */

#include "Systems.hpp"
#include "../Graphics/Sprite.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/Font.hpp"
#include "../TileMap/TileMap.hpp"
#include "../TileMap/TileRenderer.hpp"
#include "../Core/Input.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <vector>

namespace GP2Engine {

    // Helper structure for sorting entities by render layer
    struct RenderableEntity {
        EntityID entity;
        int renderLayer;
        enum class Type { Sprite, TileMap, Text } type;
    };

    void RenderSystem::Render(Registry& registry, Camera& camera) {
        auto& renderer = Renderer::GetInstance();

        // Prepare for rendering (caller is responsible for clearing)
        renderer.ResetPerformanceCounters();
        renderer.SetCamera(camera);

        // === STEP 1: Collect all renderable entities with their render layers ===
        std::vector<RenderableEntity> renderables;

        for (EntityID entity : registry.GetActiveEntities()) {
            // Check for SpriteComponent
            auto* sprite = registry.GetComponent<SpriteComponent>(entity);
            if (sprite && sprite->visible && registry.GetComponent<Transform2D>(entity)) {
                renderables.push_back({entity, sprite->renderLayer, RenderableEntity::Type::Sprite});
            }

            // Check for TileMapComponent
            auto* tileMap = registry.GetComponent<TileMapComponent>(entity);
            if (tileMap && tileMap->visible && tileMap->tileMap && tileMap->tileRenderer) {
                renderables.push_back({entity, tileMap->renderLayer, RenderableEntity::Type::TileMap});
            }

            // Check for TextComponent
            auto* text = registry.GetComponent<TextComponent>(entity);
            if (text && text->visible && text->font && registry.GetComponent<Transform2D>(entity)) {
                renderables.push_back({entity, text->renderLayer, RenderableEntity::Type::Text});
            }
        }

        // === STEP 2: Sort by render layer (lower values render first = background) ===
        std::sort(renderables.begin(), renderables.end(),
            [](const RenderableEntity& a, const RenderableEntity& b) {
                return a.renderLayer < b.renderLayer;
            });

        // === STEP 3: Render in sorted order ===
        renderer.BeginBatch();

        for (const auto& renderable : renderables) {
            switch (renderable.type) {
                case RenderableEntity::Type::Sprite: {
                    auto* sprite = registry.GetComponent<SpriteComponent>(renderable.entity);
                    auto* transform = registry.GetComponent<Transform2D>(renderable.entity);
                    if (!sprite || !transform) continue;

                    // Check if entity should be batched (stress test entities for performance)
                    auto* tag = registry.GetComponent<Tag>(renderable.entity);
                    bool shouldBatch = (tag && tag->name == "StressTest");

                    // Calculate render data (position, size, UVs)
                    // Use actual sprite size from JSON (no override)
                    const float overrideWidth = 0.0f;  // Set to 0 to use JSON size
                    const float overrideHeight = 0.0f; // Set to 0 to use JSON size
                    glm::vec2 position(transform->position.x, transform->position.y);
                    glm::vec2 size(
                        (overrideWidth > 0 ? overrideWidth : sprite->size.x) * transform->scale.x,
                        (overrideHeight > 0 ? overrideHeight : sprite->size.y) * transform->scale.y
                    );

                    // Use animation UV coordinates if an animation exists (playing or paused), otherwise use component UV
                    glm::vec4 uvCoords;
                    if (sprite->sprite && !sprite->sprite->GetCurrentAnimationName().empty()) {
                        // Animation is active (playing or paused) - use animation frame UVs
                        uvCoords = sprite->sprite->GetSourceRect();
                    } else {
                        // No animation - use static component UV coordinates
                        uvCoords = glm::vec4(sprite->uvOffset.x, sprite->uvOffset.y, sprite->uvSize.x, sprite->uvSize.y);
                    }

                    // Apply flip to UV coordinates if needed
                    if (sprite->sprite) {
                        if (sprite->sprite->GetFlipX()) {
                            // Swap left (u0) and right (u1) texture coordinates
                            float u0 = uvCoords.x;
                            float u1 = uvCoords.x + uvCoords.z;
                            float temp = u0;
                            u0 = u1;
                            u1 = temp;
                            uvCoords.x = u0;
                            uvCoords.z = u1 - u0; // Negative width
                        }
                        if (sprite->sprite->GetFlipY()) {
                            // Swap top (v0) and bottom (v1) texture coordinates
                            float v0 = uvCoords.y;
                            float v1 = uvCoords.y + uvCoords.w;
                            float temp = v0;
                            v0 = v1;
                            v1 = temp;
                            uvCoords.y = v0;
                            uvCoords.w = v1 - v0; // Negative height
                        }
                    }

                    // Render based on component type (textured sprite or colored quad)
                    if (sprite->sprite && sprite->sprite->GetTexture()) {
                        unsigned int texID = sprite->sprite->GetTexture()->GetTextureID();

                        if (shouldBatch) {
                            renderer.DrawTexturedQuadBatch(position, size, transform->rotation, texID, uvCoords, sprite->color);
                        } else {
                            renderer.DrawTexturedQuad(position, size, transform->rotation, texID, uvCoords, sprite->color);
                        }
                    } else {
                        if (shouldBatch) {
                            renderer.DrawQuadBatch(position, size, transform->rotation, sprite->color);
                        } else {
                            renderer.DrawQuad(transform->position, Vector2D(size.x, size.y), transform->rotation, sprite->color);
                        }
                    }
                    break;
                }

                case RenderableEntity::Type::TileMap: {
                    auto* tileMapComp = registry.GetComponent<TileMapComponent>(renderable.entity);
                    if (!tileMapComp) continue;

                    // Flush batch before rendering tilemap (tilemaps use their own rendering)
                    renderer.FlushBatch();

                    // Render the tilemap
                    tileMapComp->tileRenderer->Render(renderer);

                    // Resume batching for subsequent entities
                    break;
                }

                case RenderableEntity::Type::Text: {
                    auto* textComp = registry.GetComponent<TextComponent>(renderable.entity);
                    auto* transform = registry.GetComponent<Transform2D>(renderable.entity);
                    if (!textComp || !transform) continue;

                    // Flush batch before rendering text (text uses separate rendering)
                    renderer.FlushBatch();

                    // Apply offset to transform if needed
                    Transform2D adjustedTransform = *transform;
                    if (textComp->offset.x != 0.0f || textComp->offset.y != 0.0f) {
                        adjustedTransform.position.x += textComp->offset.x;
                        adjustedTransform.position.y += textComp->offset.y;
                    }

                    // Render text
                    renderer.DrawText(textComp->font.get(), textComp->text, &adjustedTransform,
                                     textComp->scale, textComp->color);

                    // Resume batching for subsequent entities
                    break;
                }
            }
        }

        renderer.EndBatch();
    }

    bool EntityCollisionSystem::WouldCollide(Registry& registry, EntityID entity, const Vector2D& testPosition) {
        // Get entity's sprite and transform for collision box calculation
        auto* sprite = registry.GetComponent<SpriteComponent>(entity);
        auto* transform = registry.GetComponent<Transform2D>(entity);
        if (!sprite || !transform) return false;

        // Override collision box size (must match render size above, set to 0 to use JSON size)
        const float overrideWidth = 0.0f;  // Set to 0 to use actual sprite size
        const float overrideHeight = 0.0f; // Set to 0 to use actual sprite size

        // Create AABB at test position (centered on entity)
        float scaledWidth = (overrideWidth > 0 ? overrideWidth : sprite->size.x) * transform->scale.x;
        float scaledHeight = (overrideHeight > 0 ? overrideHeight : sprite->size.y) * transform->scale.y;
        AABB testBox(testPosition.x - scaledWidth * 0.5f, testPosition.y - scaledHeight * 0.5f, scaledWidth, scaledHeight);

        // Test against all other entities
        for (EntityID other : registry.GetActiveEntities()) {
            if (other == entity) continue;

            auto* otherSprite = registry.GetComponent<SpriteComponent>(other);
            auto* otherTransform = registry.GetComponent<Transform2D>(other);
            auto* otherTag = registry.GetComponent<Tag>(other);
            if (!otherSprite || !otherTransform || !otherTag) continue;

            // Skip non-solid entities (background and stress test objects don't collide)
            if (otherTag->name == "Background" || otherTag->name == "StressTest") continue;

            // Create AABB for other entity (uses override sizes from outer scope)
            float otherScaledWidth = (overrideWidth > 0 ? overrideWidth : otherSprite->size.x) * otherTransform->scale.x;
            float otherScaledHeight = (overrideHeight > 0 ? overrideHeight : otherSprite->size.y) * otherTransform->scale.y;
            AABB otherBox(otherTransform->position.x - otherScaledWidth * 0.5f, otherTransform->position.y - otherScaledHeight * 0.5f, otherScaledWidth, otherScaledHeight);

            // Check intersection using AABB from PhysicsSystem
            if (testBox.Intersects(otherBox)) return true;
        }

        return false;
    }

    // ============================================================================
    // ButtonSystem Implementation
    // ============================================================================

    void ButtonSystem::Update(Registry& registry, float /*deltaTime*/) {
        // Get current mouse position
        double mouseX, mouseY;
        GP2Engine::Input::GetMousePosition(mouseX, mouseY);
        glm::vec2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

        // Get current mouse button state
        bool mousePressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);

        // Process all entities with ButtonComponent
        for (EntityID entity : registry.GetActiveEntities()) {
            auto* buttonComp = registry.GetComponent<ButtonComponent>(entity);
            auto* transform = registry.GetComponent<Transform2D>(entity);
            if (!buttonComp || !transform) continue;

            // Get optional TextComponent for auto-sizing and color feedback
            auto* textComp = registry.GetComponent<TextComponent>(entity);

            // Calculate hitbox
            glm::vec4 hitbox = CalculateHitbox(transform, buttonComp, textComp);

            // Check if mouse is hovering over button
            bool isHovered = IsPointInHitbox(mousePos, hitbox);

            // Update hover state
            buttonComp->isHovered = isHovered;

            // Update visual feedback (if TextComponent exists)
            if (textComp) {
                if (buttonComp->isPressed) {
                    textComp->color = buttonComp->clickColor;
                } else if (buttonComp->isHovered) {
                    textComp->color = buttonComp->hoverColor;
                } else {
                    textComp->color = buttonComp->normalColor;
                }
            }

            // Handle click detection
            if (isHovered && mousePressed && !buttonComp->isPressed) {
                // Button pressed this frame
                buttonComp->isPressed = true;
            } else if (!mousePressed && buttonComp->isPressed) {
                // Button released - check if still hovering (valid click)
                if (isHovered) {
                    buttonComp->wasClicked = true;
                }
                buttonComp->isPressed = false;
            }
        }
    }

    EntityID ButtonSystem::GetClickedButton(Registry& registry, ButtonComponent::Action& outAction) {
        // Find first button that was clicked this frame
        for (EntityID entity : registry.GetActiveEntities()) {
            auto* buttonComp = registry.GetComponent<ButtonComponent>(entity);
            if (buttonComp && buttonComp->wasClicked) {
                outAction = buttonComp->action;
                buttonComp->wasClicked = false; // Consume click
                return entity;
            }
        }

        outAction = ButtonComponent::Action::None;
        return INVALID_ENTITY;
    }

    glm::vec4 ButtonSystem::CalculateHitbox(
        const Transform2D* transform,
        const ButtonComponent* buttonComp,
        const TextComponent* textComp
    ) {
        glm::vec2 hitboxSize = buttonComp->hitboxSize;

        // Auto-size from text if enabled and TextComponent exists
        if (buttonComp->autoSizeFromText && textComp && textComp->font) {
            float textWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale);
            float fontSize = static_cast<float>(textComp->font->GetFontSize()) * textComp->scale;

            hitboxSize.x = textWidth + buttonComp->paddingX * 2.0f;
            hitboxSize.y = fontSize + buttonComp->paddingY * 2.0f;
        }

        // Calculate hitbox position (top-left corner)
        // For text buttons, transform->position is at text baseline, so offset upward
        float hitboxX = transform->position.x + buttonComp->hitboxOffset.x - buttonComp->paddingX;
        float hitboxY = transform->position.y + buttonComp->hitboxOffset.y;

        if (buttonComp->autoSizeFromText && textComp && textComp->font) {
            float fontSize = static_cast<float>(textComp->font->GetFontSize()) * textComp->scale;
            hitboxY -= fontSize * 0.8f + buttonComp->paddingY; // Adjust for baseline
        }

        // Return {x, y, width, height}
        return glm::vec4(hitboxX, hitboxY, hitboxSize.x, hitboxSize.y);
    }

    bool ButtonSystem::IsPointInHitbox(const glm::vec2& point, const glm::vec4& hitbox) {
        // hitbox = {x, y, width, height}
        return point.x >= hitbox.x && point.x <= hitbox.x + hitbox.z &&
               point.y >= hitbox.y && point.y <= hitbox.y + hitbox.w;
    }

} // namespace GP2Engine
