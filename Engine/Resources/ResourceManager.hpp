/**
 * @file ResourceManager.hpp
 * @author Adi (100%)
 * @brief Centralized asset management with caching
 *
 * Singleton ResourceManager that loads and caches all game assets:
 * - Textures (PNG/JPG)
 * - Audio files (WAV via FMOD)
 * - Fonts (TTF)
 * - Shaders (GLSL vertex + fragment pairs)
 *
 * Features:
 * - RAII memory management using shared_ptr
 * - Automatic caching to prevent duplicate loads
 * - Configurable asset base path
 */

#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include "../Graphics/Texture.hpp"
#include "../Graphics/Font.hpp"
#include "../Graphics/Shader.hpp"

namespace GP2Engine {

class ResourceManager {
public:
    // Singleton access
    static ResourceManager& GetInstance();

    // Prevent copying
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // ==================== CONFIGURATION ====================

    /**
     * @brief Set base path for all asset loading
     * Default: "../../Sandbox/assets/"
     */
    void SetBasePath(const std::string& basePath);

    /**
     * @brief Get current base path
     */
    const std::string& GetBasePath() const { return m_basePath; }

    // ==================== TEXTURE LOADING ====================

    /**
     * @brief Load texture from file (cached)
     * @param relativePath Path relative to base path (e.g., "textures/player.png")
     * @return Shared pointer to texture, or nullptr on failure
     */
    std::shared_ptr<Texture> LoadTexture(const std::string& relativePath);

    /**
     * @brief Unload specific texture from cache
     */
    void UnloadTexture(const std::string& relativePath);

    /**
     * @brief Get cached texture count
     */
    size_t GetTextureCount() const { return m_textures.size(); }

    // ==================== FONT LOADING ====================

    /**
     * @brief Load font from file (cached)
     * @param relativePath Path relative to base path (e.g., "fonts/arial.ttf")
     * @param fontSize Font size in pixels
     * @return Shared pointer to font, or nullptr on failure
     */
    std::shared_ptr<Font> LoadFont(const std::string& relativePath, unsigned int fontSize);

    /**
     * @brief Unload specific font from cache
     */
    void UnloadFont(const std::string& relativePath, unsigned int fontSize);

    /**
     * @brief Get cached font count
     */
    size_t GetFontCount() const { return m_fonts.size(); }

    // ==================== SHADER LOADING ====================

    /**
     * @brief Load shader from vertex and fragment files (cached)
     * @param vertexPath Path to vertex shader relative to base path
     * @param fragmentPath Path to fragment shader relative to base path
     * @return Shared pointer to shader, or nullptr on failure
     */
    std::shared_ptr<Shader> LoadShader(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief Unload specific shader from cache
     */
    void UnloadShader(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief Get cached shader count
     */
    size_t GetShaderCount() const { return m_shaders.size(); }

    // ==================== AUDIO LOADING ====================

    /**
     * @brief Load audio file via FMOD (cached by name)
     * @param relativePath Path relative to base path (e.g., "audio/music.wav")
     * @param is3D Load as 3D sound (default: false for 2D)
     * @param isLooping Should sound loop (default: false)
     * @param isStreaming Stream from disk vs load into memory (default: false)
     * @return True if loaded successfully
     */
    bool LoadAudio(const std::string& relativePath, bool is3D = false, bool isLooping = false, bool isStreaming = false);

    /**
     * @brief Unload audio from FMOD system
     */
    void UnloadAudio(const std::string& relativePath);

    /**
     * @brief Get loaded audio count
     */
    size_t GetAudioCount() const { return m_audioFiles.size(); }

    // ==================== CLEANUP ====================

    /**
     * @brief Clear all cached resources
     */
    void ClearAll();

    /**
     * @brief Clear all textures
     */
    void ClearTextures();

    /**
     * @brief Clear all fonts
     */
    void ClearFonts();

    /**
     * @brief Clear all shaders
     */
    void ClearShaders();

    /**
     * @brief Clear all audio
     */
    void ClearAudio();

private:
    ResourceManager();
    ~ResourceManager();

    /**
     * @brief Resolve relative path to full path
     */
    std::string ResolvePath(const std::string& relativePath) const;

    std::string m_basePath;

    // Asset caches with shared_ptr for RAII
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Font>> m_fonts;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, bool> m_audioFiles;
};

} // namespace GP2Engine
