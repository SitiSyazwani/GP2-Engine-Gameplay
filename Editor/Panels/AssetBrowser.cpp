/**
 * @file AssetBrowser.cpp
 * @author Rifqah (100%)
 * @brief Implementation of the asset browser
 *
 * Provides texture browsing and import functionality. Loads thumbnails thru
 * ResourceManager and supports drag-drop to SpriteComponent editors.
 * Currently displays textures only.
 */

#include "AssetBrowser.hpp"
#include "../Dialogs/FileDialog.hpp"
#include <algorithm>
#include "SpriteEditor.hpp"
#include <filesystem>

namespace fs = std::filesystem;

AssetBrowser::~AssetBrowser() {
    // Release all cached thumbnail textures on destruction
    CleanupThumbnails();
}

void AssetBrowser::Initialize(const std::string& assetsPath) {
    // Store root assets path for texture imports and scanning
    m_assetsRootPath = assetsPath;
    m_currentDirectory = m_assetsRootPath;

    // Perform initial directory scan to populate texture list
    ScanDirectory(m_currentDirectory);

    LOG_INFO("AssetBrowser initialized: " + m_assetsRootPath);
}

void AssetBrowser::Render() {
    // Skip rendering if panel is hidden
    if (!m_isVisible) return;

    ImGui::Begin("Asset Browser", &m_isVisible);

    // Import button - opens file dialog to copy external texture into assets folder
    if (ImGui::Button("Import Texture")) {
        std::string selectedFile = FileDialog::OpenFile(
            "Import Texture",
            "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0"
        );

        if (!selectedFile.empty()) {
            // Copy selected file to assets folder and refresh list
            ImportTexture(selectedFile);
        }
    }

    // Import Audio button
    ImGui::SameLine();
    if (ImGui::Button("Import Audio")) {
        std::string selectedFile = FileDialog::OpenFile(
            "Import Audio",
            "Audio Files\0*.wav;*.mp3;*.ogg;*.flac\0All Files\0*.*\0"
        );

        if (!selectedFile.empty()) {
            ImportAudio(selectedFile);
        }
    }

    ImGui::Separator();

    // Render texture grid with thumbnails and drag and drop support
    RenderAssetGrid();

    ImGui::End();
}

void AssetBrowser::ScanDirectory(const std::string& directory) {
    m_currentItems.clear();
    m_selectedItem = nullptr;

    // Validate directory exists
    if (!std::filesystem::exists(directory)) {
        LOG_ERROR("Asset directory does not exist: " + directory);
        return;
    }

    try {
        // Iterate through directory entries
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            AssetItem item;
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.lastModified = std::filesystem::last_write_time(entry);

            // Determine file type
            if (entry.is_directory()) {
                item.type = AssetItem::Type::Folder;
            } else {
                std::string extension = entry.path().extension().string();
                item.type = GetFileType(extension, item.name);
            }

            // Only show textures and Audio in list 
            if (item.type == AssetItem::Type::Texture ||
                item.type == AssetItem::Type::Audio) {
                m_currentItems.push_back(item);
            }
        }

        // Sort alphabetically by filename
        std::sort(m_currentItems.begin(), m_currentItems.end(), [](const AssetItem& a, const AssetItem& b) {
            return a.name < b.name;
        });

    } catch (const std::exception& e) {
        LOG_ERROR("Error scanning directory: " + std::string(e.what()));
    }
}

AssetItem::Type AssetBrowser::GetFileType(const std::string& extension, const std::string& filename) {
    // Convert extension to lowercase for case-insensitive comparison
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Check for spritesheet meta files first
    if (filename.length() >= 10 && filename.substr(filename.length() - 10) == ".meta.json")
        return AssetItem::Type::Unknown;

    // Check for supported texture formats
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
        return AssetItem::Type::Texture;

    // Check for font formats 
    if (ext == ".ttf" || ext == ".otf")
        return AssetItem::Type::Font;

    // Check for audio formats 
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac")
        return AssetItem::Type::Audio;

    // Check for scene files
    if (ext == ".json")
        return AssetItem::Type::Scene;

    return AssetItem::Type::Unknown;
}

void AssetBrowser::RenderAssetGrid() {
    // Calculate column count based on available width
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, (int)(panelWidth / (m_thumbnailSize + m_padding)));

    ImGui::Columns(columnCount, nullptr, false);

    // Render each asset item
    for (auto& item : m_currentItems) {
        RenderAssetItem(item);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void AssetBrowser::RenderAssetItem(AssetItem& item) {
    ImGui::PushID(item.path.c_str());

    bool clicked = false;
    unsigned int textureID = 0;

    // Load Thumbnail
    if (item.type == AssetItem::Type::Texture) {
        if (!item.thumbnailLoaded) {
            LoadThumbnail(item);
        }

        auto it = m_thumbnailCache.find(item.path);
        if (it != m_thumbnailCache.end() && it->second && it->second->IsValid()) {
            textureID = it->second->GetTextureID();
        }
    }

    fs::path metaPath = fs::path(item.path).string() + ".meta.json";
    bool hasMeta = HasSpriteMetadata(item);

    if (hasMeta) {
        // Setup TreeNode flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
        if (item.isExpanded) flags |= ImGuiTreeNodeFlags_DefaultOpen;

        if (textureID != 0) {
            // Draw thumbnail next to the arrow
            // Used GetTreeNodeToLabelSpacing to ensure the image sits nicely next to the arrow
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
            if (ImGui::ImageButton("##thumbnail_icon", (void*)(intptr_t)textureID, ImVec2(m_thumbnailSize, m_thumbnailSize))) {
                clicked = true;
            }

            ImGui::PopStyleColor(3);

            // drag and drop (Bound to the ImageButton item)
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("ASSET_PATH", item.path.c_str(), item.path.size() + 1);
                ImGui::Text("%s (Spritesheet)", item.name.c_str());
                ImGui::EndDragDropSource();
            }
        }
        else {
            // No image, just name
            ImGui::SameLine();
            ImGui::Text("%s", item.name.c_str());
        }

        // dropdown arrow
        bool nodeOpen = ImGui::TreeNodeEx(
            "##TreeNode",
            flags
        );

        //drag and drop
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ASSET_PATH", item.path.c_str(), item.path.size() + 1);
            ImGui::Text("%s (Spritesheet)", item.name.c_str());
            ImGui::EndDragDropSource();
        }

        // Check if the TreeNodeEx area was clicked for selection
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            clicked = true;
        }

        // Render children if expanded
        item.isExpanded = nodeOpen;

        if (nodeOpen) {
            ImGui::Unindent();
            RenderSpriteChildItems(item, metaPath.string());
            ImGui::TreePop(); // Must call TreePop()
        }
    }
    else {
        // Standard Textures
        if (item.type == AssetItem::Type::Texture) {
            if (textureID != 0) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
                clicked = ImGui::ImageButton("##thumbnail", (void*)(intptr_t)textureID,
                    ImVec2(m_thumbnailSize, m_thumbnailSize));
                ImGui::PopStyleColor(3);
            }
            else {
                ImVec4 iconColor = GetIconColor(item.type);
                ImGui::PushStyleColor(ImGuiCol_Button, iconColor);
                clicked = ImGui::Button("##icon", ImVec2(m_thumbnailSize, m_thumbnailSize));
                ImGui::PopStyleColor();
            }
        }
        // Audio Files - NEW!
        else if (item.type == AssetItem::Type::Audio) {
            // Show audio icon (green color)
            ImVec4 iconColor = GetIconColor(item.type);
            ImGui::PushStyleColor(ImGuiCol_Button, iconColor);
            clicked = ImGui::Button("##audio_icon", ImVec2(m_thumbnailSize, m_thumbnailSize));
            ImGui::PopStyleColor();
        }
        // Other Asset Types (Folders, etc.)
        else {
            ImVec4 iconColor = GetIconColor(item.type);
            ImGui::PushStyleColor(ImGuiCol_Button, iconColor);
            clicked = ImGui::Button("##icon", ImVec2(m_thumbnailSize, m_thumbnailSize));
            ImGui::PopStyleColor();
        }

        // Drag and drop support
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ASSET_PATH", item.path.c_str(), item.path.size() + 1);

            // Different text for audio vs texture
            if (item.type == AssetItem::Type::Audio) {
                ImGui::Text("Audio %s (Audio)", item.name.c_str());
            }
            else {
                ImGui::Text("%s", item.name.c_str());
            }

            ImGui::EndDragDropSource();
        }
    }

    // Selection on click
    if (clicked) {
        m_selectedItem = &item;
    }

    // Display filename below thumbnail (applies to all items in grid view)
    ImGui::TextWrapped("%s", item.name.c_str());

    ImGui::PopID();
}

void AssetBrowser::LoadThumbnail(AssetItem& item) {
    item.thumbnailLoaded = true;

    // Check if already cached
    if (m_thumbnailCache.find(item.path) != m_thumbnailCache.end()) {
        return;
    }

    // Load texture via ResourceManager (handles caching and lifecycle)
    auto& resMgr = GP2Engine::ResourceManager::GetInstance();
    auto texture = resMgr.LoadTexture(item.path);

    if (!texture || !texture->IsValid()) {
        LOG_ERROR("Failed to load thumbnail: " + item.path);
        return;
    }

    // Configure texture for thumbnail display
    texture->SetFilterMode(true);   // Linear filtering for smooth scaling
    texture->SetWrapMode(false);    // Clamp to edge to prevent artifacts

    // Cache texture (shared_ptr managed by ResourceManager)
    m_thumbnailCache[item.path] = texture;
    item.thumbnailTextureID = texture->GetTextureID();

    LOG_INFO("Loaded thumbnail: " + item.name + " (ID: " + std::to_string(texture->GetTextureID()) + ")");
}

ImVec4 AssetBrowser::GetIconColor(AssetItem::Type type) {
    // Return color-coded fallback color for file types without thumbnails
    switch (type) {
        case AssetItem::Type::Folder:   return ImVec4(0.9f, 0.7f, 0.3f, 1.0f);  // Yellow
        case AssetItem::Type::Texture:  return ImVec4(0.3f, 0.7f, 0.9f, 1.0f);  // Blue
        case AssetItem::Type::Font:     return ImVec4(0.7f, 0.3f, 0.9f, 1.0f);  // Purple
        case AssetItem::Type::Audio:    return ImVec4(0.3f, 0.9f, 0.5f, 1.0f);  // Green
        case AssetItem::Type::Scene:    return ImVec4(0.9f, 0.5f, 0.3f, 1.0f);  // Orange
        default:                        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // Gray (unknown)
    }
}

void AssetBrowser::CleanupThumbnails() {
    // Release thumbnail cache (shared_ptr automatically manages texture cleanup)
    size_t count = m_thumbnailCache.size();
    m_thumbnailCache.clear();
    LOG_INFO("Asset thumbnails cleaned up (" + std::to_string(count) + " textures released)");
}

std::string AssetBrowser::ImportTexture(const std::string& sourcePath) {
    // Validate source file exists
    if (!std::filesystem::exists(sourcePath)) {
        LOG_ERROR("Source file does not exist: " + sourcePath);
        return "";
    }

    try {
        // Extract filename from source path
        std::filesystem::path sourceFile(sourcePath);
        std::string filename = sourceFile.filename().string();

        // Build destination path in assets folder
        std::filesystem::path destPath = std::filesystem::path(m_assetsRootPath) / filename;

        // Copy texture file to assets folder (overwrite if exists)
        std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);

        LOG_INFO("Imported texture: " + filename + " to " + destPath.string());

        // Refresh asset list to show new texture
        ScanDirectory(m_assetsRootPath);

        return destPath.string();
    } catch (const std::exception& e) {
        LOG_ERROR("Error importing texture: " + std::string(e.what()));
        return "";
    }
}

std::string AssetBrowser::ImportAudio(const std::string& sourcePath) {
    // Validate source file exists
    if (!std::filesystem::exists(sourcePath)) {
        LOG_ERROR("Source audio file does not exist: " + sourcePath);
        return "";
    }

    try {
        // Extract filename from source path
        std::filesystem::path sourceFile(sourcePath);
        std::string filename = sourceFile.filename().string();

        // Build destination path in assets/audio_files folder
        std::filesystem::path destPath = std::filesystem::path(m_assetsRootPath) / "audio_files" / filename;

        // Create audio_files directory if it doesn't exist
        std::filesystem::create_directories(destPath.parent_path());

        // Copy audio file to assets folder (overwrite if exists)
        std::filesystem::copy_file(sourcePath, destPath,
            std::filesystem::copy_options::overwrite_existing);

        LOG_INFO("Imported audio: " + filename + " to " + destPath.string());

        // Refresh asset list to show new audio file
        ScanDirectory(m_currentDirectory);

        return destPath.string();
    }
    catch (const std::exception& e) {
        LOG_ERROR("Error importing audio: " + std::string(e.what()));
        return "";
    }
}

void AssetBrowser::RenderTiledTexturePreview(AssetItem& item, const std::string& metaPath) {

    // --- 1. Load/Retrieve Metadata (Cache Lookup) ---
    // The cache key is the path to the original texture (.png)
    auto cacheIt = m_spritesheetCache.find(item.path);

    if (cacheIt == m_spritesheetCache.end()) {
        // CACHE MISS: Load metadata and texture, then cache the result

        SpriteMetadata meta = SpriteEditor::LoadSpriteMetadata(metaPath);

        LOG_INFO("DEBUG META LOADED: Path=" + meta.texturePath +
            ", R=" + std::to_string(meta.rows) +
            ", C=" + std::to_string(meta.columns) +
            ", TILE_W=" + std::to_string(meta.tileWidth));

        // Validate and handle failure
        if (meta.rows <= 0 || meta.columns <= 0 || meta.texturePath.empty()) {
            // Cache bad data to prevent re-parsing every frame
            m_spritesheetCache[item.path] = meta;
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Meta Data!");
            return;
        }

        // Load Texture
        meta.cachedTexture = GP2Engine::ResourceManager::GetInstance().LoadTexture(meta.texturePath);

        // Cache the entire struct for future frames
        m_spritesheetCache[item.path] = meta;
        cacheIt = m_spritesheetCache.find(item.path); // Point iterator to the newly cached item
    }

    // Render the grid of tiles

    const SpriteMetadata& cachedMeta = cacheIt->second;
    std::shared_ptr<GP2Engine::Texture> texture = cachedMeta.cachedTexture; // Get the cached pointer

    if (!texture || !texture->IsValid()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Texture Missing!");
        return;
    }

    // Use the cached metadata for rendering:
    const int maxTiles = 16;
    const int numTiles = std::min(cachedMeta.rows * cachedMeta.columns, maxTiles);
    int tilesAcross = std::min(cachedMeta.columns, 4);
    float tileSize = m_thumbnailSize / tilesAcross;

    ImGui::BeginGroup();

    // This loop now runs every frame but uses cached pointers and values, resulting in fast rendering
    for (int i = 0; i < numTiles; ++i) {
        int r = i / cachedMeta.columns;
        int c = i % cachedMeta.columns;

        float textureW = (float)texture->GetWidth();
        float textureH = (float)texture->GetHeight();

        // Calculate UV coordinates using cached tile dimensions
        ImVec2 uv0 = ImVec2((float)c * cachedMeta.tileWidth / textureW,
            (float)r * cachedMeta.tileHeight / textureH);
        ImVec2 uv1 = ImVec2((float)(c + 1) * cachedMeta.tileWidth / textureW,
            (float)(r + 1) * cachedMeta.tileHeight / textureH);

        ImGui::Image(
            (ImTextureID)(intptr_t)texture->GetTextureID(),
            ImVec2(tileSize, tileSize),
            uv0, uv1
        );

        // Tooltip
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            ImGui::SetTooltip("Tile Index: %d", i);
        }

        if ((i + 1) % tilesAcross != 0 && i < numTiles - 1) {
            ImGui::SameLine(0.0f, 0.0f); // No spacing
        }
    }
    ImGui::EndGroup();
}

void AssetBrowser::RenderSpriteChildItems(AssetItem& item, const std::string& metaPath) {

    //Load Metadata
    auto cacheIt = m_spritesheetCache.find(item.path);

    // If cache miss, load metadata and texture
    if (cacheIt == m_spritesheetCache.end()) {
        SpriteMetadata meta = SpriteEditor::LoadSpriteMetadata(metaPath);

        if (meta.rows <= 0 || meta.columns <= 0 || meta.texturePath.empty()) {
            m_spritesheetCache[item.path] = meta;
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Meta Data!");
            return;
        }

        // Load Texture
        meta.cachedTexture = GP2Engine::ResourceManager::GetInstance().LoadTexture(meta.texturePath);
        m_spritesheetCache[item.path] = meta;
        cacheIt = m_spritesheetCache.find(item.path);
    }

    const SpriteMetadata& cachedMeta = cacheIt->second;
    std::shared_ptr<GP2Engine::Texture> texture = cachedMeta.cachedTexture;

    if (!texture || !texture->IsValid()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Texture Missing!");
        return;
    }

    //Render each tile as an individual list item
    float textureW = (float)texture->GetWidth();
    float textureH = (float)texture->GetHeight();

    // Use a smaller size for the individual sprite tile previews
    float tilePreviewSize = m_thumbnailSize /*/ 2.0f*/;

    for (int i = 0; i < cachedMeta.rows * cachedMeta.columns; ++i) {
        ImGui::PushID(i); // Unique ID for each tile

        int r = i / cachedMeta.columns;
        int c = i % cachedMeta.columns;

        // Calculate UV coordinates for this specific tile
        ImVec2 uv0 = ImVec2((float)c * cachedMeta.tileWidth / textureW,
            (float)r * cachedMeta.tileHeight / textureH);
        ImVec2 uv1 = ImVec2((float)(c + 1) * cachedMeta.tileWidth / textureW,
            (float)(r + 1) * cachedMeta.tileHeight / textureH);


        // Render the tile as an image button (for click/drag-drop functionality)
        bool clicked = ImGui::ImageButton(
            "##tile_preview",
            (ImTextureID)(intptr_t)texture->GetTextureID(),
            ImVec2(tilePreviewSize, tilePreviewSize),
            uv0, uv1
        );
        

        // Drag-drop source logic for the individual tile
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("SPRITE_TILE_INDEX", &i, sizeof(int));
            ImGui::Text("Dragging Tile %d", i);
            ImGui::EndDragDropSource();
        }

        if (clicked) {
            // to handle clicking the individual tile here
        }

        ImGui::PopID();
    }
}

bool AssetBrowser::HasSpriteMetadata(const AssetItem& item) const {
    // Only process textures
    if (item.type != AssetItem::Type::Texture) {
        return false;
    }

    // Determine the path to the potential meta file
    fs::path metaPath = fs::path(item.path).string() + ".meta.json";

    // Check if the meta file exists
    if (!fs::exists(metaPath)) {
        return false;
    }
    SpriteMetadata meta = SpriteEditor::LoadSpriteMetadata(metaPath.string());

    // treat it as a spritesheet only if it has a grid > 1x1.
    return meta.rows > 1 || meta.columns > 1;
}
