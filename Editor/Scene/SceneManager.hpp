/**
 * @file SceneManager.hpp
 * @author Adi (100%)
 * @brief Manages scene loading, saving, and file operations for the editor
 *
 * Handles scene lifecycle operations including creating, loading, and saving scenes.
 * Subscribes to editor events and publishes events through EventSystem for dialog
 * interactions and application control.
 *
 * Events subscribed to:
 * - EditorNewSceneEvent
 * - EditorLoadSceneEvent
 * - EditorSaveSceneEvent
 * - EditorSaveSceneAsEvent (shows dialog)
 * - EditorExitEvent
 * - EditorSaveAsConfirmEvent (from dialog)
 * - EditorSaveAndContinueEvent (from dialog)
 * - EditorDiscardChangesEvent (from dialog)
 *
 * Events published:
 * - EditorShowUnsavedWarningEvent (when unsaved changes need user confirmation)
 * - EditorRequestExitEvent (when application should exit)
 * - EditorSceneLoadedEvent (when scene is successfully loaded)
 * - EditorSceneSavedEvent (when scene is successfully saved)
 */

#pragma once

#include <Engine.hpp>
#include <string>
#include <vector>

class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() { UnsubscribeFromEvents(); }

    /**
     * @brief Initialize with registry reference and subscribe to editor events
     * @param registry Pointer to the ECS registry
     */
    void Initialize(GP2Engine::Registry* registry);

    /**
     * @brief Subscribe to all editor events this manager handles
     */
    void SubscribeToEvents();

    /**
     * @brief Unsubscribe from all editor events
     */
    void UnsubscribeFromEvents();

    /**
     * @brief Create a new empty scene
     */
    void NewScene();

    /**
     * @brief Load a scene from a file path
     * @param path Full path to the scene JSON file
     */
    void LoadScene(const std::string& path);

    /**
     * @brief Save the current scene to its existing path
     */
    void SaveCurrentScene();

    /**
     * @brief Save the current scene to a new file
     * @param filename Name of the file (extension added automatically)
     */
    void SaveSceneAs(const std::string& filename);

    /**
     * @brief Scan directory for available scene files
     * @param scenesPath Path to the scenes directory
     */
    void ScanForScenes(const std::string& scenesPath);

    // Getters
    const std::string& GetCurrentScenePath() const { return m_currentScenePath; }
    bool HasUnsavedChanges() const { return m_hasUnsavedChanges; }
    bool& GetUnsavedChangesRef() { return m_hasUnsavedChanges; }  // For ContentEditorUI to modify
    const std::vector<std::string>& GetAvailableScenes() const { return m_availableScenes; }

    // Setters
    void SetUnsavedChanges(bool hasChanges) { m_hasUnsavedChanges = hasChanges; }

private:
    // ECS registry reference for entity operations
    GP2Engine::Registry* m_registry = nullptr;

    // Current scene state
    std::string m_currentScenePath;
    bool m_hasUnsavedChanges = false;
    std::vector<std::string> m_availableScenes;

    // Event handler IDs for unsubscribing on cleanup
    GP2Engine::EventSystem::ListenerID m_newSceneHandler = 0;
    GP2Engine::EventSystem::ListenerID m_loadSceneHandler = 0;
    GP2Engine::EventSystem::ListenerID m_saveSceneHandler = 0;
    GP2Engine::EventSystem::ListenerID m_exitHandler = 0;
    GP2Engine::EventSystem::ListenerID m_saveAsConfirmHandler = 0;
    GP2Engine::EventSystem::ListenerID m_saveAndContinueHandler = 0;
    GP2Engine::EventSystem::ListenerID m_discardChangesHandler = 0;

    // Default scenes directory path
    static constexpr const char* SCENES_PATH = "../../Sandbox/assets/scenes";

    /**
     * @brief Clear all entities from the registry
     */
    void ClearAllEntities();

    /**
     * @brief Handle new scene request with unsaved changes check
     */
    void HandleNewScene();

    /**
     * @brief Handle exit request with unsaved changes check
     */
    void HandleExit();

    /**
     * @brief Attach ButtonComponent to button entities in the scene (ECS auto-setup)
     * @author Asri (100%)
     *
     * Scans for entities with specific button tag names (StartButton, QuitButton, etc.)
     * and automatically attaches ButtonComponent with the appropriate action.
     * This ensures buttons work correctly when scenes are loaded in the editor.
     */
    void AttachButtonComponentsToScene();
};
