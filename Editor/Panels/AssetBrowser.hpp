/**
 * @file AssetBrowser.hpp
 * @author Rifqah (100%)
 * @brief Asset browser for viewing and managing project assets
 *
 * Displays textures in a grid with thumbnail previews. Supports drag and drop
 * to assign textures to SpriteComponents and texture import functionality.
 * Currently shows just texture list.
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>
#include <filesystem>
#include <unordered_map>
#include "SpriteEditor.hpp"

struct AssetItem {
    std::string name;
    std::string path;
    std::filesystem::file_time_type lastModified;
    enum class Type {
        Unknown,
        Texture,
        Font,
        Audio,
        Scene,
        Spritesheet,
        Folder
    } type;

    // Thumbnail for textures
    unsigned int thumbnailTextureID = 0;
    bool thumbnailLoaded = false;

    bool isExpanded = false;
};

class AssetBrowser {
public:
    AssetBrowser() = default;
    ~AssetBrowser();

    /**
     * @brief Initialize the asset browser
     * @param assetsPath Root path to assets folder (e.g., "../../Sandbox/assets")
     */
    void Initialize(const std::string& assetsPath);

    /**
     * @brief Render the asset browser panel
     */
    void Render();

    /**
     * @brief Import a texture file from file system
     * @param sourcePath Path to source texture file
     * @return Path to imported file in assets folder, or empty string on failure
     */
    std::string ImportTexture(const std::string& sourcePath);

    std::string ImportAudio(const std::string& sourcePath);

    // === PANEL VISIBILITY ===
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

private:
    bool m_isVisible = true;
    std::string m_assetsRootPath = "../../Sandbox/assets";
    std::string m_currentDirectory;

    std::vector<AssetItem> m_currentItems;
    AssetItem* m_selectedItem = nullptr;

    // Texture thumbnails cache (managed by ResourceManager via shared_ptr)
    std::unordered_map<std::string, std::shared_ptr<GP2Engine::Texture>> m_thumbnailCache;
    // Cache for parsed spritesheet metadata
    std::unordered_map<std::string, SpriteMetadata> m_spritesheetCache;

    // Icon sizes
    float m_thumbnailSize = 64.0f;
    float m_padding = 16.0f;

    /**
     * @brief Scan directory and populate items
     */
    void ScanDirectory(const std::string& directory);

    /**
     * @brief Determine file type from extension
     */
    AssetItem::Type GetFileType(const std::string& extension, const std::string& filename);

    /**
     * @brief Load texture thumbnail via ResourceManager
     */
    void LoadThumbnail(AssetItem& item);

    /**
     * @brief Render asset grid with thumbnails and drag-drop support
     */
    void RenderAssetGrid();

    /**
     * @brief Render single asset item
     */
    void RenderAssetItem(AssetItem& item);

    /**
     * @brief Get icon color for file type
     */
    ImVec4 GetIconColor(AssetItem::Type type);

    /**
     * @brief Cleanup thumbnails
     */
    void CleanupThumbnails();


    /**
     * @brief Renders a tiled preview if a .meta.json file is found for the texture.
     */
    void RenderTiledTexturePreview(AssetItem& item, const std::string& metaPath);

    /**
    * @brief Renders the list of individual sprite tiles when the spritesheet node is expanded.
    */
    void RenderSpriteChildItems(AssetItem& item, const std::string& metaPath);

    /**
     * @brief Checks if a Texture item has an associated .meta.json file
     */
    bool HasSpriteMetadata(const AssetItem& item) const;
};

