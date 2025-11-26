/**
 * @file EditorMenuBar.hpp
 * @author Sit Syazwani (100%)
 * @brief Editor menu bar with ImGui docking support and event-driven actions
 *
 * Provides the main menu bar for the editor including File, View menus and
 * Play/Stop controls. Uses the engine's EventSystem to publish user actions,
 * maintaining loose coupling with other editor components.
 */

#pragma once

#include <string>
#include <vector>

class ContentEditorUI;
class AssetBrowser;
class ConsolePanel;
class ControlsPanel;
class TileEditor;
class SpriteEditor;

/**
 * @brief Main menu bar for the editor
 *
 * Renders the top menu bar with file operations, view toggles, and play mode controls.
 * Publishes events through EventSystem when user interacts with menu items.
 *
 * Events published:
 * - EditorNewSceneEvent
 * - EditorLoadSceneEvent
 * - EditorSaveSceneEvent
 * - EditorSaveSceneAsEvent
 * - EditorExitEvent
 * - EditorPlayEvent
 * - EditorStopEvent
 */
class EditorMenuBar {
public:
    enum class EditorState { EDIT, PLAY };

    EditorMenuBar() = default;
    ~EditorMenuBar() = default;

    /**
     * @brief Initialize menu bar with UI component references
     * @param editorUI Main editor UI for toggling
     * @param assetBrowser Asset browser for toggling
     * @param console Console panel for toggling
     * @param controlsPanel Controls help panel for toggling
     * @param tileEditor Tile editor panel for toggling
     * @param showCollisionBoxes Debug visualization toggle
     */
    void Initialize(ContentEditorUI* editorUI, AssetBrowser* assetBrowser, ConsolePanel* console, ControlsPanel* controlsPanel,SpriteEditor* spriteEditor, bool* showCollisionBoxes);

    /**
     * @brief Render complete dockspace and menu bar
     * @param currentState Current editor state (EDIT or PLAY)
     */
    void RenderWithDockspace(EditorState currentState);

    /**
     * @brief Render just the menu bar
     * @param currentState Current editor state (EDIT or PLAY)
     */
    void Render(EditorState currentState);

    /**
     * @brief Update list of available scenes for Load Scene menu
     * @param scenes List of scene file paths
     */
    void SetAvailableScenes(const std::vector<std::string>& scenes) { m_availableScenes = scenes; }

    /**
     * @brief Update whether save is enabled
     * @param canSave True if a scene is loaded and can be saved
     */
    void SetCanSave(bool canSave) { m_canSave = canSave; }

    /**
     * @brief Set current scene info for display in title bar
     * @param scenePath Pointer to current scene path
     * @param unsavedChanges Pointer to unsaved changes flag
     */
    void SetSceneInfo(const std::string* scenePath, const bool* unsavedChanges) {
        m_currentScenePath = scenePath;
        m_hasUnsavedChanges = unsavedChanges;
    }

private:
    /**
     * @brief Render File menu (New, Load, Save, Exit)
     */
    void RenderFileMenu();

    /**
     * @brief Render View menu (panel toggles, debug visualization)
     */
    void RenderViewMenu();

    /**
     * @brief Render Play/Stop button in center of menu bar
     * @param currentState Current editor state to determine button appearance
     */
    void RenderPlayStopButton(EditorState currentState);

    // References to UI components for visibility toggles
    ContentEditorUI* m_editorUI = nullptr;
    AssetBrowser* m_assetBrowser = nullptr;
    ConsolePanel* m_console = nullptr;
    ControlsPanel* m_controlsPanel = nullptr;
    TileEditor* m_tileEditor = nullptr;
    SpriteEditor* m_spriteEditor = nullptr;
    bool* m_showCollisionBoxes = nullptr;

    // Scene management state
    std::vector<std::string> m_availableScenes;
    bool m_canSave = false;

    // Scene info pointers for display in menu bar
    const std::string* m_currentScenePath = nullptr;
    const bool* m_hasUnsavedChanges = nullptr;
};
