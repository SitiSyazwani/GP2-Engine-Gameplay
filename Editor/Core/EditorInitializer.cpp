/**
 * @file EditorInitializer.cpp
 * @author Adi (100%)
 * @brief Implementation of editor initialization helpers
 *
 * Contains the implementation of initialization methods for setting up
 * the editor's core systems.
 */

#include "EditorInitializer.hpp"
#include "../Editor.hpp"

void EditorInitializer::RegisterECSComponents(GP2Engine::Registry& registry) {
    // Register all component types that can be attached to entities in the editor
    registry.RegisterComponent<GP2Engine::Transform2D>();        // Position, rotation, scale
    registry.RegisterComponent<GP2Engine::SpriteComponent>();    // Visual sprite rendering
    registry.RegisterComponent<GP2Engine::PhysicsComponent>();   // Physics body and collision
    registry.RegisterComponent<GP2Engine::AudioComponent>();     // Sound playback
    registry.RegisterComponent<GP2Engine::Tag>();                // Entity naming/identification
    registry.RegisterComponent<GP2Engine::TileMapComponent>();   // Tile map data
    registry.RegisterComponent<GP2Engine::TextComponent>();      // Text rendering
    LOG_INFO("ECS components registered");
}

void EditorInitializer::InitializeImGui(GLFWwindow* window, const char* layoutFile) {
    // Verify ImGui version compatibility
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Configure ImGui with keyboard navigation and docking support
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = false;  // Docking without holding Shift
    io.IniFilename = layoutFile;        // Persist window layout to file

    // Apply dark theme
    ImGui::StyleColorsDark();

    // Initialize ImGui backends for GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    LOG_INFO("ImGui initialized");
}

void EditorInitializer::InitializeEditorComponents(
    GP2Engine::Camera& editorCamera,
    GP2Engine::Registry& registry,
    ContentEditorUI& editorUI,
    EditorViewport& viewport,
    AssetBrowser& assetBrowser,
    int sceneWidth,
    int sceneHeight,
    const char* texturesPath
) {
    // Setup editor camera with orthographic projection matching scene dimensions
    editorCamera.SetOrthographic(0.0f, (float)sceneWidth, (float)sceneHeight, 0.0f);
    editorCamera.SetPosition(GP2Engine::Vector2D(0.0f, 0.0f));
    editorCamera.SetZoom(1.0f);

    // Initialize editor UI panels
    editorUI.Initialize();
    viewport.Initialize(&registry, &editorCamera, sceneWidth, sceneHeight);

    // Connect viewport to UI's selected entity for synchronized selection
    viewport.SetSelectedEntityPtr(editorUI.GetSelectedEntityPtr());

    //editorUI.SetViewport(&viewport);  // Connect UI to viewport for gizmo controls

    // Initialize asset browser with textures directory
    assetBrowser.Initialize(texturesPath);
}

void EditorInitializer::InitializeTileSystem(
    std::unique_ptr<GP2Engine::TileMap>& tileMap,
    std::unique_ptr<GP2Engine::TileRenderer>& tileRenderer,
    std::unique_ptr<GP2Engine::LevelEditor>& levelEditor,
    ContentEditorUI& editorUI,
    const char* scenesPath
) {
    // Create tile map and renderer instances
    tileMap = std::make_unique<GP2Engine::TileMap>();
    tileRenderer = std::make_unique<GP2Engine::TileRenderer>();

    // Load tile definitions from JSON file
    std::string tileDefPath = std::string(scenesPath) + "/test1.json";
    tileMap->LoadTileDefinitionsFromJSON(tileDefPath);

    // Connect tile renderer to tile map data
    tileRenderer->m_tileMap = tileMap.get();
    tileRenderer->InitializeDimensions();

    // Create and initialize level editor for tile-based editing
    levelEditor = std::make_unique<GP2Engine::LevelEditor>();
    levelEditor->Initialize(tileMap.get(), tileRenderer.get());

    // Integrate tile editing with the editor UI
    editorUI.SetLevelEditor(levelEditor.get(), tileMap.get(), tileRenderer.get());

    LOG_INFO("TileMap and LevelEditor initialized");
}
