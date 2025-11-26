/**
 * @file LevelEditor.cpp
 * @author Rifqah Batrisya (100%)
 * @brief Basic tile-based level editor implementation
 */

#include "LevelEditor.hpp"
#include "TileRenderer.hpp"
#include "TileMap.hpp"
#include "../Graphics/DebugRenderer.hpp"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <filesystem>

 // Add GLFW key definitions if not already included
#define GLFW_KEY_1 49
#define GLFW_KEY_2 50
#define GLFW_KEY_3 51
#define GLFW_KEY_G 71
#define GLFW_KEY_0 48
#define GLFW_KEY_9 57
#define GLFW_KEY_F 70
#define GLFW_KEY_W 87
#define GLFW_KEY_A 65
#define GLFW_KEY_S 83
#define GLFW_KEY_D 68
#define GLFW_KEY_Q 81
#define GLFW_KEY_E 69
#define GLFW_KEY_R 82
#define GLFW_KEY_C 67
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#define GLFW_PRESS 1
#define GLFW_REPEAT 2


namespace GP2Engine {

    LevelEditor::LevelEditor()
        : m_TileMap(nullptr), m_TileRenderer(nullptr) {
        // Initialize editor camera with perspective projection
        m_EditorCamera.SetPerspective(45.0f, m_ViewportSize.x / m_ViewportSize.y, 0.1f, 1000.0f);
        m_UseEditorCamera = false;
        ResetEditorCamera();
    }

    bool LevelEditor::Initialize(TileMap* tileMap, TileRenderer* tileRenderer) {
        if (!tileMap || !tileRenderer) {
            std::cerr << "LevelEditor: Invalid tilemap or renderer pointers!" << std::endl;
            return false;
        }

        m_TileMap = tileMap;
        m_TileRenderer = tileRenderer;

        // Scan for level files 
        ScanLevelFiles();

        std::cout << "LevelEditor initialized successfully!" << std::endl;
        return true;
    }

    void LevelEditor::Update(float deltaTime) {
        // Update logic can go here
        if (m_UseEditorCamera) {
            UpdateEditorCamera(deltaTime);
        }
    }

    void LevelEditor::Render() {
        if (!m_TileRenderer || !m_TileMap) return;
        RenderEditorVisuals();
    }

    void LevelEditor::RenderImGui() {
        // Draw all editor panels
        DrawPropertyEditor();   
        DrawHierarchy();         
        DrawLevelManager();      
        DrawCameraControls();    
    }

    // ==================== EDITOR CAMERA IMPLEMENTATION ====================
    void LevelEditor::UpdateEditorCamera(float deltaTime) {
        UpdateCameraPosition(deltaTime);
        UpdateCameraRotation();
    }

    void LevelEditor::UpdateCameraPosition(float deltaTime) {
        glm::vec3 movement(0.0f);

        // Get camera rotation
        glm::vec3 rotation = m_EditorCamera.GetRotation();
        float yaw = glm::radians(rotation.y); // Yaw rotation around Y-axis

        // Calculate forward and right vectors based on yaw
        glm::vec3 forward = glm::vec3(-sin(yaw), 0.0f, -cos(yaw));
        glm::vec3 right = glm::vec3(cos(yaw), 0.0f, -sin(yaw));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // WASD movement - camera relative
        if (m_CameraKeys[GLFW_KEY_W]) movement += forward;
        if (m_CameraKeys[GLFW_KEY_S]) movement -= forward;
        if (m_CameraKeys[GLFW_KEY_A]) movement -= right;
        if (m_CameraKeys[GLFW_KEY_D]) movement += right;

        // Q/E for vertical movement
        if (m_CameraKeys[GLFW_KEY_Q]) movement -= up;
        if (m_CameraKeys[GLFW_KEY_E]) movement += up;

        // Normalize and apply speed
        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement) * m_CameraMoveSpeed * deltaTime;
            m_EditorCamera.Move(movement);
        }

        // Arrow keys for world-space panning (optional - remove if not needed)
        glm::vec3 panning(0.0f);
        if (m_CameraKeys[GLFW_KEY_UP]) panning.y += m_CameraPanSpeed * deltaTime;
        if (m_CameraKeys[GLFW_KEY_DOWN]) panning.y -= m_CameraPanSpeed * deltaTime;
        if (m_CameraKeys[GLFW_KEY_LEFT]) panning.x -= m_CameraPanSpeed * deltaTime;
        if (m_CameraKeys[GLFW_KEY_RIGHT]) panning.x += m_CameraPanSpeed * deltaTime;

        if (glm::length(panning) > 0.0f) {
            m_EditorCamera.Move(panning);
        }
    }

    void LevelEditor::UpdateCameraRotation() {
        if (m_IsRotating && glm::length(glm::vec2(m_MouseDelta.x, m_MouseDelta.y)) > 0.0f) {
            // Convert mouse delta to rotation
            glm::vec3 rotationDelta;
            rotationDelta.y = m_MouseDelta.x * m_CameraRotationSpeed; // Yaw
            rotationDelta.x = m_MouseDelta.y * m_CameraRotationSpeed; // Pitch

            // Get current rotation and apply delta
            glm::vec3 currentRotation = m_EditorCamera.GetRotation();
            currentRotation += rotationDelta;

            // Clamp pitch to avoid flipping
            currentRotation.x = glm::clamp(currentRotation.x, -89.0f, 89.0f);

            m_EditorCamera.SetRotation(currentRotation);
        }

        // Reset mouse delta for next frame
        m_MouseDelta = Vector2D(0.0f, 0.0f);
    }

    void LevelEditor::UpdateCameraProjection() {
        if (m_EditorCamera.GetProjectionType() == Camera::ProjectionType::Perspective) {
            // Update perspective projection with new aspect ratio
            m_EditorCamera.SetPerspective(45.0f, m_ViewportSize.x / m_ViewportSize.y, 0.1f, 1000.0f);
        }
        else {
            // Update orthographic projection (if using ortho for 2D editing)
            float zoom = m_EditorCamera.GetZoom();
            float height = m_ViewportSize.y * 0.01f / zoom;
            float width = height * (m_ViewportSize.x / m_ViewportSize.y);
            m_EditorCamera.SetOrthographic(-width, width, -height, height);
        }
    }

    void LevelEditor::ResetEditorCamera() {
        m_EditorCamera.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        m_EditorCamera.SetRotation(glm::vec3(-45.0f, 0.0f, 0.0f)); // Look downward
        m_EditorCamera.SetZoom(1.0f);
        std::cout << "Editor camera reset to default position" << std::endl;
    }

    void LevelEditor::SetViewportSize(const Vector2D& size) {
        if (m_ViewportSize != size) {
            m_ViewportSize = size;
            UpdateCameraProjection();
        }
    }

    // ==================== INPUT HANDLING ====================
    void LevelEditor::HandleMouseInput(const Vector2D& mousePos, bool leftPressed, bool rightPressed) {
        if (!m_TileMap || !m_TileRenderer) return;

        // Only handle tile editing if not using editor camera for movement
        if (!m_UseEditorCamera || !rightPressed) {
            UpdateHoveredTile(mousePos);

            switch (m_CurrentMode) {
            case EditorMode::PlaceTiles:
                if (leftPressed) PlaceTile();
                else if (rightPressed) EraseTile();
                break;
            case EditorMode::EraseTiles:
                if (leftPressed) EraseTile();
                break;
            default:
                break;
            }
        }

        // Set rotation state for editor camera
        if (m_UseEditorCamera) {
            m_IsRotating = rightPressed;
        }
    }

    void LevelEditor::HandleMouseMove(const Vector2D& mouseDelta, bool rightPressed) {
        if (m_UseEditorCamera && rightPressed) {
            m_MouseDelta = mouseDelta;
            m_IsRotating = true;
        }
        else {
            m_IsRotating = false;
        }
    }


    void LevelEditor::HandleMouseScroll(float scrollOffset) {
        if (m_UseEditorCamera) {
            // Get current state
            float currentZoom = m_EditorCamera.GetZoom();
            glm::vec3 currentPos = m_EditorCamera.GetPosition();

            // Calculate new zoom
            float zoomStep = 0.25f;
            float newZoom = currentZoom;
            if (scrollOffset > 0) {
                newZoom -= zoomStep; // Zoom in
            }
            else {
                newZoom += zoomStep; // Zoom out
            }
            newZoom = glm::clamp(newZoom, 0.5f, 3.0f);

            // Get mouse position in screen coordinates
            ImVec2 mousePosImGui = ImGui::GetMousePos();
            Vector2D mousePos(mousePosImGui.x, mousePosImGui.y);

            // Convert mouse position to world coordinates before zoom
            glm::vec2 worldPosBefore = m_EditorCamera.ScreenToWorld(
                glm::vec2(mousePos.x, mousePos.y),
                glm::vec2(m_ViewportSize.x, m_ViewportSize.y)
            );

            // Apply new zoom
            m_EditorCamera.SetZoom(newZoom);

            // Convert mouse position to world coordinates after zoom
            glm::vec2 worldPosAfter = m_EditorCamera.ScreenToWorld(
                glm::vec2(mousePos.x, mousePos.y),
                glm::vec2(m_ViewportSize.x, m_ViewportSize.y)
            );

            // Calculate the difference and adjust camera position to keep mouse over same world point
            glm::vec2 worldDelta = worldPosAfter - worldPosBefore;

            // Move camera in the opposite direction to compensate
            glm::vec3 newPos = currentPos;
            newPos.x -= worldDelta.x;
            newPos.y -= worldDelta.y;

            // Apply the corrected position
            m_EditorCamera.SetPosition(newPos);
        }
    }


    void LevelEditor::HandleKeyboardInput(int key, int action) {
        if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

        // Editor camera controls (always available when editor camera is active)
        if (m_UseEditorCamera) {
            HandleCameraKeyInput(key, action);
        }

        // Tile editing controls (only when not rotating camera)
        if (!m_IsRotating) {
            switch (key) {
            case GLFW_KEY_1: SetEditorMode(EditorMode::PlaceTiles); break;
            case GLFW_KEY_2: SetEditorMode(EditorMode::EraseTiles); break;
            case GLFW_KEY_3: SetEditorMode(EditorMode::SelectTiles); break;

            case GLFW_KEY_G: SetGridVisible(!m_ShowGrid); break;
            case GLFW_KEY_0: SetCurrentTileType(0); break;  // Floor
            case GLFW_KEY_9: SetCurrentTileType(1); break;  // Wall

                // Alternative mappings if needed
            case GLFW_KEY_F: SetCurrentTileType(0); break;  // F for Floor
            case GLFW_KEY_W: SetCurrentTileType(1); break;  // W for Wall
            }
        }

        // Camera toggle (always available)
        if (key == GLFW_KEY_C && action == GLFW_PRESS) {
            m_UseEditorCamera = !m_UseEditorCamera;
            std::cout << "Switched to " << (m_UseEditorCamera ? "Editor" : "Game") << " camera" << std::endl;
        }
    }

    void LevelEditor::HandleCameraKeyInput(int key, int action) {
        if (key >= 0 && key < 512) {
            m_CameraKeys[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }

    }

    // ==================== CAMERA CONTROLS PANEL ====================
    void LevelEditor::DrawCameraControls() {
        if (!ImGui::Begin("Camera Controls", &m_ShowPropertyEditor)) {
            ImGui::End();
            return;
        }

        ImGui::Text("Editor Camera");
        ImGui::Separator();

        // Camera toggle
        if (ImGui::Checkbox("Use Editor Camera", &m_UseEditorCamera)) {
            if (m_UseEditorCamera) {
                ResetEditorCamera();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Camera")) {
            ResetEditorCamera();
        }

        ImGui::Separator();
        ImGui::Text("Camera Position:");
        glm::vec3 pos = m_EditorCamera.GetPosition();
        ImGui::Text("X: %.2f, Y: %.2f", pos.x, pos.y); // Only show X and Y
        ImGui::Text("Zoom Level: %.2f", m_EditorCamera.GetZoom()); // Show zoom


        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::BulletText("WASD: Move horizontally & vertically");
        ImGui::BulletText("Mouse Wheel: Zoom");
        ImGui::BulletText("C: Toggle camera mode");

        ImGui::Separator();
        ImGui::Text("Camera Settings:");
        ImGui::SliderFloat("Move Speed", &m_CameraMoveSpeed, 1.0f, 20.0f);
        ImGui::SliderFloat("Rotation Speed", &m_CameraRotationSpeed, 0.1f, 2.0f);
        ImGui::SliderFloat("Zoom Speed", &m_CameraZoomSpeed, 0.5f, 5.0f);

        ImGui::End();
    }

    void LevelEditor::HandleFileDrop(const std::string& filepath) {
        // Drag and Drop support
        std::string extension = filepath.substr(filepath.find_last_of("."));

        if (extension == ".png" || extension == ".jpg") {
            auto newTexture = Texture::Create(filepath);
            if (newTexture) {
                std::cout << "Loaded new texture: " << filepath << std::endl;
            }
        }
        else if (extension == ".txt") {
            // Add dropped level file to our list
            LevelData newLevel;
            newLevel.name = filepath.substr(filepath.find_last_of("/\\") + 1);
            newLevel.filepath = filepath;
            m_Levels.push_back(newLevel);

            // Auto-load the dropped level
            LoadLevel(static_cast<int>(m_Levels.size() - 1));
            std::cout << "Added and loaded new level: " << filepath << std::endl;
        }
        else if (extension == ".json") {
            std::cout << "Dropped JSON file: " << filepath << std::endl;
            // Optionally reload tile definitions
            m_TileMap->LoadTileDefinitionsFromJSON(filepath);
        }
    }

    // ==================== MULTIPLE LEVELS ====================
    void LevelEditor::ScanLevelFiles() {
        m_Levels.clear();

        // Use the TileMap directory
        std::string levelsDir = "../../Sandbox/assets/textures/";
        try {
            if (std::filesystem::exists(levelsDir)) {
                for (const auto& entry : std::filesystem::directory_iterator(levelsDir)) {
                    if (entry.path().extension() == ".txt") {
                        LevelData level;
                        level.name = entry.path().filename().string();
                        level.filepath = entry.path().string();
                        m_Levels.push_back(level);
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error scanning level files: " << e.what() << std::endl;
        }

        // If no levels found, at least include the default one
        if (m_Levels.empty()) {
            LevelData defaultLevel;
            defaultLevel.name = "tilemap.txt";
            defaultLevel.filepath = "../../Sandbox/assets/textures/tilemap.txt";
            m_Levels.push_back(defaultLevel);
        }

        // Auto-load the first level
        if (!m_Levels.empty()) {
            LoadLevel(0);
        }
    }

    void LevelEditor::LoadLevel(int index) {
        if (index < 0 || index >= m_Levels.size()) return;

        m_CurrentLevelIndex = index;

        m_TileMap->LoadMap(m_Levels[index].filepath);

        std::cout << "Loaded level: " << m_Levels[index].name << std::endl;
    }

    void LevelEditor::SaveCurrentLevel() {
        if (m_CurrentLevelIndex < 0 || m_CurrentLevelIndex >= m_Levels.size()) return;

        m_TileMap->SaveMap(m_Levels[m_CurrentLevelIndex].filepath);

        std::cout << "Saved level: " << m_Levels[m_CurrentLevelIndex].name << std::endl;
    }

    void LevelEditor::CreateNewLevel() {
        // Create new level data
        LevelData newLevel;
        newLevel.name = "level_" + std::to_string(m_Levels.size() + 1) + ".txt";
        newLevel.filepath = "../../Sandbox/assets/textures/" + newLevel.name;
        m_Levels.push_back(newLevel);

        // Create a basic level file with current grid dimensions
        int defaultCols = m_TileMap ? m_TileMap->GetGridCols() : 16;
        int defaultRows = m_TileMap ? m_TileMap->GetGridRows() : 12;

        std::ofstream file(newLevel.filepath);
        if (file.is_open()) {
            file << defaultCols << " " << defaultRows << "\n";
            for (int i = 0; i < defaultRows; ++i) {
                for (int j = 0; j < defaultCols; ++j) {
                    file << "0 ";
                }
                file << "\n";
            }
            file.close();
        }

        // Load the new level using friend's system
        LoadLevel(static_cast<int>(m_Levels.size() - 1));

        std::cout << "Created new level: " << newLevel.name << std::endl;
    }

    // ==================== PROPERTY EDITOR ====================
    void LevelEditor::DrawPropertyEditor() {
        if (!m_ShowPropertyEditor) return;

        if (ImGui::Begin("Tile Properties", &m_ShowPropertyEditor)) {
            ImGui::Text("Editor Mode:");
            if (ImGui::RadioButton("Place Tiles", m_CurrentMode == EditorMode::PlaceTiles))
                SetEditorMode(EditorMode::PlaceTiles);
            if (ImGui::RadioButton("Erase Tiles", m_CurrentMode == EditorMode::EraseTiles))
                SetEditorMode(EditorMode::EraseTiles);

            ImGui::Separator();

            ImGui::Text("Tile Type:");
            if (ImGui::RadioButton("Floor (0)", m_CurrentTileType == 0))
                SetCurrentTileType(0);
            if (ImGui::RadioButton("Wall (1)", m_CurrentTileType == 1))
                SetCurrentTileType(1);

            ImGui::Separator();

            if (m_HoveredTileCoords.x >= 0 && m_HoveredTileCoords.y >= 0) {
                ImGui::Text("Hovered Tile:");
                ImGui::Text("Position: (%d, %d)",
                    (int)m_HoveredTileCoords.x, (int)m_HoveredTileCoords.y);
                ImGui::Text("Type: %s", m_HoveredTileValue == 1 ? "Wall" : "Floor");

                if (ImGui::Button("Set to Floor")) {
                    m_TileMap->SetTileValue((int)m_HoveredTileCoords.x, (int)m_HoveredTileCoords.y, 0);
                }
                ImGui::SameLine();
                if (ImGui::Button("Set to Wall")) {
                    m_TileMap->SetTileValue((int)m_HoveredTileCoords.x, (int)m_HoveredTileCoords.y, 1);
                }
            }
            else {
                ImGui::Text("No tile hovered");
            }

            ImGui::Separator();
            ImGui::Checkbox("Show Grid", &m_ShowGrid);

            // Render debug grid if enabled
            if (m_ShowGrid && m_TileRenderer && m_TileMap) {
                Renderer& renderer = Renderer::GetInstance();
                m_TileRenderer->RenderDebugGrid(renderer);
            }
        }
        ImGui::End();
    }

    // ==================== HIERARCHY ====================
    void LevelEditor::DrawHierarchy() {
        if (!m_ShowHierarchy) return;

        if (ImGui::Begin("Tile Hierarchy", &m_ShowHierarchy)) {
            if (!m_TileMap) {
                ImGui::Text("No tilemap loaded");
                ImGui::End();
                return;
            }

            int wallCount = 0, floorCount = 0;
            const auto& tileData = m_TileMap->getTileMapData();
            for (int value : tileData) {
                if (value == 1) wallCount++;
                else floorCount++;
            }

            ImGui::Text("Level Statistics:");
            ImGui::Text("Walls: %d", wallCount);
            ImGui::Text("Floors: %d", floorCount);
            ImGui::Text("Total Tiles: %d", wallCount + floorCount);
            ImGui::Text("Grid Size: %d x %d", m_TileMap->GetGridCols(), m_TileMap->GetGridRows());

            ImGui::Separator();

            if (ImGui::TreeNodeEx("All Tiles", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Click tile to select:");

                // Use actual grid dimensions from TileMap
                int gridCols = m_TileMap->GetGridCols();
                int gridRows = m_TileMap->GetGridRows();

                for (int row = 0; row < gridRows; ++row) {
                    for (int col = 0; col < gridCols; ++col) {
                        int value = m_TileMap->GetTileValue(col, row);
                        std::string label = "(" + std::to_string(col) + "," + std::to_string(row) + "): " +
                            (value == 1 ? "Wall" : "Floor");

                        if (ImGui::Selectable(label.c_str(),
                            m_HoveredTileCoords.x == col && m_HoveredTileCoords.y == row)) {
                            m_HoveredTileCoords = Vector2D((float)col, (float)row);
                            m_HoveredTileValue = value;
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    // ==================== LEVEL MANAGER ====================
    void LevelEditor::DrawLevelManager() {
        if (!m_ShowLevelManager) return;

        if (ImGui::Begin("Level Manager", &m_ShowLevelManager)) {
            ImGui::Text("Current Level: %s",
                m_CurrentLevelIndex < m_Levels.size() ? m_Levels[m_CurrentLevelIndex].name.c_str() : "None");

            if (m_TileMap) {
                ImGui::Text("Grid Size: %d x %d", m_TileMap->GetGridCols(), m_TileMap->GetGridRows());
            }

            ImGui::Separator();

            if (ImGui::BeginListBox("Levels##LevelList")) {
                for (int i = 0; i < m_Levels.size(); ++i) {
                    bool isSelected = (m_CurrentLevelIndex == i);
                    if (ImGui::Selectable(m_Levels[i].name.c_str(), isSelected)) {
                        LoadLevel(i);
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::Separator();

            if (ImGui::Button("Create New Level")) {
                CreateNewLevel();
            }

            ImGui::SameLine();

            if (ImGui::Button("Save Current Level")) {
                SaveCurrentLevel(); 
            }

            ImGui::SameLine();

            if (ImGui::Button("Refresh Levels")) {
                ScanLevelFiles();
            }

            ImGui::Separator();
            ImGui::Text("- Load: TileMap::LoadMap()");
            ImGui::Text("- Save: TileMap::SaveMap()");
            ImGui::Text("- Format: Auto-detected from file");

            ImGui::Separator();
            ImGui::Text("Drag & Drop Support:");
            ImGui::Text("- Drop .txt files to add levels"); //not yet fully implemented
        }
        ImGui::End();
    }

    // ==================== HELPER METHODS ====================
    void LevelEditor::UpdateHoveredTile(const Vector2D& screenPos) {
        if (!m_TileRenderer || !m_TileMap) return;

        // Always use the camera version, but pass appropriate camera
        if (m_UseEditorCamera) {
            // Use editor camera
            m_HoveredTileCoords = m_TileRenderer->ScreenToTileCoords(screenPos, m_EditorCamera);
        }
        else {
            // Use a default camera (no movement, zoom = 1.0)
            Camera defaultCamera;
            defaultCamera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            defaultCamera.SetZoom(1.0f);
            m_HoveredTileCoords = m_TileRenderer->ScreenToTileCoords(screenPos, defaultCamera);
        }

        if (m_HoveredTileCoords.x >= 0 && m_HoveredTileCoords.y >= 0) {
            m_HoveredTileValue = m_TileMap->GetTileValue(
                static_cast<int>(m_HoveredTileCoords.x),
                static_cast<int>(m_HoveredTileCoords.y)
            );
        }
        else {
            m_HoveredTileValue = -1;
        }
    }

    void LevelEditor::PlaceTile() {
        if (!m_TileMap || m_HoveredTileCoords.x < 0 || m_HoveredTileCoords.y < 0) return;

        int col = static_cast<int>(m_HoveredTileCoords.x);
        int row = static_cast<int>(m_HoveredTileCoords.y);
        m_TileMap->SetTileValue(col, row, m_CurrentTileType);
    }

    void LevelEditor::EraseTile() {
        if (!m_TileMap || m_HoveredTileCoords.x < 0 || m_HoveredTileCoords.y < 0) return;

        int col = static_cast<int>(m_HoveredTileCoords.x);
        int row = static_cast<int>(m_HoveredTileCoords.y);
        m_TileMap->SetTileValue(col, row, 0);
    }

    void LevelEditor::RenderEditorVisuals() {
        if (!m_TileRenderer || m_HoveredTileCoords.x < 0 || m_HoveredTileCoords.y < 0) return;

        Renderer& renderer = Renderer::GetInstance();
        m_TileRenderer->RenderSelectionBox(
            renderer,
            static_cast<int>(m_HoveredTileCoords.x),
            static_cast<int>(m_HoveredTileCoords.y)
        );
    }
}