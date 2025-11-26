/**
 * @file ResourceManager.cpp
 * @author Asri (100%)
 * @brief Implementation of centralized resource manager
 */

#include "ResourceManager.hpp"
#include "../Audio/AudioEngine.hpp"
#include "../Core/Logger.hpp"

namespace GP2Engine {

// ==================== SINGLETON ====================

ResourceManager::ResourceManager()
    : m_basePath("../../Sandbox/assets/")
{
    // Default base path points to Sandbox assets directory
    // Can be changed via SetBasePath() for different project structures
}

ResourceManager::~ResourceManager() {
    // Clear all cached resources on shutdown
    // Ensures proper cleanup and prevents memory leaks
    ClearAll();
}

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

// ==================== CONFIGURATION ====================

void ResourceManager::SetBasePath(const std::string& basePath) {
    m_basePath = basePath;

    // Ensure trailing slash
    if (!m_basePath.empty() && m_basePath.back() != '/' && m_basePath.back() != '\\') {
        m_basePath += '/';
    }
}

// ==================== TEXTURE LOADING ====================

std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& relativePath) {
    // Check cache first - avoids redundant disk I/O and GPU uploads
    // Using relative path as key allows same asset from different base paths
    auto it = m_textures.find(relativePath);
    if (it != m_textures.end()) {
        LOG_INFO("ResourceManager: Using cached texture: " + relativePath);
        return it->second;
    }

    // Not cached - load from disk
    std::string fullPath = ResolvePath(relativePath);
    LOG_INFO("ResourceManager: Loading texture: " + fullPath);

    // Create texture from file
    auto texture = Texture::Create(fullPath);
    if (!texture || !texture->IsValid()) {
        LOG_ERROR("ResourceManager: Failed to load texture: " + fullPath);
        return nullptr;
    }

    // Cache for future requests - shared_ptr ensures automatic cleanup
    m_textures[relativePath] = texture;
    LOG_INFO("ResourceManager: Loaded texture: " + relativePath);
    return texture;
}

void ResourceManager::UnloadTexture(const std::string& relativePath) {
    // Remove from cache - shared_ptr will delete if no other references exist
    m_textures.erase(relativePath);
    LOG_INFO("ResourceManager: Unloaded texture: " + relativePath);
}

// ==================== FONT LOADING ====================

std::shared_ptr<Font> ResourceManager::LoadFont(const std::string& relativePath, unsigned int fontSize) {
    // Fonts must be cached by BOTH path and size
    // Same TTF file at different sizes creates different bitmap atlases
    std::string key = relativePath + "_" + std::to_string(fontSize);

    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second;
    }

    // Not cached - load and rasterize font at requested size
    std::string fullPath = ResolvePath(relativePath);
    auto font = Font::Create(fullPath, fontSize);
    if (!font) {
        LOG_ERROR("ResourceManager: Failed to load font: " + fullPath);
        return nullptr;
    }

    // Cache the rasterized font atlas for this specific size
    m_fonts[key] = font;
    LOG_INFO("ResourceManager: Loaded font: " + relativePath + " (" + std::to_string(fontSize) + "px)");
    return font;
}

void ResourceManager::UnloadFont(const std::string& relativePath, unsigned int fontSize) {
    // Must match the same key format used in LoadFont
    std::string key = relativePath + "_" + std::to_string(fontSize);
    m_fonts.erase(key);
    LOG_INFO("ResourceManager: Unloaded font: " + relativePath);
}

// ==================== SHADER LOADING ====================

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    // Shaders cached by BOTH vertex and fragment paths
    // Using pipe separator to create unique key for each shader program
    std::string key = vertexPath + "|" + fragmentPath;

    auto it = m_shaders.find(key);
    if (it != m_shaders.end()) {
        return it->second;
    }

    // Not cached - resolve paths and compile shader program
    std::string fullVertexPath = ResolvePath(vertexPath);
    std::string fullFragmentPath = ResolvePath(fragmentPath);

    try {
        // Shader constructor compiles GLSL and links program
        auto shader = std::make_shared<Shader>(fullVertexPath, fullFragmentPath);
        m_shaders[key] = shader;

        LOG_INFO("ResourceManager: Loaded shader: " + vertexPath + " + " + fragmentPath);
        return shader;

    } catch (const std::exception& e) {
        // Shader compilation errors are thrown as exceptions
        LOG_ERROR("ResourceManager: Failed to load shader: " + std::string(e.what()));
        return nullptr;
    }
}

void ResourceManager::UnloadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    // Must match the same key format used in LoadShader
    std::string key = vertexPath + "|" + fragmentPath;
    m_shaders.erase(key);
    LOG_INFO("ResourceManager: Unloaded shader: " + vertexPath + " + " + fragmentPath);
}

// ==================== AUDIO LOADING ====================

bool ResourceManager::LoadAudio(const std::string& relativePath, bool is3D, bool isLooping, bool isStreaming) {
    // Check if already loaded - audio managed by FMOD system
    // Track which files are loaded, FMOD handles the actual audio data
    if (m_audioFiles.count(relativePath) > 0) {
        return true;
    }

    std::string fullPath = ResolvePath(relativePath);

    try {
        // Delegate to FMOD audio engine for loading
        // is3D: enables 3D spatial audio features
        // isLooping: sound repeats automatically
        // isStreaming: stream from disk instead of loading into memory (for large files)
        DKAudioEngine::LoadSound(fullPath, is3D, isLooping, isStreaming);
        m_audioFiles[relativePath] = true;

        // Log with flags for debugging audio configuration
        std::string flags = (is3D ? " (3D)" : " (2D)");
        if (isLooping) flags += " (looping)";
        if (isStreaming) flags += " (streaming)";

        LOG_INFO("ResourceManager: Loaded audio: " + relativePath + flags);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("ResourceManager: Failed to load audio: " + relativePath + " - " + e.what());
        return false;
    }
}

void ResourceManager::UnloadAudio(const std::string& relativePath) {
    // Early exit if not loaded
    if (m_audioFiles.find(relativePath) == m_audioFiles.end()) {
        return;
    }

    try {
        // Delegate unloading to FMOD - frees audio memory
        DKAudioEngine::UnLoadSound(ResolvePath(relativePath));
        m_audioFiles.erase(relativePath);
        LOG_INFO("ResourceManager: Unloaded audio: " + relativePath);
    } catch (const std::exception& e) {
        LOG_ERROR("ResourceManager: Unload error: " + std::string(e.what()));
    }
}

// ==================== CLEANUP ====================

void ResourceManager::ClearAll() {
    // Clear all cached resources
    // Called during shutdown or when switching between scenes/projects
    ClearTextures();
    ClearFonts();
    ClearShaders();
    ClearAudio();
    LOG_INFO("ResourceManager: Cleared all resources");
}

void ResourceManager::ClearTextures() {
    // Removes all textures from cache
    m_textures.clear();
}

void ResourceManager::ClearFonts() {
    // Removes all fonts from cache
    // shared_ptr handles cleanup of font atlas textures
    m_fonts.clear();
}

void ResourceManager::ClearShaders() {
    // Removes all shaders from cache
    // shared_ptr handles cleanup of OpenGL shader programs
    m_shaders.clear();
}

void ResourceManager::ClearAudio() {
    // Only clears tracking map - actual audio cleanup handled by FMOD
    // Audio memory freed when FMOD system shuts down
    m_audioFiles.clear();
}

// ==================== HELPER METHODS ====================

std::string ResourceManager::ResolvePath(const std::string& relativePath) const {
    // Handle absolute paths
    if (std::filesystem::path(relativePath).is_absolute()) {
        return relativePath;
    }

    // Handle relative paths
    // Prepend base path to create full path
    // Example: "textures/player.png" -> "../../Sandbox/assets/textures/player.png"
    return m_basePath + relativePath;
}

} // namespace GP2Engine
