/**
 * @file EditorInputHandler.cpp
 * @author Adi (100%)
 * @brief Implementation of editor input handling and routing
 *
 * Processes input each frame and routes it to appropriate systems based
 * on editor state (edit/play mode) and UI context (viewport hovered).
 */

#include "EditorInputHandler.hpp"
#include "../Editor.hpp"

void EditorInputHandler::Initialize(
    EditorViewport* viewport,
    PlayModeManager* playModeManager,
    GP2Engine::LevelEditor* levelEditor
) {
    m_viewport = viewport;
    m_playModeManager = playModeManager;
    m_levelEditor = levelEditor;
}

void EditorInputHandler::HandleInput() {
    // ESC key handling: different behavior based on mode
    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::Escape)) {
        if (m_playModeManager->IsPlaying()) {
            // In play mode, publish stop event 
            GP2Engine::EventSystem::Publish(EditorStopEvent{});
        } else {
            // In edit mode, publish exit event
            GP2Engine::EventSystem::Publish(EditorExitEvent{});
        }
    }

    // Edit mode input: only process editor controls when not in play mode
    if (!m_playModeManager->IsPlaying()) {
        // Forward mouse input to LevelEditor when viewport is hovered
        if (m_levelEditor && m_viewport->IsHovered()) {
            // Get mouse position in scene space (transformed from viewport panel)
            GP2Engine::Vector2D sceneMousePos = m_viewport->GetSceneMousePosition();

            // Only if mouse is within viewport bounds
            if (sceneMousePos.x >= 0 && sceneMousePos.y >= 0) {
                bool leftPressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);
                bool rightPressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Right);

                m_levelEditor->HandleMouseInput(sceneMousePos, leftPressed, rightPressed);
            }
        }

        // TODO: Add keyboard shortcuts
    }
}
