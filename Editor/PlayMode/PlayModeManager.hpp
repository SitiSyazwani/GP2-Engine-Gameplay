/**
 * @file PlayModeManager.hpp
 * @author Adi (100%)
 * @brief Manages play mode state and transitions for the editor
 *
 * Handles switching between EDIT and PLAY modes. Saves/restores scene state
 * and camera position using JSON snapshots. Delegates game logic to HollowsPlayMode.
 */

#pragma once

#include <Engine.hpp>
#include "HollowsPlayMode.hpp"

class PlayModeManager {
public:
    enum class EditorState { EDIT, PLAY };

    PlayModeManager() = default;
    ~PlayModeManager() = default;

    /**
     * @brief Initialize the play mode manager
     * @param registry ECS registry containing the scene
     * @param camera Editor camera to save/restore during transitions
     * @param sceneWidth Width of the scene viewport
     * @param sceneHeight Height of the scene viewport
     */
    void Initialize(GP2Engine::Registry* registry, GP2Engine::Camera* camera, int sceneWidth, int sceneHeight);

    /**
     * @brief Start play mode - saves scene/camera state and initializes game systems
     * @return true if play mode started successfully, false otherwise
     */
    bool StartPlayMode();

    /**
     * @brief Stop play mode - restores scene/camera state and cleanup game systems
     */
    void StopPlayMode();

    /**
     * @brief Update play mode logic each frame (only runs when in PLAY state)
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void Update(float deltaTime);

    /**
     * @brief Get current editor state (EDIT or PLAY)
     * @return Current EditorState
     */
    EditorState GetState() const { return m_state; }

    /**
     * @brief Check if currently in play mode
     * @return true if in PLAY state, false if in EDIT state
     */
    bool IsPlaying() const { return m_state == EditorState::PLAY; }

private:
    // Temporary file path for storing scene snapshot during play mode
    static constexpr const char* TEMP_SNAPSHOT_FILE = "temp_editor_snapshot.json";

    // References to engine systems
    GP2Engine::Registry* m_registry = nullptr;
    GP2Engine::Camera* m_camera = nullptr;
    int m_sceneWidth = 1024;
    int m_sceneHeight = 768;

    // Current editor state and game play mode handler
    EditorState m_state = EditorState::EDIT;
    HollowsPlayMode m_playMode;

    // Saved camera state for restoring after play mode exits
    glm::vec3 m_savedCameraPosition{0.0f, 0.0f, 0.0f};
    float m_savedCameraZoom = 1.0f;
};
