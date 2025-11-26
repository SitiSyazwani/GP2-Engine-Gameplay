/**
 * @file Sprite.hpp
 * @brief Sprite class for 2D rendering with animation support
 * @author Asri (100%)
 * 
 * This file contains the Sprite class definition which represents a drawable
 * 2D object with position, rotation, scale, and texture. It supports sprite
 * sheet animations and provides all transform operations.
 */

#pragma once

#include "Texture.hpp"
#include "../Math/Vector2D.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace GP2Engine {

    // Forward declarations
    class Renderer;
    struct Transform2D;
    
    /**
     * @brief Animation frame data
     *
     * Contains information about a single frame in an animation sequence.
     * All coordinates are in PIXELS (not normalized).
     *
     */
    struct AnimationFrame {
        glm::vec2 sourcePosition{0.0f}; ///< Position in sprite sheet (PIXEL coordinates)
        glm::vec2 sourceSize{1.0f};     ///< Size in sprite sheet (PIXEL coordinates)
        float duration{1.0f/12.0f};     ///< Duration in seconds (default 12 FPS)
        
        /**
         * @brief Default constructor
         */
        AnimationFrame() = default;
        
        /**
         * @brief Constructor with parameters
         * 
         * @param pos Position in sprite sheet
         * @param size Size in sprite sheet
         * @param dur Duration in seconds
         */
        AnimationFrame(glm::vec2 pos, glm::vec2 size, float dur = 1.0f/12.0f)
            : sourcePosition(pos), sourceSize(size), duration(dur) {}
    };
    
    /**
     * @brief Animation data container
     * 
     * Contains a sequence of animation frames and playback settings.
     * 
     * @author Asri (100%)
     */
    struct Animation {
        std::vector<AnimationFrame> frames; ///< Animation frames
        bool loop{true};                    ///< Whether animation loops
        std::string name;                   ///< Animation name
        
        /**
         * @brief Default constructor
         */
        Animation() = default;
        
        /**
         * @brief Constructor with name and loop setting
         * 
         * @param animName Animation name
         * @param shouldLoop Whether animation should loop
         */
        Animation(const std::string& animName, bool shouldLoop = true)
            : loop(shouldLoop), name(animName) {}
    };
    
    /**
     * @brief Sprite class for 2D rendering with animation support
     * 
     * Represents a drawable 2D object with position, rotation, scale, and texture.
     * Supports sprite sheet animations and provides all transform operations
     * required by the rubrics (scaling, rotation, etc.).
     * 
     * Satisfies Rubric 1205 (scaling), 1206 (rotation), 1207 (animation), and 1208 (animation control).
     * 
     * @author Asri (100%)
     */
    class Sprite {
    public:
        /**
         * @brief Default constructor
         */
        Sprite();
        
        /**
         * @brief Constructor with texture
         * 
         * @param texture Texture to use for the sprite
         */
        explicit Sprite(TexturePtr texture);
        
        /**
         * @brief Constructor with texture and position
         * 
         * @param texture Texture to use for the sprite
         * @param position Initial position
         */
        Sprite(TexturePtr texture, const Vector2D& position);
        
        /**
         * @brief Destructor
         */
        ~Sprite() = default;
        
        /**
         * @brief Set sprite texture
         * 
         * @param texture Texture to use
         */
        void SetTexture(TexturePtr texture);
        
        /**
         * @brief Get sprite texture
         * 
         * @return Pointer to texture
         */
        TexturePtr GetTexture() const { return m_Texture; }
        
        /**
         * @brief Set sprite position using Vector2D
         * 
         * @param position New position
         */
        void SetPosition(const Vector2D& position) { m_Position = position; }
        
        /**
         * @brief Set sprite position using coordinates
         * 
         * @param x X coordinate
         * @param y Y coordinate
         */
        void SetPosition(float x, float y) { m_Position = Vector2D(x, y); }
        
        /**
         * @brief Set uniform scale (Rubric 1205)
         * 
         * @param scale Scale factor for both axes
         */
        void SetScale(float scale) { m_Scale = Vector2D(scale, scale); }
        
        /**
         * @brief Set scale using Vector2D (Rubric 1205)
         * 
         * @param scale Scale factors for X and Y axes
         */
        void SetScale(const Vector2D& scale) { m_Scale = scale; }
        
        /**
         * @brief Set scale using separate values (Rubric 1205)
         * 
         * @param scaleX Scale factor for X axis
         * @param scaleY Scale factor for Y axis
         */
        void SetScale(float scaleX, float scaleY) { m_Scale = Vector2D(scaleX, scaleY); }
        
        /**
         * @brief Set sprite rotation (Rubric 1206)
         *
         * @param degrees Rotation angle in degrees
         */
        void SetRotation(float degrees) { m_Rotation = degrees; }

        /**
         * @brief Rotate sprite to point toward a target position
         *
         * @param targetX Target X position (world/screen space)
         * @param targetY Target Y position (world/screen space)
         */
        void RotateTowardPoint(float targetX, float targetY);
        
        /**
         * @brief Set sprite size using Vector2D
         *
         * @param size New size
         */
        void SetSize(const Vector2D& size) {
            m_Size = size;
            m_SizeManuallySet = true;
        }

        /**
         * @brief Set sprite size using separate values
         *
         * @param width Width
         * @param height Height
         */
        void SetSize(float width, float height) {
            m_Size = Vector2D(width, height);
            m_SizeManuallySet = true;
        }

        /**
         * @brief Set sprite flip state for mirroring
         *
         * @param flipX Flip horizontally
         * @param flipY Flip vertically
         */
        void SetFlip(bool flipX, bool flipY) {
            m_FlipX = flipX;
            m_FlipY = flipY;
        }

        /**
         * @brief Set horizontal flip state
         *
         * @param flip Flip horizontally
         */
        void SetFlipX(bool flip) { m_FlipX = flip; }

        /**
         * @brief Set vertical flip state
         *
         * @param flip Flip vertically
         */
        void SetFlipY(bool flip) { m_FlipY = flip; }

        /**
         * @brief Get horizontal flip state
         *
         * @return true if flipped horizontally
         */
        bool GetFlipX() const { return m_FlipX; }

        /**
         * @brief Get vertical flip state
         *
         * @return true if flipped vertically
         */
        bool GetFlipY() const { return m_FlipY; }

        /**
         * @brief Get sprite position
         *
         * @return Reference to position
         */
        const Vector2D& GetPosition() const { return m_Position; }
        
        /**
         * @brief Get sprite scale
         * 
         * @return Reference to scale
         */
        const Vector2D& GetScale() const { return m_Scale; }
        
        /**
         * @brief Get sprite rotation
         * 
         * @return Rotation angle in degrees
         */
        float GetRotation() const { return m_Rotation; }
        
        /**
         * @brief Get sprite size
         * 
         * @return Reference to size
         */
        const Vector2D& GetSize() const { return m_Size; }
        
        /**
         * @brief Set sprite color
         * 
         * @param color RGBA color
         */
        void SetColor(const glm::vec4& color) { m_Color = color; }
        
        /**
         * @brief Set sprite color using components
         * 
         * @param r Red component
         * @param g Green component
         * @param b Blue component
         * @param a Alpha component
         */
        void SetColor(float r, float g, float b, float a = 1.0f) { m_Color = glm::vec4(r, g, b, a); }
        
        /**
         * @brief Set sprite opacity
         * 
         * @param alpha Alpha value (0.0 to 1.0)
         */
        void SetOpacity(float alpha) { m_Color.a = alpha; }
        
        /**
         * @brief Get sprite color
         * 
         * @return Reference to color
         */
        const glm::vec4& GetColor() const { return m_Color; }
        
        /**
         * @brief Set sprite visibility
         * 
         * @param visible Whether sprite is visible
         */
        void SetVisible(bool visible) { m_Visible = visible; }
        
        /**
         * @brief Check if sprite is visible
         * 
         * @return true if visible, false otherwise
         */
        bool IsVisible() const { return m_Visible; }
        
        /**
         * @brief Set source rectangle for sprite sheet
         * 
         * @param sourceRect Source rectangle (normalized coordinates)
         */
        void SetSourceRect(const glm::vec4& sourceRect) { m_SourceRect = sourceRect; }
        
        /**
         * @brief Set source rectangle using components
         * 
         * @param x X coordinate
         * @param y Y coordinate
         * @param width Width
         * @param height Height
         */
        void SetSourceRect(float x, float y, float width, float height) { 
            m_SourceRect = glm::vec4(x, y, width, height); 
        }
        
        /**
         * @brief Get source rectangle
         * 
         * @return Reference to source rectangle
         */
        const glm::vec4& GetSourceRect() const { return m_SourceRect; }
        
        /**
         * @brief Add animation to sprite (Rubric 1207)
         * 
         * @param name Animation name
         * @param animation Animation data
         */
        void AddAnimation(const std::string& name, const Animation& animation);
        
        /**
         * @brief Add animation using frame list (Rubric 1207)
         * 
         * @param name Animation name
         * @param frames Animation frames
         * @param loop Whether animation loops
         */
        void AddAnimation(const std::string& name, const std::vector<AnimationFrame>& frames, bool loop = true);
        
        /**
         * @brief Play animation (Rubric 1208)
         * 
         * @param name Animation name to play
         * @return true if animation found and started, false otherwise
         */
        bool PlayAnimation(const std::string& name);
        
        /**
         * @brief Stop current animation (Rubric 1208)
         */
        void StopAnimation();
        
        /**
         * @brief Pause current animation
         */
        void PauseAnimation();
        
        /**
         * @brief Resume paused animation
         */
        void ResumeAnimation();
        
        /**
         * @brief Update animation timing
         * 
         * @param deltaTime Time elapsed since last update
         */
        void UpdateAnimation(float deltaTime);
        
        /**
         * @brief Check if animation is playing
         * 
         * @return true if animation is playing, false otherwise
         */
        bool IsAnimationPlaying() const { return m_IsAnimationPlaying; }
        
        /**
         * @brief Get current animation name
         * 
         * @return Current animation name
         */
        const std::string& GetCurrentAnimationName() const { return m_CurrentAnimationName; }
        
        /**
         * @brief Get current animation frame
         * 
         * @return Current frame index
         */
        int GetCurrentFrame() const { return m_CurrentFrame; }
        
        /**
         * @brief Draw sprite using internal transform
         * 
         * @param renderer Renderer to use
         */
        void Draw(Renderer& renderer);
        
        /**
         * @brief Draw sprite with external transform
         * 
         * @param renderer Renderer to use
         * @param transform External transform to apply
         */
        void Draw(Renderer& renderer, const Transform2D* transform);
        
        /**
         * @brief Get world size (after scaling)
         * 
         * @return World size
         */
        glm::vec2 GetWorldSize() const;
        
        /**
         * @brief Get sprite bounds in world space
         * 
         * @return AABB bounds (minX, minY, maxX, maxY)
         */
        glm::vec4 GetBounds() const;
        
    private:
        // Transform properties
        Vector2D m_Position{0.0f, 0.0f};     ///< Sprite position
        Vector2D m_Scale{1.0f, 1.0f};         ///< Sprite scale factors
        float m_Rotation{0.0f};              ///< Sprite rotation in degrees
        Vector2D m_Size{100.0f, 100.0f};     ///< Sprite size (default 100x100)
        bool m_FlipX{false};                 ///< Horizontal flip flag
        bool m_FlipY{false};                 ///< Vertical flip flag
        
        // Rendering properties
        TexturePtr m_Texture{nullptr};       ///< Sprite texture
        glm::vec4 m_Color{1.0f, 1.0f, 1.0f, 1.0f}; ///< Sprite color (RGBA)
        bool m_Visible{true};                ///< Sprite visibility
        glm::vec4 m_SourceRect{0.0f, 0.0f, 1.0f, 1.0f}; ///< Source rectangle (normalized)
        
        // Animation system
        std::unordered_map<std::string, Animation> m_Animations; ///< Animation map
        std::string m_CurrentAnimationName;  ///< Current animation name
        int m_CurrentFrame{0};               ///< Current frame index
        float m_AnimationTimer{0.0f};       ///< Animation timer
        bool m_IsAnimationPlaying{false};    ///< Animation playing flag
        bool m_IsAnimationPaused{false};     ///< Animation paused flag
        bool m_SizeManuallySet{false};       ///< Whether size was manually set by user
        
        /**
         * @brief Update source rectangle from current frame
         */
        void UpdateSourceRectFromCurrentFrame();
        
        /**
         * @brief Get current animation
         * 
         * @return Pointer to current animation, or nullptr if none
         */
        const Animation* GetCurrentAnimation() const;
    };
    
    // Type alias for shared sprite pointer
    using SpritePtr = std::shared_ptr<Sprite>;
    
} // namespace GP2Engine