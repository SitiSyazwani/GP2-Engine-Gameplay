/**
 * @file Component.hpp
 * @author Adi (100%)
 * @brief ECS component definitions for GP2Engine
 *
 * Components are pure data structures that define entity properties.
 * Integrated with existing engine systems (Graphics, Physics, Audio)
 *
 */

#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "../Math/Vector2D.hpp"
#include "Entity.hpp"

namespace GP2Engine {

    // Forward declarations for engine systems
    class Sprite;
    class PhysicsBody;
    class TileMap;
    class TileRenderer;
    class Font;
    using SpritePtr = std::shared_ptr<Sprite>;
    using FontPtr = std::shared_ptr<Font>;

    /**
     * @brief 2D Transform component
     *
     * Stores spatial data for entities in 2D space.
     * Used by rendering, physics, and collision systems.
     *
     * Position is in world coordinates, rotation in degrees, scale is multiplicative.
     */
    struct Transform2D {
        Vector2D position{0.0f, 0.0f};
        float rotation = 0.0f;        // Rotation in degrees
        Vector2D scale{1.0f, 1.0f};

        Transform2D() = default;
        Transform2D(const Vector2D& pos) : position(pos) {}
        Transform2D(const Vector2D& pos, float rot) : position(pos), rotation(rot) {}
        Transform2D(const Vector2D& pos, float rot, const Vector2D& scl) : position(pos), rotation(rot), scale(scl) {}
    };

    /**
     * @brief Sprite component for rendering
     *
     * Supports two rendering modes:
     * 1. Sprite object - textured quad using engine's Sprite system
     * 2. Colored quad - solid color rectangle
     *
     * RenderSystem uses this component to determine how to render entities.
     * Layer controls draw order (0 = background, higher = foreground).
     */
    struct SpriteComponent {
        // Core rendering data
        SpritePtr sprite = nullptr;         // If set, renders textured sprite
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};  // Tint for sprites or color for quads
        Vector2D size{32.0f, 32.0f};       // Size

        // Rendering properties
        bool visible = true;
        int renderLayer = 1;                // Render order (higher = drawn later)

        // UV coordinates for sprite sheets
        Vector2D uvOffset{0.0f, 0.0f};     // UV start position (0-1 range)
        Vector2D uvSize{1.0f, 1.0f};       // UV size (0-1 range)

        // Constructors
        SpriteComponent() = default;

        // Create colored quad
        explicit SpriteComponent(const Vector2D& quadSize, const glm::vec4& col = glm::vec4(1.0f)) : color(col), size(quadSize) {}

        SpriteComponent(float width, float height, float r, float g, float b, float a = 1.0f) : color(r, g, b, a), size(Vector2D(width, height)) {}

        // Create sprite
        explicit SpriteComponent(SpritePtr spr, int layer = 1) : sprite(spr), renderLayer(layer) {}

        // Helper methods
        bool IsTextured() const { return sprite != nullptr; }
        bool IsQuad() const { return !IsTextured(); }
    };

    /**
     * @brief Physics component
     *
     * Links entity to physics engine PhysicsBody system.
     * The PhysicsBody is owned and updated by the physics engine.
     *
     * If syncTransform is true, Transform2D will be updated
     * from PhysicsBody position each frame.
     */
    struct PhysicsComponent {
        PhysicsBody* body = nullptr;  // Owned by PhysicsSystem
        bool syncTransform = true;     // Sync with Transform2D

        PhysicsComponent() = default;
        explicit PhysicsComponent(PhysicsBody* physicsBody) : body(physicsBody) {}
        PhysicsComponent(PhysicsBody* physicsBody, bool autoSync) : body(physicsBody), syncTransform(autoSync) {}
    };

    /**
     * @brief Audio component
     *
     * References sound files for playback through the audio engine.
     *
     */
    struct AudioComponent {
        std::string soundFile;         // Path to sound file
        float volume = 1.0f;
        bool loop = false;
        bool playOnStart = false;

        AudioComponent() = default;
        explicit AudioComponent(const std::string& file) : soundFile(file) {}
        AudioComponent(const std::string& file, float vol, bool shouldLoop = false) : soundFile(file), volume(vol), loop(shouldLoop) {}
    };


    /**
     * @brief Tag component
     *
     * Used for entity identification and grouping.
     * Name identifies individual entities (e.g., "Player", "Enemy1").
     * Group categorizes entities for batch operations (e.g., "enemies", "pickups").
     *
     * Tags are also used by systems:
     * - RenderSystem checks for "StressTest" tag to enable batching
     * - EntityCollisionSystem skips "Background" and "StressTest" entities
     */
    struct Tag {
        std::string name;
        std::string group;             // Optional grouping (e.g., "enemies", "pickups")

        Tag() = default;
        explicit Tag(const std::string& tagName) : name(tagName) {}
        Tag(const std::string& tagName, const std::string& tagGroup) : name(tagName), group(tagGroup) {}
    };

    /**
     * @brief TileMap component for rendering tile-based maps
     *
     * Renders a TileMap using TileRenderer.
     * The TileMap and TileRenderer are managed externally (usually in the main game class).
     *
     * RenderLayer defaults to -1 to render behind other sprites.
     * Lower values = rendered first (background), higher values = rendered later (foreground).
     */
    struct TileMapComponent {
        TileMap* tileMap = nullptr;           // Pointer to TileMap data
        TileRenderer* tileRenderer = nullptr; // Pointer to TileRenderer

        // Rendering properties
        bool visible = true;
        int renderLayer = -1;                 // Default to background (-1)

        TileMapComponent() = default;
        TileMapComponent(TileMap* map, TileRenderer* renderer, int layer = -1)
            : tileMap(map), tileRenderer(renderer), renderLayer(layer) {}
    };

    /**
     * @brief Text component for rendering text strings
     *
     * Renders text using a Font at a screen position.
     * Position is relative to Transform2D if present, or absolute if not.
     *
     * RenderLayer controls draw order (same as SpriteComponent).
     * Text color supports alpha for transparency.
     */
    struct TextComponent {
        FontPtr font = nullptr;               // Font to use for rendering
        std::string text;                     // Text string to display
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f}; // Text color (RGBA)
        float scale = 1.0f;                   // Text scale factor

        // Rendering properties
        bool visible = true;
        int renderLayer = 10;                 // Default to foreground (10)

        // Optional offset from Transform2D position
        Vector2D offset{0.0f, 0.0f};

        TextComponent() = default;
        explicit TextComponent(FontPtr fnt, const std::string& txt = "", int layer = 10)
            : font(fnt), text(txt), renderLayer(layer) {}
        TextComponent(FontPtr fnt, const std::string& txt, const glm::vec4& col, float scl = 1.0f, int layer = 10)
            : font(fnt), text(txt), color(col), scale(scl), renderLayer(layer) {}
    };

    /**
     * @brief AI component for pathfinding and chase behavior
     *
     * Attach this component to any entity to give it AI pathfinding capabilities.
     * The AISystem will process all entities with this component and handle
     * pathfinding, movement, and target tracking.
     *
     * Requires Transform2D component on the same entity.
     * Optionally requires SpriteComponent for animation updates.
     */
    struct AIComponent {
        // Target tracking
        EntityID targetEntity = INVALID_ENTITY;  // Entity to chase (e.g., player)

        // AI behavior parameters
        float chaseSpeed = 200.0f;               // Movement speed
        float detectionRange = 200.0f;           // Range to detect and chase target
        bool isChasing = false;                  // Currently chasing target

        // Pathfinding state
        bool usePathfinding = true;              // Enable A* pathfinding
        size_t currentPathIndex = 0;             // Current waypoint index
        float pathRecalculateTimer = 0.0f;       // Timer for path recalculation
        float pathRecalculateInterval = 0.5f;    // Recalculate path every N seconds

        // Animation textures (optional, for dynamic sprite switching)
        void* horizontalTexture = nullptr;       // Texture for horizontal movement
        void* verticalTexture = nullptr;         // Texture for vertical movement
        std::string currentAnimation;            // Current animation name

        AIComponent() = default;
        explicit AIComponent(EntityID target) : targetEntity(target) {}
        AIComponent(EntityID target, float speed, float range)
            : targetEntity(target), chaseSpeed(speed), detectionRange(range) {}
    };

    /**
     * @brief Button component for interactive UI elements
     *
     * Attach this component to any entity to make it clickable.
     * Works with TextComponent or SpriteComponent for visual representation.
     * ButtonSystem processes all entities with this component for hover/click detection.
     *
     * Requires Transform2D component on the same entity.
     * Optionally requires TextComponent for text buttons.
     */
    struct ButtonComponent {
        // Button action type
        enum class Action {
            None,
            StartGame,
            OpenSettings,
            QuitGame,
            Custom
        };

        Action action = Action::None;           // Action to trigger on click
        std::string customAction;               // Custom action name (if Action::Custom)

        // Hitbox configuration
        glm::vec2 hitboxSize{200.0f, 60.0f};   // Button hitbox size
        glm::vec2 hitboxOffset{0.0f, 0.0f};    // Offset from transform position
        float paddingX = 20.0f;                 // Horizontal padding for auto-sizing
        float paddingY = 10.0f;                 // Vertical padding for auto-sizing
        bool autoSizeFromText = true;           // Auto-calculate hitbox from TextComponent

        // Visual feedback
        glm::vec4 normalColor{1.0f, 1.0f, 1.0f, 1.0f};   // Normal state color
        glm::vec4 hoverColor{1.0f, 1.0f, 0.0f, 1.0f};    // Hovered state color
        glm::vec4 clickColor{0.8f, 0.8f, 0.0f, 1.0f};    // Clicked state color

        // State
        bool isHovered = false;                 // Currently hovered by mouse
        bool isPressed = false;                 // Currently being pressed
        bool wasClicked = false;                // Clicked this frame (consumed by systems)

        ButtonComponent() = default;
        explicit ButtonComponent(Action act) : action(act) {}
        ButtonComponent(Action act, const glm::vec2& size)
            : action(act), hitboxSize(size), autoSizeFromText(false) {}
    };
}