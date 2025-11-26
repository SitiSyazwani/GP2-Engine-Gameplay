/**
 * @file FontPicker.hpp
 * @author Siti Syazwani (100%)
 * @brief Reusable font picker widget for editor UI
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>

/**
 * @brief Font selection widget
 *
 * Provides a reusable UI widget for scanning, displaying, and selecting
 * fonts from a specified directory. Includes font size input.
 */
class FontPicker {
public:
    FontPicker() = default;
    ~FontPicker() = default;

    /**
     * @brief Initialize the font picker
     * @param fontsPath Path to fonts directory
     * @param defaultSize Default font size
     */
    void Initialize(const std::string& fontsPath, int defaultSize = 48);

    /**
     * @brief Render the font picker UI
     * @param outSelectedPath Output: Selected font path (if selection made)
     * @param outFontSize Output: Selected font size
     * @return True if a font was selected this frame, false otherwise
     */
    bool Render(std::string& outSelectedPath, int& outFontSize);

    /**
     * @brief Refresh the font list by rescanning directory
     */
    void Refresh();

    /**
     * @brief Check if fonts were found
     * @return True if font list is not empty
     */
    bool HasFonts() const { return !m_availableFonts.empty(); }

    /**
     * @brief Get current font size
     * @return Current font size value
     */
    int GetFontSize() const { return m_fontSize; }

    /**
     * @brief Set font size
     * @param size New font size (clamped to 8-256)
     */
    void SetFontSize(int size);

private:
    std::string m_fontsPath;
    std::vector<std::string> m_availableFonts;
    int m_selectedIndex = 0;
    int m_fontSize = 48;
    bool m_scanned = false;

    static constexpr int MIN_FONT_SIZE = 8;
    static constexpr int MAX_FONT_SIZE = 256;

    /**
     * @brief Scan fonts directory and populate list
     */
    void ScanFonts();
};
