/**
 * @file Texture.hpp
 * @brief Texture class for loading and managing OpenGL textures
 * @author Asri (100%)
 * 
 * This file contains the Texture class definition which handles loading
 * images from disk and creating OpenGL textures. It provides resource
 * management and texture metadata access.
 */

#pragma once

#include <string>
#include <memory>

namespace GP2Engine {
    
    /**
     * @brief Texture class for loading and managing OpenGL textures
     * 
     * Handles loading images from disk and creating OpenGL textures.
     * Provides resource management and texture metadata access.
     * Supports various image formats through STB Image library.
     * 
     * @author Asri (100%)
   
     */
    class Texture {
    public:
        /**
         * @brief Default constructor
         */
        Texture();
        
        /**
         * @brief Constructor with file path
         * 
         * @param filePath Path to texture file
         */
        explicit Texture(const std::string& filePath);
        
        /**
         * @brief Destructor
         */
        ~Texture();
        
        /**
         * @brief Delete copy constructor and assignment operator to prevent texture duplication
         */
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        
        /**
         * @brief Move constructor
         * 
         * @param other Texture to move from
         */
        Texture(Texture&& other) noexcept;
        
        /**
         * @brief Move assignment operator
         * 
         * @param other Texture to move from
         * @return Reference to this texture
         */
        Texture& operator=(Texture&& other) noexcept;
        
        /**
         * @brief Load texture from file
         * 
         * @param filePath Path to texture file
         * @return true if loading successful, false otherwise
         */
        bool LoadFromFile(const std::string& filePath);
        
        /**
         * @brief Load texture from raw data
         * 
         * @param data Raw image data
         * @param width Image width
         * @param height Image height
         * @param channels Number of color channels
         * @return true if loading successful, false otherwise
         */
        bool LoadFromData(unsigned char* data, int width, int height, int channels = 4);
        
        /**
         * @brief Destroy texture and free resources
         */
        void Destroy();
        
        /**
         * @brief Get OpenGL texture ID
         * 
         * @return OpenGL texture ID
         */
        unsigned int GetTextureID() const { return m_TextureID; }
        
        /**
         * @brief Get texture width
         * 
         * @return Texture width in pixels
         */
        int GetWidth() const { return m_Width; }
        
        /**
         * @brief Get texture height
         * 
         * @return Texture height in pixels
         */
        int GetHeight() const { return m_Height; }
        
        /**
         * @brief Get number of color channels
         * 
         * @return Number of color channels
         */
        int GetChannels() const { return m_Channels; }
        
        /**
         * @brief Get texture file path
         * 
         * @return Reference to file path
         */
        const std::string& GetFilePath() const { return m_FilePath; }
        
        /**
         * @brief Check if texture is valid
         * 
         * @return true if texture is valid, false otherwise
         */
        bool IsValid() const { return m_TextureID != 0; }
        
        /**
         * @brief Bind texture to OpenGL texture unit
         * 
         * @param slot Texture unit slot (default 0)
         */
        void Bind(unsigned int slot = 0) const;
        
        /**
         * @brief Unbind texture
         */
        void Unbind() const;
        
        /**
         * @brief Set texture filter mode
         * 
         * @param linear true for linear filtering, false for nearest
         */
        void SetFilterMode(bool linear = true);
        
        /**
         * @brief Set texture wrap mode
         * 
         * @param repeat true for repeat, false for clamp to edge
         */
        void SetWrapMode(bool repeat = true);
        
        /**
         * @brief Create texture from file (static factory method)
         * 
         * @param filePath Path to texture file
         * @return Shared pointer to texture, or nullptr if loading failed
         */
        static std::shared_ptr<Texture> Create(const std::string& filePath);
        
        /**
         * @brief Create texture from raw data (static factory method)
         * 
         * @param data Raw image data
         * @param width Image width
         * @param height Image height
         * @param channels Number of color channels
         * @return Shared pointer to texture, or nullptr if loading failed
         */
        static std::shared_ptr<Texture> CreateFromData(unsigned char* data, int width, int height, int channels = 4);
        
    private:
        unsigned int m_TextureID{0};         ///< OpenGL texture ID
        int m_Width{0};                      ///< Texture width
        int m_Height{0};                     ///< Texture height
        int m_Channels{0};                   ///< Number of color channels
        std::string m_FilePath;             ///< Texture file path
        
        /**
         * @brief Generate OpenGL texture from data
         * 
         * @param data Raw image data
         */
        void GenerateTexture(unsigned char* data);
        
        /**
         * @brief Set default texture parameters
         */
        void SetDefaultParameters();
    };
    
    // Type alias for shared texture pointer
    using TexturePtr = std::shared_ptr<Texture>;
    
} // namespace GP2Engine