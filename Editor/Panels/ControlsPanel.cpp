/**
 * @file ControlsPanel.cpp
 * @author Fauzan (100%)
 * @brief Implementation of editor controls help panel
 *
 * Renders collapsible sections with editor shortcuts and game controls.
 * Helps users discover and remember editor functionality.
 */

#include "ControlsPanel.hpp"

void ControlsPanel::Render() {
    if (!m_visible) return;

    ImGui::Begin("Controls", &m_visible);

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Middle Mouse - Pan camera");
        ImGui::Text("Q / Z - Zoom in/out");
    }

    if (ImGui::CollapsingHeader("Entity Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Left Click - Select entity");
        ImGui::Text("Left Drag - Move entity");
    }

    if (ImGui::CollapsingHeader("Entity Operations")) {
        ImGui::Text("Hierarchy - Create/Copy/Delete");
        ImGui::Text("Properties - Edit components");
        ImGui::Text("Drag texture - Create sprite");
    }

    if (ImGui::CollapsingHeader("File Operations")) {
        ImGui::Text("File > New - New scene");
        ImGui::Text("File > Load - Load scene");
        ImGui::Text("File > Save - Save scene");
    }

    if (ImGui::CollapsingHeader("Play Mode")) {
        ImGui::Text("Play - Test scene");
        ImGui::Text("Stop - Return to edit");
    }

    if (ImGui::CollapsingHeader("Game Controls (Play Mode)")) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Available in Play Mode:");
        ImGui::Separator();
        ImGui::Text("WASD - Move character");
        ImGui::Text("Shift - Sprint");
        ImGui::Text("Space - Test sound");
        ImGui::Text("Left Click - Action");
    }

    if (ImGui::CollapsingHeader("View Options")) {
        ImGui::Text("View > Collision Boxes");
        ImGui::Text("View > Panels - Toggle UI");
    }

    ImGui::End();
}
