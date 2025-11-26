/**
 * @file FontPicker.cpp
 * @author Siti Syazwani (100%)
 * @brief Implementation of font picker widget
 *
 * Scans a directory for font files and provides a dropdown selector with
 * size input for assigning fonts to TextComponents via the Properties panel.
 */

#include "FontPicker.hpp"
#include <filesystem>
#include <algorithm>

void FontPicker::Initialize(const std::string& fontsPath, int defaultSize) {
    // Store fonts directory path and initial size
    m_fontsPath = fontsPath;
    m_fontSize = defaultSize;

    // Scan directory to populate available fonts list
    Refresh();
}

bool FontPicker::Render(std::string& outSelectedPath, int& outFontSize) {
    bool fontSelected = false;

    // Show refresh button 
    if (!m_scanned || ImGui::Button("Refresh Fonts")) {
        Refresh();
    }

    // Handle empty font list
    if (m_availableFonts.empty()) {
        ImGui::TextDisabled("(No fonts found)");
        return false;
    }

    ImGui::Text("Change Font:");

    // Validate selected index is within bounds
    if (m_selectedIndex >= m_availableFonts.size()) {
        m_selectedIndex = 0;
    }

    // Font file dropdown selector
    if (ImGui::BeginCombo("##FontSelect", m_availableFonts[m_selectedIndex].c_str())) {
        for (int i = 0; i < m_availableFonts.size(); i++) {
            bool isSelected = (m_selectedIndex == i);
            if (ImGui::Selectable(m_availableFonts[i].c_str(), isSelected)) {
                m_selectedIndex = i;
            }
            // Auto-scroll to selected item when dropdown opens
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Font size input 
    ImGui::PushItemWidth(100);
    ImGui::InputInt("Font Size", &m_fontSize, 1, 10);
    SetFontSize(m_fontSize);  // Clamp to valid range (8-256)
    ImGui::PopItemWidth();

    // Apply button - returns selected font and size
    if (ImGui::Button("Apply Font")) {
        outSelectedPath = m_availableFonts[m_selectedIndex];
        outFontSize = m_fontSize;
        fontSelected = true;
    }

    return fontSelected;
}

void FontPicker::Refresh() {
    // Rescan fonts directory and mark as scanned
    ScanFonts();
    m_scanned = true;
}

void FontPicker::SetFontSize(int size) {
    // Clamp font size to valid range (8-256 pixels)
    m_fontSize = std::clamp(size, MIN_FONT_SIZE, MAX_FONT_SIZE);
}

void FontPicker::ScanFonts() {
    m_availableFonts.clear();

    // Validate fonts directory exists
    if (!std::filesystem::exists(m_fontsPath)) {
        LOG_ERROR("Fonts directory does not exist: " + m_fontsPath);
        return;
    }

    try {
        // Iterate through directory and find .ttf and .otf font files
        for (const auto& entry : std::filesystem::directory_iterator(m_fontsPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".ttf" || ext == ".otf") {
                    // Store full path for ResourceManager to load
                    m_availableFonts.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error scanning fonts: " + std::string(e.what()));
    }

    // Reset selection index if out of bounds after rescan
    if (m_selectedIndex >= m_availableFonts.size()) {
        m_selectedIndex = 0;
    }
}
