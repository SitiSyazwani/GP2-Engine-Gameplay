/**
 * @file TexturePicker.cpp
 * @author Adi (100%)
 * @brief Implementation of texture picker widget
 *
 * Scans a directory for texture files and provides a dropdown selector for
 * assigning textures to SpriteComponents thru the Properties panel.
 */

#include "TexturePicker.hpp"
#include <filesystem>

void TexturePicker::Initialize(const std::string& texturesPath) {
    // Store textures directory path for scanning
    m_texturesPath = texturesPath;

    // Scan directory to populate available textures list
    Refresh();
}

bool TexturePicker::Render(std::string& outSelectedPath) {
    bool textureSelected = false;

    // Show refresh button
    if (!m_scanned || ImGui::Button("Refresh Textures")) {
        Refresh();
    }

    // Handle empty texture list
    if (m_availableTextures.empty()) {
        ImGui::TextDisabled("(No textures found)");
        return false;
    }

    ImGui::Text("Assign Texture:");

    // Texture file dropdown selector
    if (ImGui::BeginCombo("##TextureSelect", m_availableTextures[m_selectedIndex].c_str())) {
        for (int i = 0; i < m_availableTextures.size(); i++) {
            bool isSelected = (m_selectedIndex == i);
            if (ImGui::Selectable(m_availableTextures[i].c_str(), isSelected)) {
                m_selectedIndex = i;
            }
            // Auto-scroll to selected item when dropdown opens
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Apply button - returns selected texture path
    if (ImGui::Button("Apply Texture")) {
        outSelectedPath = m_availableTextures[m_selectedIndex];
        textureSelected = true;
    }

    return textureSelected;
}

void TexturePicker::Refresh() {
    // Rescan textures directory and mark as scanned
    ScanTextures();
    m_scanned = true;
}

void TexturePicker::ScanTextures() {
    m_availableTextures.clear();

    // Validate textures directory exists
    if (!std::filesystem::exists(m_texturesPath)) {
        LOG_ERROR("Textures directory does not exist: " + m_texturesPath);
        return;
    }

    try {
        // Iterate through directory and find supported image files
        for (const auto& entry : std::filesystem::directory_iterator(m_texturesPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Check for common image formats
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                    // Store relative path from assets folder for ResourceManager
                    std::string relativePath = "textures/" + entry.path().filename().string();
                    m_availableTextures.push_back(relativePath);
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error scanning textures: " + std::string(e.what()));
    }

    // Reset selection index if out of bounds after rescan
    if (m_selectedIndex >= m_availableTextures.size()) {
        m_selectedIndex = 0;
    }
}
