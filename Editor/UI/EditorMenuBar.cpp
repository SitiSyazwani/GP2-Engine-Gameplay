/**
 * @file EditorMenuBar.cpp
 * @author Siti Syazwani (100%)
 * @brief Implementation of editor menu bar with event-driven actions
 *
 * Renders the main menu bar and publishes editor events through the EventSystem
 * when users interact with menu items. This maintains loose coupling between
 * UI and application logic.
 */

#include "EditorMenuBar.hpp"
#include "../Panels/ContentEditorUI.hpp"
#include "../Panels/AssetBrowser.hpp"
#include "../Panels/ConsolePanel.hpp"
#include "../Panels/ControlsPanel.hpp"
#include "../Panels/SpriteEditor.hpp"
#include "../Core/EditorEvents.hpp"
#include <Core/EventSystem.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

void EditorMenuBar::Initialize(ContentEditorUI* editorUI, AssetBrowser* assetBrowser, ConsolePanel* console, ControlsPanel* controlsPanel,SpriteEditor* spriteEditor,  bool* showCollisionBoxes) {
    // Store references to UI components for panel visibility toggles
    m_editorUI = editorUI;
    m_assetBrowser = assetBrowser;
    m_console = console;
    m_controlsPanel = controlsPanel;
    m_showCollisionBoxes = showCollisionBoxes;
    m_spriteEditor = spriteEditor;
}

void EditorMenuBar::RenderWithDockspace(EditorState currentState) {
    // Setup fullscreen dockspace window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Make window fill entire viewport
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Style the window to be invisible background
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    // Create dockspace for panel docking
    ImGuiID dockspaceID = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    // Render menu bar at top of dockspace
    Render(currentState);

    ImGui::End();
}

void EditorMenuBar::Render(EditorState currentState) {
    if (ImGui::BeginMenuBar()) {
        // Render menu dropdowns
        RenderFileMenu();
        RenderViewMenu();

        // Display current scene filename in menu bar
        ImGui::Separator();
        if (m_currentScenePath && !m_currentScenePath->empty()) {
            std::string filename = std::filesystem::path(*m_currentScenePath).filename().string();
            ImGui::Text("Current Scene:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", filename.c_str());

            // Show yellow asterisk for unsaved changes
            if (m_hasUnsavedChanges && *m_hasUnsavedChanges) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            }
        } else {
            // No scene loaded yet
            ImGui::Text("Current Scene:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Unsaved)");
        }

        // Display current editor mode (EDIT or PLAY)
        ImGui::Separator();
        ImGui::Text("Mode:");
        ImGui::SameLine();
        if (currentState == EditorState::EDIT) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.3f, 1.0f), "EDIT");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "PLAY");
        }
        ImGui::Separator();

        // Render centered Play/Stop button
        RenderPlayStopButton(currentState);
        ImGui::EndMenuBar();
    }
}

void EditorMenuBar::RenderFileMenu() {
    if (ImGui::BeginMenu("File")) {
        // Create new empty scene
        if (ImGui::MenuItem("New Scene")) {
            GP2Engine::EventSystem::Publish(EditorNewSceneEvent{});
        }

        // Load existing scene from list
        if (ImGui::BeginMenu("Load Scene")) {
            if (m_availableScenes.empty()) {
                ImGui::MenuItem("(No scenes found)", nullptr, false, false);
            } else {
                // Display all available scene files
                for (const auto& scenePath : m_availableScenes) {
                    std::string filename = std::filesystem::path(scenePath).filename().string();
                    if (ImGui::MenuItem(filename.c_str())) {
                        GP2Engine::EventSystem::Publish(EditorLoadSceneEvent{scenePath});
                    }
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        // Save current scene (disabled if no scene loaded)
        if (ImGui::MenuItem("Save", nullptr, false, m_canSave)) {
            GP2Engine::EventSystem::Publish(EditorSaveSceneEvent{});
        }

        // Save to new file (always enabled)
        if (ImGui::MenuItem("Save As...")) {
            GP2Engine::EventSystem::Publish(EditorSaveSceneAsEvent{});
        }

        ImGui::Separator();

        // Exit editor (with ESC shortcut hint)
        if (ImGui::MenuItem("Exit", "ESC")) {
            GP2Engine::EventSystem::Publish(EditorExitEvent{});
        }

        ImGui::EndMenu();
    }
}

void EditorMenuBar::RenderViewMenu() {
    if (ImGui::BeginMenu("View")) {
        // Toggle editor UI panels
        if (m_editorUI) {
            bool showHierarchy = m_editorUI->GetShowHierarchy();
            if (ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy)) {
                m_editorUI->SetShowHierarchy(showHierarchy);
            }

            bool showProperties = m_editorUI->GetShowProperties();
            if (ImGui::MenuItem("Properties", nullptr, &showProperties)) {
                m_editorUI->SetShowProperties(showProperties);
            }
        }

        // Toggle asset browser panel
        if (m_assetBrowser) {
            bool showAssetBrowser = m_assetBrowser->IsVisible();
            if (ImGui::MenuItem("Asset Browser", nullptr, &showAssetBrowser)) {
                m_assetBrowser->SetVisible(showAssetBrowser);
            }
        }

        // Toggle console panel
        if (m_console) {
            bool showConsole = m_console->IsVisible();
            if (ImGui::MenuItem("Console", nullptr, &showConsole)) {
                m_console->SetVisible(showConsole);
            }
        }

        // Toggle controls help panel
        if (m_controlsPanel) {
            bool showControls = m_controlsPanel->IsVisible();
            if (ImGui::MenuItem("Controls", nullptr, &showControls)) {
                m_controlsPanel->SetVisible(showControls);
            }
        }

        // Toggle sprite editor panel
        if (m_spriteEditor) {
            bool showSpriteEditor = m_spriteEditor->IsVisible();
            if (ImGui::MenuItem("Sprite Editor", nullptr, &showSpriteEditor)) {
                m_spriteEditor->SetVisible(showSpriteEditor);
            }
        }

        ImGui::Separator();

        // Toggle debug collision box visualization
        if (m_showCollisionBoxes) {
            ImGui::MenuItem("Collision Boxes", nullptr, m_showCollisionBoxes);
        }

        ImGui::EndMenu();
    }
}

void EditorMenuBar::RenderPlayStopButton(EditorState currentState) {
    // Calculate center position for button in menu bar
    float menuBarWidth = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
    float buttonWidth = 100.0f;
    float centerPos = (menuBarWidth * 0.5f) - (buttonWidth * 0.5f);
    ImGui::SetCursorPosX(centerPos);

    if (currentState == EditorState::EDIT) {
        // Green Play button when in EDIT mode
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("Play", ImVec2(buttonWidth, 0))) {
            GP2Engine::EventSystem::Publish(EditorPlayEvent{});
        }
        ImGui::PopStyleColor(3);
    } else {
        // Red Stop button when in PLAY mode
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Stop", ImVec2(buttonWidth, 0))) {
            GP2Engine::EventSystem::Publish(EditorStopEvent{});
        }
        ImGui::PopStyleColor(3);
    }
}
