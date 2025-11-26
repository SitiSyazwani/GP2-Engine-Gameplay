/**
 * @file Events.hpp
 * @brief Common event type definitions for GP2Engine
 * @author Asri (100%)
 *
 * Defines common event structures used throughout the engine.
 * Add new event types here as needed.
 */

#pragma once

#include "../Math/Vector2D.hpp"
#include "../ECS/Entity.hpp"

namespace GP2Engine {

    /**
     * @brief Event fired when two entities collide
     */
    struct CollisionEvent {
        EntityID entity1;
        EntityID entity2;
        Vector2D collisionPoint;
        Vector2D collisionNormal;
    };

    /**
     * @brief Event fired when an entity is created
     */
    struct EntityCreatedEvent {
        EntityID entity;
    };

    /**
     * @brief Event fired when an entity is destroyed
     */
    struct EntityDestroyedEvent {
        EntityID entity;
    };

    /**
     * @brief Event fired when a key is pressed
     */
    struct KeyPressedEvent {
        int key;        // GLFW key code
        int scancode;
        int mods;       // Modifier keys (Shift, Ctrl, Alt)
    };

    /**
     * @brief Event fired when a mouse button is clicked
     */
    struct MouseClickEvent {
        int button;     // Mouse button (0=left, 1=right, 2=middle)
        Vector2D position;  // Screen position
    };

    /**
     * @brief Event fired when player takes damage
     */
    struct PlayerDamagedEvent {
        EntityID player;
        EntityID damageSource;
        float damageAmount;
        float remainingHealth;
    };

    /**
     * @brief Event fired when player dies
     */
    struct PlayerDeathEvent {
        EntityID player;
        EntityID killer;
    };

    /**
     * @brief Event fired when an audio sound finishes playing
     */
    struct AudioFinishedEvent {
        std::string soundName;
        int channelId;
    };

    /**
     * @brief Event fired when window is resized
     */
    struct WindowResizeEvent {
        int width;
        int height;
    };

    /**
     * @brief Event fired when an item is picked up
     */
    struct ItemPickupEvent {
        EntityID player;
        EntityID item;
        std::string itemType;
    };

    // ===== MENU EVENTS =====

    /**
     * @brief Event fired when Start Game button is clicked
     */
    struct MenuStartGameEvent {
        // No additional data needed
    };

    /**
     * @brief Event fired when Settings button is clicked
     */
    struct MenuOpenSettingsEvent {
        // No additional data needed
    };

    /**
     * @brief Event fired when Quit button is clicked
     */
    struct MenuQuitGameEvent {
        // No additional data needed
    };

    /**
     * @brief Event fired when any menu button is hovered
     */
    struct MenuButtonHoverEvent {
        std::string buttonName;
    };

} // namespace GP2Engine
