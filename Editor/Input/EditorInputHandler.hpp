/**
 * @file EditorInputHandler.hpp
 * @author Adi (100%)
 * @brief Centralized input handling for editor operations
 *
 * Manages input routing for different editor states:
 * - Edit mode: Viewport camera controls, entity selection, tile editing
 * - Play mode: Game input (handled by game systems)
 * - ESC key: Mode switching and application exit
 *
 * Uses the engine's Input system to check key/mouse states and forwards
 * input to appropriate systems based on editor state and UI context.
 */

#pragma once

#include <Engine.hpp>

// Forward declarations
class EditorViewport;
class PlayModeManager;

/**
 * @brief Processes and routes editor input to appropriate systems
 *
 * Central input handler that checks editor state and viewport context
 * to route keyboard and mouse input to the correct systems.
 */
class EditorInputHandler {
public:
    EditorInputHandler() = default;
    ~EditorInputHandler() = default;

    /**
     * @brief Initialize input handler with editor component references
     * @param viewport Editor viewport for mouse input and hover detection
     * @param playModeManager Play mode manager for state checking
     * @param levelEditor Tile editor for tile map mouse input
     */
    void Initialize(
        EditorViewport* viewport,
        PlayModeManager* playModeManager,
        GP2Engine::LevelEditor* levelEditor
    );

    /**
     * @brief Process input each frame
     *
     * Routes input based on editor state:
     * - ESC key handling (stop play mode or exit editor)
     * - Edit mode: Forward mouse input to level editor when viewport is hovered
     * - Play mode: Input handled by game systems directly
     */
    void HandleInput();

private:
    EditorViewport* m_viewport = nullptr;
    PlayModeManager* m_playModeManager = nullptr;
    GP2Engine::LevelEditor* m_levelEditor = nullptr;
};
