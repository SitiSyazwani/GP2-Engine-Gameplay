/**
 * @file ControlsPanel.hpp
 * @author Fauzan (100%)
 * @brief Editor controls help panel
 *
 * Displays keyboard and mouse shortcuts for using the editor, including
 * camera controls, entity operations, and game controls during play mode.
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>

class ControlsPanel {
public:
    ControlsPanel() = default;
    ~ControlsPanel() = default;

    /**
     * @brief Render the controls help panel
     */
    void Render();

    /**
     * @brief Get panel visibility
     */
    bool IsVisible() const { return m_visible; }

    /**
     * @brief Set panel visibility
     */
    void SetVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Toggle panel visibility
     */
    void Toggle() { m_visible = !m_visible; }

private:
    bool m_visible = true;  // Default to visible so ImGui can save state
};
