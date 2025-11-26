/**
 * @file TexturePicker.hpp
 * @author Adi (100%)
 * @brief Reusable texture picker widget for editor UI
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>

/**
 * @brief Texture selection widget
 *
 * Provides a reusable UI widget for scanning, displaying, and selecting
 * textures from a specified directory. Handles refresh and caching.
 */
class TexturePicker {
public:
    TexturePicker() = default;
    ~TexturePicker() = default;

    /**
     * @brief Initialize the texture picker
     * @param texturesPath Path to textures directory
     */
    void Initialize(const std::string& texturesPath);

    /**
     * @brief Render the texture picker UI
     * @param outSelectedPath Output: Selected texture path (if selection made)
     * @return True if a texture was selected this frame, false otherwise
     */
    bool Render(std::string& outSelectedPath);

    /**
     * @brief Refresh the texture list by rescanning directory
     */
    void Refresh();

    /**
     * @brief Check if textures were found
     * @return True if texture list is not empty
     */
    bool HasTextures() const { return !m_availableTextures.empty(); }

private:
    std::string m_texturesPath;
    std::vector<std::string> m_availableTextures;
    int m_selectedIndex = 0;
    bool m_scanned = false;

    /**
     * @brief Scan textures directory and populate list
     */
    void ScanTextures();
};
