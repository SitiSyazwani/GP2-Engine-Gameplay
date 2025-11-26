// SpriteEditor.cpp

#include "SpriteEditor.hpp"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include "../Engine/Engine.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void SpriteEditor::Render() {
    if (!m_Visible) return;

    ImGui::Begin("Spritesheet Editor", &m_Visible);

    ImGui::Text("--- Spritesheet Definition ---");
    ImGui::Separator();

    // 1. Drag & Drop Target
    HandleDragDrop();

    ImGui::Separator();

    // 2. Grid Inputs (Only editable if a texture is loaded)
    ImGui::Text("Grid Definition:");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.30f);

    // Clamp inputs to a minimum of 1
    // Reset save status if the input value changes
    if (ImGui::InputInt("Rows", &m_Rows)) {
        m_JustSaved = false; // Reset indicator on edit
    }
    m_Rows = glm::max(1, m_Rows);

    ImGui::SameLine();
    // Reset save status if the input value changes
    if (ImGui::InputInt("Columns", &m_Columns)) {
        m_JustSaved = false; // Reset indicator on edit
    }
    m_Columns = glm::max(1, m_Columns);

    ImGui::PopItemWidth();

    ImGui::Separator();

    // 3. Preview (Must come before buttons for correct draw list offset)
    RenderTexturePreview();

    ImGui::Separator();

    // 4. Output/Save Controls
    ImGui::Text("Output:");
    // This ImGui call is now correct because m_OutputMetaFileName is a char[]
    if (ImGui::InputText(
        "Meta File Name",
        m_OutputMetaFileName, // The char array
        MAX_FILENAME_LENGTH   // The size of the char array (assuming defined in .hpp)
    )) {
        m_JustSaved = false; // Reset indicator on edit
    }

    // The Split button can be integrated into the Save logic, as saving implies splitting
    // Use the full available width for simplicity
    if (ImGui::Button("Save Definition (Split)", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        SaveSpriteSheetData();
    }

    // NEW & SIMPLIFIED: Display the "Saved!" indicator immediately below the button
    if (m_JustSaved) {
        // Center the text using available width
        float textWidth = ImGui::CalcTextSize("Saved!").x;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f); // Center alignment

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Saved!"); // Green text
    }


    // Display calculated tile size (keeping this at the bottom for informational purposes)
    if (m_SpritesheetTexture && m_Rows > 0 && m_Columns > 0) {
        int tileW = m_SpritesheetTexture->GetWidth() / m_Columns;
        int tileH = m_SpritesheetTexture->GetHeight() / m_Rows;
        ImGui::Text("Calculated Tile Size: %dx%d pixels", tileW, tileH);
    }


    ImGui::End();
}

void SpriteEditor::HandleDragDrop() {
    ImGui::BeginChild("##DropTarget", ImVec2(ImGui::GetContentRegionAvail().x, 40), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextWrapped("Drop Spritesheet Texture Here");

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
            const char* dragged_path = (const char*)payload->Data;
            std::string filePath = dragged_path;

            // Only load if it's a new or valid texture
            if (filePath != m_SpritesheetPath) {
                // Assuming ResourceManager is correctly integrated through Engine.hpp
                m_SpritesheetTexture = GP2Engine::ResourceManager::GetInstance().LoadTexture(filePath);

                if (m_SpritesheetTexture && m_SpritesheetTexture->IsValid()) {
                    m_SpritesheetPath = filePath;

                    std::string stem = fs::path(filePath).stem().string();
                    strncpy_s(m_OutputMetaFileName, stem.c_str(), MAX_FILENAME_LENGTH - 1);
                    m_OutputMetaFileName[MAX_FILENAME_LENGTH - 1] = '\0'; // Ensure null termination

                    // Basic guess for initial split (e.g., 8x8 is common)
                    m_Rows = 8;
                    m_Columns = 8;
                    std::cout << "Loaded spritesheet: " << filePath << std::endl;
                }
                else {
                    std::cerr << "Failed to load spritesheet: " << filePath << std::endl;
                    m_SpritesheetTexture = nullptr;
                    m_SpritesheetPath = "";
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::EndChild();
}

void SpriteEditor::RenderTexturePreview() {
    if (!m_SpritesheetTexture || !m_SpritesheetTexture->IsValid()) {
        ImGui::Text("No texture loaded for preview.");
        return;
    }

    float width = (float)m_SpritesheetTexture->GetWidth();
    float height = (float)m_SpritesheetTexture->GetHeight();

    // Calculate the size to fit the panel
    float previewWidth = ImGui::GetContentRegionAvail().x;
    float aspectRatio = height / width;
    ImVec2 imageSize = ImVec2(previewWidth, previewWidth * aspectRatio);

    // Render the full texture
    ImGui::Text("Preview (%dx%d):", (int)width, (int)height);
    ImGui::Image((ImTextureID)(intptr_t)m_SpritesheetTexture->GetTextureID(), imageSize);

    // --- Overlay Grid ---
    if (m_Rows > 0 && m_Columns > 0) {

        // 1. Get the screen position of the image (top-left corner)
        // ImGui::GetItemRectMin() returns the top-left corner of the last rendered item (the image).
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 imageScreenPos = ImGui::GetItemRectMin();

        // Use the size calculated earlier
        // We also need the bottom-right corner for drawing lines
        ImVec2 imageScreenMax = ImGui::GetItemRectMax();

        float tileW = imageSize.x / m_Columns;
        float tileH = imageSize.y / m_Rows;

        // Draw vertical lines
        for (int c = 1; c < m_Columns; ++c) {
            drawList->AddLine(
                ImVec2(imageScreenPos.x + c * tileW, imageScreenPos.y),
                ImVec2(imageScreenPos.x + c * tileW, imageScreenMax.y), // Use Max Y
                IM_COL32(255, 0, 0, 255) // Red lines
            );
        }

        // Draw horizontal lines
        for (int r = 1; r < m_Rows; ++r) {
            drawList->AddLine(
                ImVec2(imageScreenPos.x, imageScreenPos.y + r * tileH),
                ImVec2(imageScreenMax.x, imageScreenPos.y + r * tileH), // Use Max X
                IM_COL32(255, 0, 0, 255) // Red lines
            );
        }
    }
}

void SpriteEditor::SaveSpriteSheetData() {
    // 1. Validation: Ensure a valid texture is loaded and grid is defined.
    // NOTE: Removed the check for m_OutputMetaFileName[0] since we no longer rely on it.
    if (!m_SpritesheetTexture || !m_SpritesheetTexture->IsValid() || m_Rows <= 0 || m_Columns <= 0) {
        std::cerr << "Save failed: Invalid spritesheet or zero rows/columns." << std::endl;
        return;
    }

    // 2. Calculate actual pixel dimensions
    int tileWidth = m_SpritesheetTexture->GetWidth() / m_Columns;
    int tileHeight = m_SpritesheetTexture->GetHeight() / m_Rows;
    // ... (Warning check remains the same) ...

    // 3. Construct the Correct Meta File Path
    // The Asset Browser expects: [Texture_Path].meta.json
    std::string metaFilePath = fs::path(m_SpritesheetPath).string() + ".meta.json"; // <--- THIS IS THE FIX

    std::ofstream outfile(metaFilePath);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file for saving meta data: " << metaFilePath << std::endl;
        return;
    }

    // 4. Write JSON Data
    // The "texture" key should still contain just the filename, as they are in the same folder.
    outfile << "{\n";
    outfile << "  \"texture\": \"" << fs::path(m_SpritesheetPath).filename().string() << "\",\n";
    outfile << "  \"rows\": " << m_Rows << ",\n";
    outfile << "  \"columns\": " << m_Columns << ",\n";
    outfile << "  \"tile_width\": " << tileWidth << ",\n";
    outfile << "  \"tile_height\": " << tileHeight << "\n";
    outfile << "}\n";

    outfile.close();
    std::cout << "Spritesheet meta data saved to: " << metaFilePath << std::endl;

    m_JustSaved = true;
}

SpriteMetadata SpriteEditor::LoadSpriteMetadata(const std::string& metaPath) {
    SpriteMetadata meta{};
    std::ifstream file(metaPath);

    if (!file.is_open()) {
        std::cerr << "Failed to open spritesheet meta file: " << metaPath << std::endl;
        return meta;
    }

    try {
        json j;
        file >> j;

        // 1. Get the path as written in the JSON
        std::string texturePathInJson = j.value("texture", "");

        // 2. Resolve the path relative to the meta file location
        if (!texturePathInJson.empty()) {
            fs::path metaDir = fs::path(metaPath).parent_path();
            fs::path fullTexturePath = metaDir / texturePathInJson;

            // Store the resolved path for the Resource Manager
            meta.texturePath = fullTexturePath.string();
        }
        else {
            // Set the extracted path if resolution fails or texture key is missing
            meta.texturePath = texturePathInJson;
        }

        // 3. Extract other values
        meta.rows = j.value("rows", 0);
        meta.columns = j.value("columns", 0);
        meta.tileWidth = j.value("tile_width", 0);
        meta.tileHeight = j.value("tile_height", 0);

    }
    catch (const std::exception& e) {
        std::cerr << "Error loading metadata from " << metaPath << ": " << e.what() << std::endl;
        return SpriteMetadata{};
    }

    return meta;
}
