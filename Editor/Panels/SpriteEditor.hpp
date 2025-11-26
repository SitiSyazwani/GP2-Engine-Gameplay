#pragma once

#include <string>
#include <memory>
#include "../Engine/Engine.hpp"
#include <glm/glm.hpp>
#include "nlohmann/json.hpp"

// data structure needed to read meta file
struct SpriteMetadata {
    std::string texturePath;
    int rows;
    int columns;
    int tileWidth;
    int tileHeight;
    std::shared_ptr<GP2Engine::Texture> cachedTexture = nullptr;
};

class SpriteEditor {
public:
    SpriteEditor() = default;

    void Render();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }
    void Toggle() { m_Visible = !m_Visible; }

    static SpriteMetadata LoadSpriteMetadata(const std::string& metaPath);

private:
    bool m_Visible = false;

    bool m_JustSaved = false;

    // Spritesheet data
    std::shared_ptr<GP2Engine::Texture> m_SpritesheetTexture = nullptr;
    std::string m_SpritesheetPath;

    // Grid definition inputs
    int m_Rows = 1;
    int m_Columns = 1;

    // Output file path
    static constexpr int MAX_FILENAME_LENGTH = 128;
    char m_OutputMetaFileName[MAX_FILENAME_LENGTH] = "new_spritesheet";

    // Helper functions
    void HandleDragDrop();
    void RenderTexturePreview();
    void SaveSpriteSheetData();

};