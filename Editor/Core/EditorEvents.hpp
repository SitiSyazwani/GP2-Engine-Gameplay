/**
 * @file EditorEvents.hpp
 * @author Adi (100%)
 * @brief Event definitions for the GP2Engine Content Editor
 *
 * Defines all events used by the editor for decoupled communication between
 * UI components, scene management, and application lifecycle.
 *
 * Events are published through the engine's EventSystem and allow components
 * to communicate without direct dependencies.
 */

#pragma once

#include <string>

// ==================== MENU EVENTS ====================

/**
 * @brief Event published when user clicks "New Scene" in menu
 */
struct EditorNewSceneEvent {};

/**
 * @brief Event published when user clicks "Load Scene" in menu
 */
struct EditorLoadSceneEvent {
    std::string scenePath;
};

/**
 * @brief Event published when user clicks "Save Scene" in menu
 */
struct EditorSaveSceneEvent {};

/**
 * @brief Event published when user clicks "Save Scene As..." in menu
 */
struct EditorSaveSceneAsEvent {};

/**
 * @brief Event published when user clicks "Exit" in menu
 */
struct EditorExitEvent {};

/**
 * @brief Event published when user clicks "Play" button
 */
struct EditorPlayEvent {};

/**
 * @brief Event published when user clicks "Stop" button
 */
struct EditorStopEvent {};

// ==================== DIALOG EVENTS ====================

/**
 * @brief Event published when user confirms "Save As" dialog with a filename
 */
struct EditorSaveAsConfirmEvent {
    std::string filename;
};

/**
 * @brief Event published when user chooses to save and continue in unsaved changes dialog
 */
struct EditorSaveAndContinueEvent {};

/**
 * @brief Event published when user chooses to discard changes
 */
struct EditorDiscardChangesEvent {
    enum class Action {
        NewScene,
        Exit
    };
    Action action;
};

// ==================== SCENE EVENTS ====================

/**
 * @brief Event published when scene needs to show unsaved changes warning
 */
struct EditorShowUnsavedWarningEvent {
    bool isExit;  // true if warning is for exit, false if for new scene
};

/**
 * @brief Event published when application should exit
 */
struct EditorRequestExitEvent {};

/**
 * @brief Event published when scene is successfully loaded
 */
struct EditorSceneLoadedEvent {
    std::string scenePath;
};

/**
 * @brief Event published when scene is successfully saved
 */
struct EditorSceneSavedEvent {
    std::string scenePath;
};

// ==================== PLAY MODE EVENTS ====================

/**
 * @brief Event published when play mode starts successfully
 */
struct EditorPlayModeStartedEvent {};

/**
 * @brief Event published when play mode stops
 */
struct EditorPlayModeStoppedEvent {};
