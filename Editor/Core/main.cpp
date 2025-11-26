/**
 * @file main.cpp
 * @author Adi (100%)
 * @brief Main entry point and application class for the GP2Engine Content Editor
 *
 * Defines the LevelEditor class, which inherits from GP2Engine::Application,
 * providing a standalone visual editor for creating and editing game scenes.
 * Manages the editor lifecycle, UI panels, scene management, and play mode testing.
 *
 * Architecture:
 * - EditorViewport: Main canvas for visualizing and editing the scene
 * - ContentEditorUI: Entity hierarchy and property inspector
 * - AssetBrowser: Texture and asset selection panel
 * - SceneManager: Scene loading, saving, and state management
 * - PlayModeManager: In-editor game testing
 */

#include "../Editor.hpp"
#include <filesystem>


class LevelEditor : public GP2Engine::Application {
private:
    // Editor constants
    static constexpr float DEFAULT_SPRITE_SIZE = 128.0f;
    static constexpr const char* SANDBOX_ASSETS_PATH = "../../Sandbox/assets";
    static constexpr const char* SCENES_PATH = "../../Sandbox/assets/scenes";
    static constexpr const char* TEXTURES_PATH = "../../Sandbox/assets/textures";
    static constexpr const char* EDITOR_LAYOUT_FILE = "../../Editor/editor_layout.ini";
    static constexpr const char* TEMP_SNAPSHOT_FILE = "temp_editor_snapshot.json";

    // Editor UI components
    ContentEditorUI m_editorUI;
    EditorViewport m_viewport;
    AssetBrowser m_assetBrowser;
    ConsolePanel m_console;
    ControlsPanel m_controlsPanel;
    SpriteEditor m_spriteEditor;

    // Editor managers
    SceneManager m_sceneManager;
    EditorMenuBar m_menuBar;
    EditorDialogs m_dialogs;
    PlayModeManager m_playModeManager;
    EditorInitializer m_initializer;
    EditorInputHandler m_inputHandler;
    EditorDebugRenderer m_debugrenderer;

    // Editor camera for scene visualization
    GP2Engine::Camera m_editorCamera;

    // Tile map editing system
    std::unique_ptr<GP2Engine::TileMap> m_tileMap;
    std::unique_ptr<GP2Engine::TileRenderer> m_tileRenderer;
    std::unique_ptr<GP2Engine::LevelEditor> m_levelEditor;

    // Scene dimensions from config
    int m_sceneWidth = 1024;
    int m_sceneHeight = 768;

    // Debug visualization
    EditorDebugRenderer m_debugRenderer;
    bool m_showCollisionBoxes = false;

public:
    void OnStart() override {
        // Initialize console logging system
        m_console.Initialize();
        LOG_INFO("=== GP2Engine Content Editor Starting ===");

        // Load configuration and scene dimensions
        GP2Engine::ConfigLoader& config = GP2Engine::ConfigLoader::GetInstance();
        config.LoadConfig("assets/configs/game_config.json");
        m_sceneWidth = config.GetInt("window.width", 1024);
        m_sceneHeight = config.GetInt("window.height", 768);
        LOG_INFO("Scene dimensions: " + std::to_string(m_sceneWidth) + "x" + std::to_string(m_sceneHeight));

        // Register all ECS components and initialize resource manager
        m_initializer.RegisterECSComponents(GetRegistry());
        GP2Engine::ResourceManager::GetInstance();

        // Initialize ImGui and editor UI components
        m_initializer.InitializeImGui(GetWindow(), EDITOR_LAYOUT_FILE);
        m_initializer.InitializeEditorComponents(m_editorCamera, GetRegistry(), m_editorUI, m_viewport, m_assetBrowser, m_sceneWidth, m_sceneHeight, TEXTURES_PATH);
        m_initializer.InitializeTileSystem(m_tileMap, m_tileRenderer, m_levelEditor, m_editorUI, SCENES_PATH);

        // Initialize scene and play mode managers
        m_sceneManager.Initialize(&GetRegistry());
        m_sceneManager.ScanForScenes(SCENES_PATH);

        // --- Generate Stress Test Scene (1K Entities) ---
        const std::string TEST_SCENE_PATH = "../../Sandbox/assets/scenes/stress_test_scene.json";
        if (!std::filesystem::exists(TEST_SCENE_PATH)) {
            m_debugrenderer.GenerateStressTestScene();
        }

        m_playModeManager.Initialize(&GetRegistry(), &m_editorCamera, m_sceneWidth, m_sceneHeight);

        // Initialize menu bar with references to all panels
        m_menuBar.Initialize(&m_editorUI, &m_assetBrowser, &m_console, &m_controlsPanel, &m_spriteEditor, &m_showCollisionBoxes);
        m_menuBar.SetSceneInfo(&m_sceneManager.GetCurrentScenePath(), &m_sceneManager.GetUnsavedChangesRef());

        // Initialize viewport and input handling
        m_viewport.SetDebugVisualization(&m_showCollisionBoxes);
        m_inputHandler.Initialize(&m_viewport, &m_playModeManager, m_levelEditor.get());

        // Subscribe to editor events through EventSystem
        m_sceneManager.SubscribeToEvents();
        SubscribeToEditorEvents();

        



        LOG_INFO("=== Editor Ready ===");
    }

    void Update(float deltaTime) override {
        // Handle input
        m_inputHandler.HandleInput();

        // Update viewport camera
        m_viewport.Update(deltaTime);

        // Update editor UI state
        m_editorUI.Update(deltaTime, GetRegistry(), m_sceneManager.GetCurrentScenePath(), m_sceneManager.GetUnsavedChangesRef());

        // Update menu bar state
        m_menuBar.SetAvailableScenes(m_sceneManager.GetAvailableScenes());
        m_menuBar.SetCanSave(!m_sceneManager.GetCurrentScenePath().empty());

        // Update level editor
        if (m_levelEditor) {
            m_levelEditor->Update(deltaTime);
        }

        // Handle texture drop from AssetBrowser to viewport (only in edit mode)
        if (!m_playModeManager.IsPlaying() && m_viewport.HasDraggedTexture()) {
            // Create sprite entity manually
            GP2Engine::EntityID newEntity = GetRegistry().CreateEntity();

            // Add Transform2D component
            GP2Engine::Transform2D transform;
            transform.position = m_viewport.GetSceneMousePosition();
            transform.rotation = 0.0f;
            transform.scale = GP2Engine::Vector2D(1.0f, 1.0f);
            GetRegistry().AddComponent<GP2Engine::Transform2D>(newEntity, transform);

            // Load texture and add SpriteComponent
            auto& resMgr = GP2Engine::ResourceManager::GetInstance();
            auto texture = resMgr.LoadTexture(m_viewport.GetDraggedTexturePath());
            if (texture && texture->IsValid()) {
                GP2Engine::SpriteComponent spriteComp;
                spriteComp.renderLayer = 1;
                spriteComp.visible = true;
                spriteComp.size = GP2Engine::Vector2D(DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE);
                spriteComp.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                spriteComp.sprite = std::make_shared<GP2Engine::Sprite>(texture);
                spriteComp.sprite->SetSize(spriteComp.size);
                GetRegistry().AddComponent<GP2Engine::SpriteComponent>(newEntity, spriteComp);
            }

            // Add Tag component
            GP2Engine::Tag tag;
            tag.name = "New Entity";
            tag.group = "default";
            GetRegistry().AddComponent<GP2Engine::Tag>(newEntity, tag);

            m_sceneManager.SetUnsavedChanges(true);
            m_viewport.ClearDraggedTexture();
        }

        // Play mode updates
        m_playModeManager.Update(deltaTime);
    }

    void Render() override {
        // Use the engine's Renderer to clear the screen
        auto& renderer = GP2Engine::Renderer::GetInstance();
        renderer.Clear();

        // Render tilemap
        if (m_tileRenderer) {
            m_tileRenderer->Render(renderer);
        }

        // Begin ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render dockspace and menu bar
        m_menuBar.RenderWithDockspace(static_cast<EditorMenuBar::EditorState>(m_playModeManager.GetState()));

        // Render editor viewport (shows scene entities)
        m_viewport.Render();

        // Render editor UI panels
        m_editorUI.Render(GetRegistry(), m_sceneManager.GetCurrentScenePath(), m_sceneManager.GetUnsavedChangesRef(), m_playModeManager.IsPlaying());

        // Render asset browser
        m_assetBrowser.Render();

        // Render console panel
        m_console.Render();

        // Render controls help panel
        m_controlsPanel.Render();

        //Render sprite editor
        m_spriteEditor.Render();

        // Render dialogs
        m_dialogs.RenderSaveAsDialog();
        m_dialogs.RenderUnsavedChangesWarning();

        // End ImGui frame and render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

private:
    /**
     * @brief Subscribe to all editor events
     *
     * Sets up event subscriptions for UI interactions, play mode control,
     * and application lifecycle management.
     */
    void SubscribeToEditorEvents() {
        // Subscribe to Save As event to show dialog
        GP2Engine::EventSystem::Subscribe<EditorSaveSceneAsEvent>(
            [this](const EditorSaveSceneAsEvent&) {
                m_dialogs.ShowSaveAsDialog();
            }
        );

        // Subscribe to play/stop events
        GP2Engine::EventSystem::Subscribe<EditorPlayEvent>(
            [this](const EditorPlayEvent&) {
                if (m_playModeManager.StartPlayMode()) {
                    m_viewport.SetCameraControlsEnabled(false);
                    m_viewport.SetDragDropEnabled(false);
                }
            }
        );

        // Subscribe to play/stop events
        GP2Engine::EventSystem::Subscribe<EditorStopEvent>(
            [this](const EditorStopEvent&) {
                m_playModeManager.StopPlayMode();
                m_viewport.SetCameraControlsEnabled(true);
                m_viewport.SetDragDropEnabled(true);
            }
        );

        // Subscribe to unsaved warning event to show dialog
        GP2Engine::EventSystem::Subscribe<EditorShowUnsavedWarningEvent>(
            [this](const EditorShowUnsavedWarningEvent& event) {
                if (event.isExit) {
                    m_dialogs.ShowUnsavedWarning(EditorDialogs::UnsavedAction::Exit);
                } else {
                    m_dialogs.ShowUnsavedWarning(EditorDialogs::UnsavedAction::NewScene);
                }
            }
        );

        // Subscribe to exit request event to close application
        GP2Engine::EventSystem::Subscribe<EditorRequestExitEvent>(
            [this](const EditorRequestExitEvent&) {
                Close();
            }
        );
    }

};

// Application entry point
int main() {
    // Enable run-time memory check for debug builds
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    LOG_INFO("==========================================================");
    LOG_INFO("        GP2Engine - Level Editor");
    LOG_INFO("==========================================================");

    LevelEditor editor;
    if (!editor.Init(1600, 900, "GP2Engine - Level Editor")) {
        LOG_ERROR("Failed to initialize editor!");
        return -1;
    }

    // Set target FPS to 200
    GP2Engine::Time::SetTargetFPS(200);

    editor.Run();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    LOG_INFO("ImGui shut down successfully");

    return 0;
}
