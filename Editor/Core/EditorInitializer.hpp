/**
 * @file EditorInitializer.hpp
 * @author Adi (100%)
 * @brief Helper class for initializing editor subsystems
 *
 * Provides initialization methods for the editor's core systems including:
 * - ECS component registration
 * - ImGui initialization and docking setup
 * - Editor UI components (viewport, panels, asset browser)
 * - Tile map editing system
 *
 * This class separates initialization logic from the main editor application
 * for better organization and maintainability.
 */

#pragma once

#include <Engine.hpp>
#include <memory>

// Forward declarations
class ContentEditorUI;
class EditorViewport;
class AssetBrowser;

/**
 * @brief Handles initialization of editor subsystems
 *
 * Provides methods to set up the editor in stages, allowing for organized
 * initialization of ECS components, ImGui, UI panels, and tile editing systems.
 */
class EditorInitializer {
public:
    EditorInitializer() = default;
    ~EditorInitializer() = default;

    /**
     * @brief Register all ECS components used by the editor
     * @param registry ECS registry to register components with
     *
     * Registers Transform2D, SpriteComponent, PhysicsComponent, AudioComponent,
     * Tag, TileMapComponent, and TextComponent.
     */
    void RegisterECSComponents(GP2Engine::Registry& registry);

    /**
     * @brief Initialize ImGui for the editor
     * @param window GLFW window handle
     * @param layoutFile Path to ImGui layout .ini file for docking persistence
     *
     * Sets up ImGui with docking enabled and dark theme.
     */
    void InitializeImGui(GLFWwindow* window, const char* layoutFile);

    /**
     * @brief Initialize core editor UI components
     * @param editorCamera Camera for scene visualization
     * @param registry ECS registry reference
     * @param editorUI Entity hierarchy and properties panel
     * @param viewport Main scene viewport for editing
     * @param assetBrowser Texture and asset selection panel
     * @param sceneWidth Width of the scene in pixels
     * @param sceneHeight Height of the scene in pixels
     * @param texturesPath Path to textures directory for asset browser
     *
     * Sets up the editor camera, viewport, UI panels, and asset browser.
     */
    void InitializeEditorComponents(
        GP2Engine::Camera& editorCamera,
        GP2Engine::Registry& registry,
        ContentEditorUI& editorUI,
        EditorViewport& viewport,
        AssetBrowser& assetBrowser,
        int sceneWidth,
        int sceneHeight,
        const char* texturesPath
    );

    /**
     * @brief Initialize tile map editing system
     * @param tileMap Unique pointer to tile map instance
     * @param tileRenderer Unique pointer to tile renderer instance
     * @param levelEditor Unique pointer to level editor instance
     * @param editorUI Reference to editor UI for integration
     * @param scenesPath Path to scenes directory containing tile definitions
     *
     * Creates and initializes the tile map, renderer, and level editor for
     * tile-based level editing functionality.
     */
    void InitializeTileSystem(
        std::unique_ptr<GP2Engine::TileMap>& tileMap,
        std::unique_ptr<GP2Engine::TileRenderer>& tileRenderer,
        std::unique_ptr<GP2Engine::LevelEditor>& levelEditor,
        ContentEditorUI& editorUI,
        const char* scenesPath
    );
};
