/**
 * @file Font.cpp
 * @brief Font class implementation for text rendering with FreeType
 * @author Graphics Team
 *
 * This file contains the implementation of the Font class which handles
 * loading TrueType fonts and managing character glyphs for text rendering.
 */

#include "Font.hpp"
#include <iostream>
#include <glad/glad.h>

// Include FreeType headers
#include <ft2build.h>
#include FT_FREETYPE_H

namespace GP2Engine {

    // Static FreeType library instance
    static FT_Library s_FTLibrary = nullptr;
    static bool s_FTInitialized = false;

    /**
     * @brief Initialize FreeType library
     */
    static bool InitializeFreeType() {
        if (s_FTInitialized) {
            return true;
        }

        if (FT_Init_FreeType(&s_FTLibrary)) {
            std::cerr << "ERROR::FREETYPE: Could not initialize FreeType Library" << std::endl;
            return false;
        }

        s_FTInitialized = true;
        return true;
    }

    Font::Font() = default;

    Font::~Font() {
        // Clean up OpenGL textures
        for (const auto& pair : m_Characters) {
            if (pair.second.textureID != 0) {
                glDeleteTextures(1, &pair.second.textureID);
            }
        }
    }

    bool Font::LoadFromFile(const std::string& fontPath, unsigned int fontSize) {
        // Initialize FreeType if not already done
        if (!InitializeFreeType()) {
            return false;
        }

        m_FontPath = fontPath;
        m_FontSize = fontSize;

        // Load font face
        FT_Face face;
        if (FT_New_Face(s_FTLibrary, fontPath.c_str(), 0, &face)) {
            std::cerr << "ERROR::FREETYPE: Failed to load font from " << fontPath << std::endl;
            return false;
        }

        // Set pixel size
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        // Disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Load ASCII characters
        if (!LoadCharacters(face)) {
            FT_Done_Face(face);
            return false;
        }

        // Cleanup
        FT_Done_Face(face);

        // Reset pixel storage
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        m_IsLoaded = true;
        return true;
    }

    bool Font::LoadCharacters(FT_Face face) {
        // Load first 128 ASCII characters
        for (unsigned char c = 0; c < 128; c++) {
            if (!GenerateCharacterTexture(face, c)) {
                std::cerr << "WARNING::FREETYPE: Failed to load character '" << c << "'" << std::endl;
                // Continue loading other characters even if one fails
            }
        }

        return !m_Characters.empty();
    }

    bool Font::GenerateCharacterTexture(FT_Face face, char c) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            return false;
        }

        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        if (texture == 0) {
            std::cerr << "ERROR::OPENGL: glGenTextures failed for character '" << c << "'" << std::endl;
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, texture);

        // Upload texture data
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "ERROR::OPENGL: glTexImage2D failed for character '" << c
                      << "' with error code: " << error << std::endl;
            glDeleteTextures(1, &texture);
            return false;
        }

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character for later use
        Character character;
        character.textureID = texture;
        character.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        character.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        character.advance = static_cast<unsigned int>(face->glyph->advance.x);

        m_Characters[c] = character;

        return true;
    }

    const Character& Font::GetCharacter(char c) const {
        auto it = m_Characters.find(c);
        if (it != m_Characters.end()) {
            return it->second;
        }

        // Return default character if not found
        return GetDefaultCharacter();
    }

    float Font::CalculateTextWidth(const std::string& text, float scale) const {
        float width = 0.0f;

        for (const char& c : text) {
            const Character& ch = GetCharacter(c);
            width += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }

        return width;
    }

    std::shared_ptr<Font> Font::Create(const std::string& fontPath, unsigned int fontSize) {
        auto font = std::make_shared<Font>();
        if (!font->LoadFromFile(fontPath, fontSize)) {
            std::cerr << "ERROR::FONT: Failed to create font from " << fontPath << std::endl;
            return nullptr;
        }
        return font;
    }

    const Character& Font::GetDefaultCharacter() {
        // Simple static default character
        static const Character defaultChar{};
        return defaultChar;
    }

    void Font::ShutdownFreeTypeLibrary() {
        if (s_FTInitialized) {
            FT_Done_FreeType(s_FTLibrary);
            s_FTInitialized = false;
            s_FTLibrary = nullptr;
        }
    }

} // namespace GP2Engine
