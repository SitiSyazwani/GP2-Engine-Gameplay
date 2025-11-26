/**
 * @file Sprite.cpp
 * @brief Sprite class implementation for 2D rendering with animation support
 * @author Asri (100%)
 * 
 * This file contains the implementation of the Sprite class which represents
 * a drawable 2D object with position, rotation, scale, and texture. It supports
 * sprite sheet animations and provides all transform operations.
 */

#include "Sprite.hpp"
#include "Renderer.hpp"
#include "../ECS/Component.hpp"
#include <iostream>

namespace GP2Engine {

    Sprite::Sprite() = default;

    Sprite::Sprite(TexturePtr texture) {
        SetTexture(texture);
    }

    Sprite::Sprite(TexturePtr texture, const Vector2D& position)
        : m_Position(position) {
        SetTexture(texture);
    }

    void Sprite::SetTexture(TexturePtr texture) {
        m_Texture = texture;

        // Auto-set size from texture if user hasn't manually set it
        if (m_Texture && m_Texture->IsValid() && !m_SizeManuallySet) {
            m_Size = Vector2D(static_cast<float>(m_Texture->GetWidth()),
                             static_cast<float>(m_Texture->GetHeight()));
        }
    }
    
    void Sprite::AddAnimation(const std::string& name, const Animation& animation) {
        m_Animations[name] = animation;
    }
    
    void Sprite::AddAnimation(const std::string& name, const std::vector<AnimationFrame>& frames, bool loop) {
        Animation animation(name, loop);
        animation.frames = frames;
        m_Animations[name] = animation;
    }
    
    bool Sprite::PlayAnimation(const std::string& name) {
        auto it = m_Animations.find(name);
        if (it == m_Animations.end()) {
            std::cerr << "Animation not found: " << name << std::endl;
            return false;
        }
        
        // Only restart if it's a different animation or if current animation has stopped
        if (m_CurrentAnimationName != name || !m_IsAnimationPlaying) {
            m_CurrentAnimationName = name;
            m_CurrentFrame = 0;
            m_AnimationTimer = 0.0f;
            m_IsAnimationPlaying = true;
            m_IsAnimationPaused = false;
            
            // Update source rect immediately
            UpdateSourceRectFromCurrentFrame();
        }
        
        return true;
    }
    
    void Sprite::StopAnimation() {
        m_IsAnimationPlaying = false;
        m_IsAnimationPaused = false;
        m_CurrentFrame = 0;
        m_AnimationTimer = 0.0f;
        
        // Reset to full texture
        m_SourceRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    void Sprite::PauseAnimation() {
        m_IsAnimationPaused = true;
    }
    
    void Sprite::ResumeAnimation() {
        m_IsAnimationPaused = false;
    }

    void Sprite::UpdateAnimation(float deltaTime) {
        if (!m_IsAnimationPlaying || m_IsAnimationPaused) {
            return;
        }

        const Animation* currentAnim = GetCurrentAnimation();
        if (!currentAnim || currentAnim->frames.empty()) {
            return;
        }

        // Validate current frame index
        if (m_CurrentFrame < 0 || m_CurrentFrame >= static_cast<int>(currentAnim->frames.size())) {
            m_CurrentFrame = 0;
        }

        m_AnimationTimer += deltaTime;

        // Check if we should advance to the next frame
        const AnimationFrame& currentFrame = currentAnim->frames[m_CurrentFrame];
        if (m_AnimationTimer >= currentFrame.duration) {
            m_AnimationTimer -= currentFrame.duration;
            m_CurrentFrame++;

            // Check if animation is complete
            if (m_CurrentFrame >= static_cast<int>(currentAnim->frames.size())) {
                if (currentAnim->loop) {
                    m_CurrentFrame = 0; // Loop back to start
                } else {
                    m_CurrentFrame = static_cast<int>(currentAnim->frames.size()) - 1; // Stay on last frame
                    m_IsAnimationPlaying = false; // Stop animation
                }
            }

            // Update source rect for new frame
            UpdateSourceRectFromCurrentFrame();
        }
    }
    
    void Sprite::Draw(Renderer& renderer) {
        if (!m_Visible || !m_Texture || !m_Texture->IsValid()) {
            return;
        }

        // Animation timing is handled externally via UpdateAnimation()
        renderer.DrawSprite(*this);
    }

    void Sprite::Draw(Renderer& renderer, const Transform2D* transform) {
        if (!m_Visible || !m_Texture || !m_Texture->IsValid()) {
            return;
        }

        // Animation timing is handled externally via UpdateAnimation()
        renderer.DrawSprite(*this, transform);
    }
    
    glm::vec2 Sprite::GetWorldSize() const {
        return glm::vec2(m_Size.x * m_Scale.x, m_Size.y * m_Scale.y);
    }
    
    glm::vec4 Sprite::GetBounds() const {
        glm::vec2 worldSize = GetWorldSize();
        glm::vec2 halfSize = worldSize * 0.5f;
        
        // Return as (minX, minY, maxX, maxY)
        return glm::vec4(
            m_Position.x - halfSize.x,
            m_Position.y - halfSize.y,
            m_Position.x + halfSize.x,
            m_Position.y + halfSize.y
        );
    }
    
    void Sprite::UpdateSourceRectFromCurrentFrame() {
        const Animation* currentAnim = GetCurrentAnimation();
        if (!currentAnim || currentAnim->frames.empty()) {
            return;
        }

        // Bounds check
        if (m_CurrentFrame < 0 || m_CurrentFrame >= static_cast<int>(currentAnim->frames.size())) {
            std::cerr << "WARNING: Invalid animation frame index: " << m_CurrentFrame << std::endl;
            return;
        }

        const AnimationFrame& frame = currentAnim->frames[m_CurrentFrame];

        // Frame coordinates are ALWAYS in pixels - convert to normalized [0,1] for rendering
        if (m_Texture && m_Texture->IsValid()) {
            float texWidth = static_cast<float>(m_Texture->GetWidth());
            float texHeight = static_cast<float>(m_Texture->GetHeight());

            m_SourceRect = glm::vec4(
                frame.sourcePosition.x / texWidth,
                frame.sourcePosition.y / texHeight,
                frame.sourceSize.x / texWidth,
                frame.sourceSize.y / texHeight
            );
        } else {
            // No texture - default to full rect (should rarely happen)
            m_SourceRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        }
    }
    
    const Animation* Sprite::GetCurrentAnimation() const {
        if (m_CurrentAnimationName.empty()) {
            return nullptr;
        }
        
        auto it = m_Animations.find(m_CurrentAnimationName);
        return (it != m_Animations.end()) ? &it->second : nullptr;
    }
    
} // namespace GP2Engine