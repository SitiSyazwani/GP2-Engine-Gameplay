/**
 * @file SceneManager.cpp
 * @author Adi (100%)
 * @brief Implementation of scene management functionality
 *
 * Implements scene lifecycle operations and event-driven communication
 * with the editor UI through EventSystem.
 */

#include "SceneManager.hpp"
#include "../Core/EditorEvents.hpp"
#include <filesystem>

void SceneManager::Initialize(GP2Engine::Registry* registry) {
    // Store registry reference for scene operations
    m_registry = registry;
}

void SceneManager::SubscribeToEvents() {
    // Subscribe to menu bar events for scene operations
    m_newSceneHandler = GP2Engine::EventSystem::Subscribe<EditorNewSceneEvent>(
        [this](const EditorNewSceneEvent&) { HandleNewScene(); }
    );

    m_loadSceneHandler = GP2Engine::EventSystem::Subscribe<EditorLoadSceneEvent>(
        [this](const EditorLoadSceneEvent& event) { LoadScene(event.scenePath); }
    );

    m_saveSceneHandler = GP2Engine::EventSystem::Subscribe<EditorSaveSceneEvent>(
        [this](const EditorSaveSceneEvent&) { SaveCurrentScene(); }
    );

    m_exitHandler = GP2Engine::EventSystem::Subscribe<EditorExitEvent>(
        [this](const EditorExitEvent&) { HandleExit(); }
    );

    // Subscribe to dialog response events for user confirmations
    m_saveAsConfirmHandler = GP2Engine::EventSystem::Subscribe<EditorSaveAsConfirmEvent>(
        [this](const EditorSaveAsConfirmEvent& event) { SaveSceneAs(event.filename); }
    );

    m_saveAndContinueHandler = GP2Engine::EventSystem::Subscribe<EditorSaveAndContinueEvent>(
        [this](const EditorSaveAndContinueEvent&) { SaveCurrentScene(); }
    );

    m_discardChangesHandler = GP2Engine::EventSystem::Subscribe<EditorDiscardChangesEvent>(
        [this](const EditorDiscardChangesEvent& event) {
            // Handle discard action based on what triggered it
            if (event.action == EditorDiscardChangesEvent::Action::NewScene) {
                NewScene();
            } else if (event.action == EditorDiscardChangesEvent::Action::Exit) {
                GP2Engine::EventSystem::Publish(EditorRequestExitEvent{});
            }
        }
    );
}

void SceneManager::UnsubscribeFromEvents() {
    // Unsubscribe all event handlers to prevent dangling callbacks
    GP2Engine::EventSystem::Unsubscribe<EditorNewSceneEvent>(m_newSceneHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorLoadSceneEvent>(m_loadSceneHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorSaveSceneEvent>(m_saveSceneHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorExitEvent>(m_exitHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorSaveAsConfirmEvent>(m_saveAsConfirmHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorSaveAndContinueEvent>(m_saveAndContinueHandler);
    GP2Engine::EventSystem::Unsubscribe<EditorDiscardChangesEvent>(m_discardChangesHandler);
}

void SceneManager::NewScene() {
    // Clear existing entities from the scene
    ClearAllEntities();

    // Reset ECS registry state for fresh scene
    m_registry->ResetEntityIDs();        // Reset ID counter for deterministic IDs
    m_registry->ClearAllComponents();    // Clear all component storage

    // Reset scene manager state
    m_currentScenePath = "";
    m_hasUnsavedChanges = false;
    LOG_INFO("New scene created");
}

void SceneManager::LoadScene(const std::string& path) {
    LOG_INFO("Loading scene: " + path);

    // Clear current scene before loading new one
    ClearAllEntities();

    // Load scene from JSON file using engine's JsonSerializer
    if (GP2Engine::JsonSerializer::LoadScene(*m_registry, path)) {
        m_currentScenePath = path;
        m_hasUnsavedChanges = false;
        size_t entityCount = m_registry->GetActiveEntities().size();
        LOG_INFO("Scene loaded successfully: " + path + " (" + std::to_string(entityCount) + " entities)");

        // Auto-attach ButtonComponents to button entities (ECS setup)
        AttachButtonComponentsToScene();

        // Notify other editor systems that scene was loaded
        GP2Engine::EventSystem::Publish(EditorSceneLoadedEvent{path});
    } else {
        LOG_ERROR("Failed to load scene: " + path);
        m_currentScenePath = "";
    }
}

void SceneManager::SaveCurrentScene() {
    // If no path set, create a default temp scene path
    if (m_currentScenePath.empty()) {
        LOG_INFO("Save As dialog needed (defaulting to temp scene)");
        m_currentScenePath = std::string(SCENES_PATH) + "/editor_temp.json";
    }

    // Save scene to JSON file using engine's JsonSerializer
    if (GP2Engine::JsonSerializer::SaveScene(*m_registry, m_currentScenePath)) {
        m_hasUnsavedChanges = false;
        LOG_INFO("Scene saved to: " + m_currentScenePath);

        // Notify other editor systems that scene was saved
        GP2Engine::EventSystem::Publish(EditorSceneSavedEvent{m_currentScenePath});
    } else {
        LOG_ERROR("Failed to save scene!");
    }
}

void SceneManager::SaveSceneAs(const std::string& filename) {
    // Build full path from scenes directory and filename
    std::string fullPath = std::string(SCENES_PATH) + "/" + filename;

    // Ensure .json extension is present
    if (fullPath.find(".json") == std::string::npos) {
        fullPath += ".json";
    }

    // Save to new path using engine's JsonSerializer
    if (GP2Engine::JsonSerializer::SaveScene(*m_registry, fullPath)) {
        m_currentScenePath = fullPath;
        m_hasUnsavedChanges = false;
        ScanForScenes(SCENES_PATH);  // Refresh available scenes list
        LOG_INFO("Scene saved as: " + fullPath);

        // Notify other editor systems that scene was saved
        GP2Engine::EventSystem::Publish(EditorSceneSavedEvent{fullPath});
    } else {
        LOG_ERROR("Failed to save scene as: " + fullPath);
    }
}

void SceneManager::ScanForScenes(const std::string& scenesPath) {
    m_availableScenes.clear();

    // Validate scenes directory exists
    if (!std::filesystem::exists(scenesPath)) {
        LOG_INFO("Scenes directory not found: " + scenesPath);
        return;
    }

    try {
        // Scan directory for .json scene files
        for (const auto& entry : std::filesystem::directory_iterator(scenesPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                m_availableScenes.push_back(entry.path().string());
            }
        }
        LOG_INFO("Found " + std::to_string(m_availableScenes.size()) + " scene(s)");
    } catch (const std::exception& e) {
        LOG_ERROR("Error scanning scenes directory: " + std::string(e.what()));
    }
}

void SceneManager::HandleNewScene() {
    // Check for unsaved changes before creating new scene
    if (m_hasUnsavedChanges) {
        // Show warning dialog (false = not an exit action)
        GP2Engine::EventSystem::Publish(EditorShowUnsavedWarningEvent{false});
    } else {
        // No unsaved changes, proceed with new scene
        NewScene();
    }
}

void SceneManager::HandleExit() {
    // Check for unsaved changes before exiting
    if (m_hasUnsavedChanges) {
        // Show warning dialog (true = exit action)
        GP2Engine::EventSystem::Publish(EditorShowUnsavedWarningEvent{true});
    } else {
        LOG_INFO("ESC pressed - Quitting editor...");
        // No unsaved changes, proceed with exit
        GP2Engine::EventSystem::Publish(EditorRequestExitEvent{});
    }
}

void SceneManager::ClearAllEntities() {
    // Copy entity IDs to vector first to avoid iterator invalidation
    std::vector<GP2Engine::EntityID> entitiesToDestroy;
    for (auto entity : m_registry->GetActiveEntities()) {
        entitiesToDestroy.push_back(entity);
    }

    // Now safely destroy all entities
    for (GP2Engine::EntityID entity : entitiesToDestroy) {
        m_registry->DestroyEntity(entity);
    }
}

void SceneManager::AttachButtonComponentsToScene() {
    /**
     * @author Asri (100%)
     * @brief Auto-attach ButtonComponents to button entities for ECS button system
     *
     * Scans all entities in the loaded scene for button tag names and automatically
     * attaches the appropriate ButtonComponent. This ensures buttons work in both
     * the editor and game without manual component attachment.
     *
     * This follows pure ECS architecture: any entity can be a button by having ButtonComponent.
     */

    int buttonsAttached = 0;

    // Scan all entities for button tag names
    for (GP2Engine::EntityID entity : m_registry->GetActiveEntities()) {
        auto* tag = m_registry->GetComponent<GP2Engine::Tag>(entity);
        if (!tag) continue;

        // Check if entity already has ButtonComponent (avoid duplicates)
        if (m_registry->GetComponent<GP2Engine::ButtonComponent>(entity)) {
            continue;
        }

        // Attach ButtonComponent based on tag name
        if (tag->name == "StartButton") {
            m_registry->AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::StartGame));
            LOG_INFO("Attached ButtonComponent (StartGame) to entity " + std::to_string(entity));
            buttonsAttached++;
        } else if (tag->name == "QuitButton") {
            m_registry->AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::QuitGame));
            LOG_INFO("Attached ButtonComponent (QuitGame) to entity " + std::to_string(entity));
            buttonsAttached++;
        } else if (tag->name == "SettingsButton") {
            m_registry->AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::OpenSettings));
            LOG_INFO("Attached ButtonComponent (OpenSettings) to entity " + std::to_string(entity));
            buttonsAttached++;
        }
    }

    if (buttonsAttached > 0) {
        LOG_INFO("Auto-attached " + std::to_string(buttonsAttached) + " ButtonComponent(s) to scene");
    }
}
