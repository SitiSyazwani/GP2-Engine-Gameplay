/**
 * @file EditorDialogs.hpp
 * @author Adi (100%)
 * @brief Modal dialogs for editor operations (Save As, Unsaved Changes warnings)
 *
 * Manages ImGui modal popups for user confirmation and input. Uses the engine's
 * EventSystem to publish user actions without direct coupling to scene management.
 */

#pragma once

#include <string>

/**
 * @brief Modal dialog manager for the editor
 *
 * Handles common editor dialogs like Save As and unsaved changes warnings.
 * Publishes events through EventSystem when users confirm actions.
 *
 * Events published:
 * - EditorSaveAsConfirmEvent (when user confirms Save As with filename)
 * - EditorSaveAndContinueEvent (when user chooses to save before proceeding)
 * - EditorDiscardChangesEvent (when user discards unsaved changes)
 */
class EditorDialogs {
public:
    enum class UnsavedAction { None, NewScene, Exit };

    EditorDialogs() = default;
    ~EditorDialogs() = default;

    /**
     * @brief Render the Save As dialog modal
     *
     * Displays a text input for entering scene filename and Save/Cancel buttons.
     * Publishes EditorSaveAsConfirmEvent on confirmation.
     */
    void RenderSaveAsDialog();

    /**
     * @brief Render the Unsaved Changes warning modal
     *
     * Displays warning about unsaved changes with options to save, discard, or cancel.
     * Publishes appropriate events based on user choice.
     */
    void RenderUnsavedChangesWarning();

    /**
     * @brief Show the Save As dialog
     */
    void ShowSaveAsDialog() { m_showSaveAsDialog = true; }

    /**
     * @brief Show unsaved changes warning
     * @param action Action that triggered the warning (NewScene or Exit)
     */
    void ShowUnsavedWarning(UnsavedAction action) {
        m_pendingAction = action;
        m_showUnsavedWarning = true;
    }

    /**
     * @brief Check if Save As dialog is currently open
     */
    bool IsSaveAsDialogOpen() const { return m_showSaveAsDialog; }

    /**
     * @brief Check if Unsaved Warning dialog is currently open
     */
    bool IsUnsavedWarningOpen() const { return m_showUnsavedWarning; }

private:
    static constexpr const char* SCENES_PATH = "../../Sandbox/assets/scenes";

    // Dialog dimensions
    static constexpr float SAVE_AS_DIALOG_WIDTH = 400.0f;
    static constexpr float SAVE_AS_DIALOG_HEIGHT = 120.0f;
    static constexpr float UNSAVED_WARNING_WIDTH = 400.0f;
    static constexpr float UNSAVED_WARNING_HEIGHT = 140.0f;

    // Button dimensions
    static constexpr float STANDARD_BUTTON_WIDTH = 120.0f;
    static constexpr float WIDE_BUTTON_WIDTH = 180.0f;

    bool m_showSaveAsDialog = false;
    bool m_showUnsavedWarning = false;
    char m_saveAsFileName[256] = "new_scene.json";
    UnsavedAction m_pendingAction = UnsavedAction::None;
};
