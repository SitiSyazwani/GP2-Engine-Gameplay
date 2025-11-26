/**
 * @file EditorDialogs.cpp
 * @author Adi (100%)
 * @brief Implementation of modal dialogs for editor operations
 *
 * Handles rendering and user interaction for Save As and Unsaved Changes dialogs.
 * Publishes events through EventSystem when users make decisions.
 */

#include "EditorDialogs.hpp"
#include "../Core/EditorEvents.hpp"
#include <Core/EventSystem.hpp>
#include <imgui.h>

void EditorDialogs::RenderSaveAsDialog() {
    // Only render if dialog is active
    if (!m_showSaveAsDialog) return;

    // Request ImGui to open the modal popup 
    ImGui::OpenPopup("Save As");

    // Center the dialog on the main viewport
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(SAVE_AS_DIALOG_WIDTH, SAVE_AS_DIALOG_HEIGHT));

    // Begin modal popup
    // The &m_showSaveAsDialog allows user to close via X button
    if (ImGui::BeginPopupModal("Save As", &m_showSaveAsDialog, ImGuiWindowFlags_NoResize)) {
        // Show destination directory path
        ImGui::Text("Save scene to: %s", SCENES_PATH);
        ImGui::Separator();

        // Text input for filename entry
        ImGui::InputText("Filename", m_saveAsFileName, sizeof(m_saveAsFileName));
        ImGui::TextDisabled("(.json extension will be added automatically)");

        ImGui::Separator();

        // Save button - publishes event with filename and closes dialog
        if (ImGui::Button("Save", ImVec2(STANDARD_BUTTON_WIDTH, 0))) {
            GP2Engine::EventSystem::Publish(EditorSaveAsConfirmEvent{m_saveAsFileName});
            m_showSaveAsDialog = false;
        }

        ImGui::SameLine();

        // Cancel button - just closes dialog without action
        if (ImGui::Button("Cancel", ImVec2(STANDARD_BUTTON_WIDTH, 0))) {
            m_showSaveAsDialog = false;
        }

        ImGui::EndPopup();
    }
}

void EditorDialogs::RenderUnsavedChangesWarning() {
    // Only render if warning is active
    if (!m_showUnsavedWarning) return;

    // Request ImGui to open the modal popup
    ImGui::OpenPopup("Unsaved Changes");

    // Center the dialog on the main viewport
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(UNSAVED_WARNING_WIDTH, UNSAVED_WARNING_HEIGHT));

    // Begin modal popup (blocks interaction with other windows)
    if (ImGui::BeginPopupModal("Unsaved Changes", &m_showUnsavedWarning, ImGuiWindowFlags_NoResize)) {
        // Display warning message to user
        ImGui::TextWrapped("You have unsaved changes. What would you like to do?");
        ImGui::Separator();
        ImGui::Spacing();

        // "Save and Continue" button - saves current scene then proceeds with pending action
        if (ImGui::Button("Save and Continue", ImVec2(WIDE_BUTTON_WIDTH, 0))) {
            // First save the current scene
            GP2Engine::EventSystem::Publish(EditorSaveAndContinueEvent{});

            // Then proceed with the pending action (NewScene or Exit)
            // This is published after save completes to ensure changes aren't lost
            if (m_pendingAction == UnsavedAction::NewScene) {
                GP2Engine::EventSystem::Publish(EditorDiscardChangesEvent{EditorDiscardChangesEvent::Action::NewScene});
            } else if (m_pendingAction == UnsavedAction::Exit) {
                GP2Engine::EventSystem::Publish(EditorDiscardChangesEvent{EditorDiscardChangesEvent::Action::Exit});
            }

            // Close dialog and reset state
            m_showUnsavedWarning = false;
            m_pendingAction = UnsavedAction::None;
        }

        ImGui::SameLine();

        // "Discard Changes" button - proceeds with action without saving
        if (ImGui::Button("Discard Changes", ImVec2(WIDE_BUTTON_WIDTH, 0))) {
            // Publish discard event to continue with pending action
            // This will cause SceneManager to load new scene or exit without saving
            if (m_pendingAction == UnsavedAction::NewScene) {
                GP2Engine::EventSystem::Publish(EditorDiscardChangesEvent{EditorDiscardChangesEvent::Action::NewScene});
            } else if (m_pendingAction == UnsavedAction::Exit) {
                GP2Engine::EventSystem::Publish(EditorDiscardChangesEvent{EditorDiscardChangesEvent::Action::Exit});
            }

            // Close dialog and reset state
            m_showUnsavedWarning = false;
            m_pendingAction = UnsavedAction::None;
        }

        ImGui::Spacing();

        // "Cancel" button - centered, cancels the entire operation
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - STANDARD_BUTTON_WIDTH) * 0.5f);
        if (ImGui::Button("Cancel", ImVec2(STANDARD_BUTTON_WIDTH, 0))) {
            // User cancelled - donesn't save, just close dialog
            m_pendingAction = UnsavedAction::None;
            m_showUnsavedWarning = false;
        }

        ImGui::EndPopup();
    }
}
