/**
 * @file Font.hpp
 * @brief Font class for text rendering with FreeType
 * @author Graphics Team
 *
 * This file contains the Font class definition which handles loading
 * TrueType fonts and managing character glyphs for text rendering.
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

// Forward declare FreeType types to avoid including FreeType in header
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace GP2Engine {

    /**
     * @brief Character glyph data
     *
     * Contains all information needed to render a single character.
     */
    struct Character {
        unsigned int textureID;     ///< OpenGL texture ID for this glyph
        glm::ivec2 size;            ///< Size of glyph
        glm::ivec2 bearing;         ///< Offset from baseline to left/top of glyph
        unsigned int advance;       ///< Horizontal offset to advance to next glyph

        Character() : textureID(0), size(0), bearing(0), advance(0) {}
    };

    /**
     * @brief Font class for loading and managing TrueType fonts
     *
     * Handles loading fonts using FreeType, generating texture atlases
     * for characters, and providing glyph information for rendering.
     *
     * Usage:
     * 1. Create font: auto font = Font::Create("path/to/font.ttf", 48);
     * 2. Get character data: const Character& ch = font->GetCharacter('A');
     * 3. Use with TextRenderer to draw text
     *
     * @author Graphics Team
     */
    class Font {
    public:
        /**
         * @brief Constructor
         */
        Font();

        /**
         * @brief Destructor
         */
        ~Font();

        /**
         * @brief Delete copy constructor and assignment operator
         */
        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;

        /**
         * @brief Load font from file
         *
         * @param fontPath Path to TrueType font file
         * @param fontSize Font size in pixels
         * @return true if loading successful, false otherwise
         */
        bool LoadFromFile(const std::string& fontPath, unsigned int fontSize = 48);

        /**
         * @brief Get character glyph data
         *
         * @param c Character to get
         * @return Reference to character data
         */
        const Character& GetCharacter(char c) const;

        /**
         * @brief Check if font is valid
         *
         * @return true if font is loaded, false otherwise
         */
        bool IsValid() const { return m_IsLoaded; }

        /**
         * @brief Get font size
         *
         * @return Font size in pixels
         */
        unsigned int GetFontSize() const { return m_FontSize; }

        /**
         * @brief Get font file path
         *
         * @return Reference to font path
         */
        const std::string& GetFontPath() const { return m_FontPath; }

        /**
         * @brief Calculate text width
         *
         * @param text Text to measure
         * @param scale Text scale factor
         * @return Text width in pixels
         */
        float CalculateTextWidth(const std::string& text, float scale = 1.0f) const;

        /**
         * @brief Create font (static factory method)
         *
         * @param fontPath Path to TrueType font file
         * @param fontSize Font size in pixels
         * @return Shared pointer to font, or nullptr if loading failed
         */
        static std::shared_ptr<Font> Create(const std::string& fontPath, unsigned int fontSize = 48);

        /**
         * @brief Get default character (used when requested character not found)
         *
         * @return Reference to default character
         */
        static const Character& GetDefaultCharacter();

        /**
         * @brief Shutdown FreeType library (call at application exit)
         *
         * Cleans up the static FreeType library instance. Should be called
         * once during application shutdown to prevent memory leaks.
         */
        static void ShutdownFreeTypeLibrary();

    private:
        std::unordered_map<char, Character> m_Characters; ///< Character map
        std::string m_FontPath;                           ///< Font file path
        unsigned int m_FontSize{48};                      ///< Font size in pixels
        bool m_IsLoaded{false};                           ///< Font loaded flag

        /**
         * @brief Load ASCII characters (32-127)
         *
         * @param face FreeType face handle
         * @return true if successful, false otherwise
         */
        bool LoadCharacters(FT_Face face);

        /**
         * @brief Generate OpenGL texture for a single character
         *
         * @param face FreeType face handle
         * @param c Character to generate
         * @return true if successful, false otherwise
         */
        bool GenerateCharacterTexture(FT_Face face, char c);
    };

    // Type alias for shared font pointer
    using FontPtr = std::shared_ptr<Font>;

} // namespace GP2Engine
