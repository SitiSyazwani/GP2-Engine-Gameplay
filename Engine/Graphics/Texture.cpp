/**
 * @file Texture.cpp
 * @brief Texture class implementation for loading and managing OpenGL textures
 * @author Asri (100%)
 * 
 * This file contains the implementation of the Texture class which handles
 * loading images from disk and creating OpenGL textures. It provides resource
 * management and texture metadata access.
 */

#include "Texture.hpp"
#include <glad/glad.h>
#include <iostream>

// STB Image for loading textures
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace GP2Engine {
    
    Texture::Texture() = default;
    
    Texture::Texture(const std::string& filePath) {
        LoadFromFile(filePath);
    }
    
    Texture::~Texture() {
        Destroy();
    }
    
    Texture::Texture(Texture&& other) noexcept 
        : m_TextureID(other.m_TextureID)
        , m_Width(other.m_Width)
        , m_Height(other.m_Height)
        , m_Channels(other.m_Channels)
        , m_FilePath(std::move(other.m_FilePath)) {
        
        // Reset the moved-from object
        other.m_TextureID = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Channels = 0;
    }
    
    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            // Destroy current texture
            Destroy();
            
            // Move data from other
            m_TextureID = other.m_TextureID;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Channels = other.m_Channels;
            m_FilePath = std::move(other.m_FilePath);
            
            // Reset the moved-from object
            other.m_TextureID = 0;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_Channels = 0;
        }
        return *this;
    }
    
    bool Texture::LoadFromFile(const std::string& filePath) {
        // Destroy existing texture if any
        Destroy();
        
        // Don't flip images - let ImGui viewport handle the coordinate conversion
        stbi_set_flip_vertically_on_load(false);
        
        // Load image data - force to 4 channels (RGBA) for consistency
        unsigned char* data = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 4);
        if (data) {
            m_Channels = 4; // Override detected channels to ensure RGBA
        }
        
        if (!data) {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            return false;
        }
        
        m_FilePath = filePath;
        GenerateTexture(data);
        
        // Free image data
        stbi_image_free(data);
        
        return true;
    }
    
    bool Texture::LoadFromData(unsigned char* data, int width, int height, int channels) {
        if (!data || width <= 0 || height <= 0 || channels <= 0) {
            std::cerr << "Invalid texture data provided" << std::endl;
            return false;
        }
        
        // Destroy existing texture if any
        Destroy();
        
        m_Width = width;
        m_Height = height;
        m_Channels = channels;
        m_FilePath = ""; // No file path for raw data
        
        GenerateTexture(data);
        
        return true;
    }
    
    void Texture::Destroy() {
        if (m_TextureID != 0) {
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
            m_Width = 0;
            m_Height = 0;
            m_Channels = 0;
            m_FilePath.clear();
        }
    }
    
    void Texture::Bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
    }
    
    void Texture::Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void Texture::SetFilterMode(bool linear) {
        if (m_TextureID == 0) return;
        
        Bind();
        GLenum filter = linear ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        Unbind();
    }
    
    void Texture::SetWrapMode(bool repeat) {
        if (m_TextureID == 0) return;
        
        Bind();
        GLenum wrap = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        Unbind();
    }
    
    std::shared_ptr<Texture> Texture::Create(const std::string& filePath) {
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(filePath)) {
            return texture;
        }
        return nullptr;
    }
    
    std::shared_ptr<Texture> Texture::CreateFromData(unsigned char* data, int width, int height, int channels) {
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromData(data, width, height, channels)) {
            return texture;
        }
        return nullptr;
    }
    
    void Texture::GenerateTexture(unsigned char* data) {
        // Generate OpenGL texture
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        
        // Determine format based on channels
        GLenum internalFormat, format;
        switch (m_Channels) {
            case 1:
                internalFormat = GL_R8;
                format = GL_RED;
                break;
            case 2:
                internalFormat = GL_RG8;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                std::cerr << "Unsupported number of channels: " << m_Channels << std::endl;
                internalFormat = GL_RGBA8;
                format = GL_RGBA;
                break;
        }
        
        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Set default parameters
        SetDefaultParameters();
        
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void Texture::SetDefaultParameters() {
        // Set texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
} // namespace GP2Engine