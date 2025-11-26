/**
 * @file LevelEditor.hpp
 * @author Rifqah Batrisya (100%)
 * @brief Experimental level editor for tile-based level creation, camera testing and TileRenderer debugging
 *
 * Provides tile placement, camera controls, multiple level management, 
 * and real-time visual feedback.
 * Supports 2D workflows with intuitive UI panels.
 *
 * Note: This version was developed as a tilemap-driven sandbox to test editor
 * functionality and camera behavior. It was not integrated into the final
 * ImGui-based LevelEditor, but retained for reference and experimentation.
 * Reviewed and approved by tech lead for submission.
 */


#pragma once

#include "../Graphics/Renderer.hpp"
#include "../Graphics/Sprite.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/DebugRenderer.hpp"
#include "../Graphics/Camera.hpp"
#include "../Math/Vector2D.hpp"
#include "TileMap.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class TileRenderer;
namespace GP2Engine {
    class TileMap;
}

namespace GP2Engine {

    /**
     * @brief Level editor system for creating and modifying tile-based levels
     *
     * Implements a comprehensive level editing environment with multiple editing modes,
     * real-time visual feedback, and support for managing multiple level files. The editor
     * provides both 2D and 3D editing capabilities with a free-roaming camera system.
     */
    class LevelEditor {
    public:
        /**
         * @brief Available editing modes for the level editor
         */
        enum class EditorMode {
            PlaceTiles,     // Place new tiles in the level
            EraseTiles,     // Remove existing tiles from the level  
            SelectTiles,    // Select tiles for manipulation
            MoveCamera      // Navigate the camera around the level
        };

        /**
         * @brief Data structure representing a level file
         */
        struct LevelData {
            std::string name;        // Display name of the level
            std::string filepath;    // File system path to the level data
            int wallCount = 0;       // Number of wall tiles in the level
            int floorCount = 0;      // Number of floor tiles in the level
        };

        /**
         * @brief Default constructor
         */
        LevelEditor();

        /**
         * @brief Default destructor
         */
        ~LevelEditor() = default;

        /**
         * @brief Initialize the level editor system
         * @param tileMap Pointer to the tile map system
         * @param tileRenderer Pointer to the tile rendering system
         * @return True if initialization succeeded, false otherwise
         */
        bool Initialize(TileMap* tileMap, TileRenderer* tileRenderer);

        /**
         * @brief Update the level editor state
         * @param deltaTime Time elapsed since last frame in seconds
         */
        void Update(float deltaTime);

        /**
         * @brief Render editor-specific visuals and overlays
         */
        void Render();

        /**
         * @brief Render all ImGui editor panels and interfaces
         */
        void RenderImGui();

        // Input handling methods
        /**
         * @brief Handle mouse click input for tile editing
         * @param mousePos Current mouse position in screen coordinates
         * @param leftPressed True if left mouse button is pressed
         * @param rightPressed True if right mouse button is pressed
         */
        void HandleMouseInput(const Vector2D& mousePos, bool leftPressed, bool rightPressed);

        /**
         * @brief Handle mouse movement for camera control
         * @param mouseDelta Change in mouse position since last frame
         * @param rightPressed True if right mouse button is pressed (for camera rotation)
         */
        void HandleMouseMove(const Vector2D& mouseDelta, bool rightPressed);

        /**
         * @brief Handle mouse scroll input for camera zoom
         * @param scrollOffset Scroll wheel delta value
         */
        void HandleMouseScroll(float scrollOffset);

        /**
         * @brief Handle keyboard input for editor commands
         * @param key GLFW key code of the pressed key
         * @param action GLFW action (press, release, repeat)
         */
        void HandleKeyboardInput(int key, int action);

        /**
         * @brief Handle file drop events for asset importing
         * @param filepath Path to the dropped file
         */
        void HandleFileDrop(const std::string& filepath);

        // Getters
        /**
         * @brief Get the current editor mode
         * @return Current EditorMode value
         */
        EditorMode GetEditorMode() const { return m_CurrentMode; }

        /**
         * @brief Get the currently selected tile type for placement
         * @return Tile type ID (0 for floor, 1 for wall, etc.)
         */
        int GetCurrentTileType() const { return m_CurrentTileType; }

        /**
         * @brief Check if the grid visualization is enabled
         * @return True if grid is visible, false otherwise
         */
        bool IsGridVisible() const { return m_ShowGrid; }

        /**
         * @brief Get the coordinates of the currently hovered tile
         * @return Vector2D containing tile column and row indices
         */
        Vector2D GetHoveredTileCoords() const { return m_HoveredTileCoords; }

        /**
         * @brief Get the value/type of the currently hovered tile
         * @return Tile value (-1 if no tile is hovered)
         */
        int GetHoveredTileValue() const { return m_HoveredTileValue; }

        /**
         * @brief Get reference to the editor camera
         * @return Reference to the editor Camera object
         */
        Camera& GetEditorCamera() { return m_EditorCamera; }

        /**
         * @brief Check if the editor camera is currently active
         * @return True if using editor camera, false if using game camera
         */
        bool IsUsingEditorCamera() const { return m_UseEditorCamera; }

        // Setters
        /**
         * @brief Set the current editor mode
         * @param mode New EditorMode to activate
         */
        void SetEditorMode(EditorMode mode) { m_CurrentMode = mode; }

        /**
         * @brief Set the tile type for placement operations
         * @param type Tile type ID to use for new placements
         */
        void SetCurrentTileType(int type) { m_CurrentTileType = type; }

        /**
         * @brief Set grid visualization visibility
         * @param show True to show grid, false to hide
         */
        void SetGridVisible(bool show) { m_ShowGrid = show; }

        /**
         * @brief Switch between editor camera and game camera
         * @param useEditorCam True to use editor camera, false for game camera
         */
        void SetUseEditorCamera(bool useEditorCam) {
            m_UseEditorCamera = useEditorCam;
            if (useEditorCam) {
                ResetEditorCamera();
            }
        }

        /**
         * @brief Update the viewport size for camera calculations
         * @param size New viewport dimensions in pixels
         */
        void SetViewportSize(const Vector2D& size);

    private:
        // Core systems
        TileMap* m_TileMap;              // Pointer to tile map data system
        TileRenderer* m_TileRenderer;    // Pointer to tile rendering system

        // Editor state
        EditorMode m_CurrentMode{ EditorMode::PlaceTiles };  ///< Current editing mode
        int m_CurrentTileType{ 1 };      // Currently selected tile type for placement
        bool m_ShowGrid{ true };         // Grid visualization toggle
        bool m_UseEditorCamera{ true };  // Camera mode toggle (editor vs game camera)

        // Tile interaction state
        Vector2D m_HoveredTileCoords{ -1.0f, -1.0f };  ///< Coordinates of hovered tile
        int m_HoveredTileValue{ -1 };    // Value of the currently hovered tile

        // Level management
        std::vector<LevelData> m_Levels; // List of available level files
        int m_CurrentLevelIndex{ 0 };    // Index of currently loaded level

        // UI panel visibility toggles
        bool m_ShowPropertyEditor{ true };   // Tile properties panel visibility
        bool m_ShowHierarchy{ true };        // Tile hierarchy panel visibility  
        bool m_ShowLevelManager{ true };     // Level manager panel visibility

        // ==================== EDITOR CAMERA SYSTEM ====================
        Camera m_EditorCamera;           // Free-roaming editor camera

        // Camera control state
        Vector2D m_MouseDelta{ 0.0f, 0.0f }; // Mouse movement delta for camera rotation
        bool m_IsRotating{ false };      // Camera rotation active flag
        bool m_IsPanning{ false };       // Camera panning active flag

        // Camera control parameters
        float m_CameraMoveSpeed{ 5.0f };     // Camera movement speed multiplier
        float m_CameraRotationSpeed{ 0.3f }; // Camera rotation sensitivity
        float m_CameraZoomSpeed{ 0.05f };     // Camera zoom speed multiplier
        float m_CameraPanSpeed{ 0.01f };     // Camera panning speed multiplier

        // Input state tracking
        bool m_CameraKeys[512]{ false }; // Keyboard state array for camera movement
        Vector2D m_ViewportSize{ 1280.0f, 720.0f };  ///< Current viewport dimensions

        // Editor Camera Methods
        /**
         * @brief Update editor camera based on current input state
         * @param deltaTime Time elapsed since last frame in seconds
         */
        void UpdateEditorCamera(float deltaTime);

        /**
         * @brief Update camera position based on keyboard input
         * @param deltaTime Time elapsed since last frame in seconds
         */
        void UpdateCameraPosition(float deltaTime);

        /**
         * @brief Update camera rotation based on mouse input
         */
        void UpdateCameraRotation();

        /**
         * @brief Update camera projection matrix based on current settings
         */
        void UpdateCameraProjection();

        /**
         * @brief Reset editor camera to default position and orientation
         */
        void ResetEditorCamera();

        /**
         * @brief Process keyboard input for camera control commands
         * @param key GLFW key code
         * @param action GLFW action (press, release, repeat)
         */
        void HandleCameraKeyInput(int key, int action);

        // Level management methods
        /**
         * @brief Scan directory for available level files
         */
        void ScanLevelFiles();

        /**
         * @brief Load a specific level by index
         * @param index Index of the level to load
         */
        void LoadLevel(int index);

        /**
         * @brief Save the current level to disk
         */
        void SaveCurrentLevel();

        /**
         * @brief Create a new empty level file
         */
        void CreateNewLevel();

        // Tile editing methods
        /**
         * @brief Update hovered tile coordinates based on mouse position
         * @param screenPos Current mouse position in screen coordinates
         */
        void UpdateHoveredTile(const Vector2D& screenPos);

        /**
         * @brief Place a tile at the current hovered position
         */
        void PlaceTile();

        /**
         * @brief Erase tile at the current hovered position
         */
        void EraseTile();

        /**
         * @brief Render editor-specific visual feedback (selection boxes, etc.)
         */
        void RenderEditorVisuals();

        // IMGUI Panels
        /**
         * @brief Render tile properties editor panel
         */
        void DrawPropertyEditor();

        /**
         * @brief Render tile hierarchy and statistics panel
         */
        void DrawHierarchy();

        /**
         * @brief Render level management panel
         */
        void DrawLevelManager();

        /**
         * @brief Render camera controls and status panel
         */
        void DrawCameraControls();
    };
}