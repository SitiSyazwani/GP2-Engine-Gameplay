/**
 * @file PlayModeManager.cpp
 * @author Adi (100%)
 * @brief Implementation of play mode management
 *
 * Manages transitions between EDIT and PLAY states. Creates JSON snapshots
 * of the scene and camera state before entering play mode, then restores
 * them when exiting. Delegates actual game logic to HollowsPlayMode.
 */

#include "PlayModeManager.hpp"

void PlayModeManager::Initialize(GP2Engine::Registry* registry, GP2Engine::Camera* camera, int sceneWidth, int sceneHeight) {
    // Store references to engine systems needed for play mode
    m_registry = registry;
    m_camera = camera;

    // Store scene dimensions for play mode initialization
    m_sceneWidth = sceneWidth;
    m_sceneHeight = sceneHeight;
}

bool PlayModeManager::StartPlayMode() {
    LOG_INFO("=== Starting Play Mode ===");

    // Save current scene state to temporary JSON snapshot for restoration on exit
    // This captures all entities, components, and their property values
    if (GP2Engine::JsonSerializer::SaveScene(*m_registry, TEMP_SNAPSHOT_FILE)) {
        LOG_INFO("Scene state saved for play mode");
    } else {
        LOG_ERROR("Failed to save scene snapshot!");
        return false;
    }

    // Save camera state (position and zoom) to restore editor view on exit
    m_savedCameraPosition = m_camera->GetPosition();
    m_savedCameraZoom = m_camera->GetZoom();
    LOG_INFO("Saved camera position: (" + std::to_string(m_savedCameraPosition.x) + ", " + std::to_string(m_savedCameraPosition.y) + ")");

    // Initialize and start Hollows play mode logic
    if (!m_playMode.Start(*m_registry, *m_camera, m_sceneWidth, m_sceneHeight)) {
        LOG_ERROR("Failed to start play mode!");
        return false;
    }

    // Transition to PLAY state
    m_state = EditorState::PLAY;
    return true;
}

void PlayModeManager::StopPlayMode() {
    LOG_INFO("=== Stopping Play Mode ===");

    // Restore scene from JSON snapshot which reverts all entity/component changes made during play
    if (GP2Engine::JsonSerializer::LoadScene(*m_registry, TEMP_SNAPSHOT_FILE)) {
        LOG_INFO("Scene restored from snapshot");
    } else {
        LOG_ERROR("Failed to restore scene snapshot!");
    }

    // Stop Hollows play mode and cleanup game state
    m_playMode.Stop();

    // Restore camera to original editor view (position and zoom)
    m_camera->SetPosition(m_savedCameraPosition);
    m_camera->SetZoom(m_savedCameraZoom);
    LOG_INFO("Restored camera position: (" + std::to_string(m_savedCameraPosition.x) + ", " + std::to_string(m_savedCameraPosition.y) + ")");

    // Transition back to EDIT state
    m_state = EditorState::EDIT;
    LOG_INFO("=== Edit Mode Active ===");
}

void PlayModeManager::Update(float deltaTime) {
    // Only run game logic when in PLAY state (not in EDIT mode)
    if (m_state == EditorState::PLAY) {
        // Delegate to Hollows play mode for game update logic
        m_playMode.Update(*m_registry, deltaTime, *m_camera, m_sceneWidth, m_sceneHeight);
    }
}
