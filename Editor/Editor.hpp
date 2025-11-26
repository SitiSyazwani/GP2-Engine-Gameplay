/**
 * @file Editor.hpp
 * @author Adi (100%)
 * @brief Header that includes all core editor systems and components
 *
 * This header provides a single include point for all editor functionality.
 * Include this file in editor source files to access all editor systems without
 * needing individual includes for each component.
 *
 * Usage:
 *   #include <Editor.hpp>
 */

#pragma once

// Engine core 
#include <Engine.hpp>

// ImGui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// Editor Core
#include "Core/EditorInitializer.hpp"
#include "Core/EditorEvents.hpp"

// Editor UI
#include "UI/EditorViewport.hpp"
#include "UI/EditorMenuBar.hpp"

// Editor Panels
#include "Panels/ContentEditorUI.hpp"
#include "Panels/AssetBrowser.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/ControlsPanel.hpp"
#include "Panels/TexturePicker.hpp"
#include "Panels/FontPicker.hpp"
#include "Panels/SpriteEditor.hpp"
// Scene Management
#include "Scene/SceneManager.hpp"

// Play Mode
#include "PlayMode/PlayModeManager.hpp"

// Input & Debug
#include "Input/EditorInputHandler.hpp"
#include "DebugRender/EditorDebugRenderer.hpp"

// Dialogs
#include "Dialogs/EditorDialogs.hpp"
#include "Dialogs/FileDialog.hpp"
