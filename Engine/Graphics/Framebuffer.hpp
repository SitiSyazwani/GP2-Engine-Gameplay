/**
 * @file Framebuffer.hpp
 * @brief OpenGL framebuffer abstraction for texture rendering
 * @author Adi (100%)
 *
 * Provides a high-level interface for creating and managing OpenGL framebuffers
 * operations. Used for editor viewports, minimaps, and other texture-based rendering tasks.
 *
 * Features:
 * - Color texture attachment for rendering
 * - Depth/stencil renderbuffer for depth testing
 * - Automatic resource cleanup
 * - Resize support
 */

#pragma once

namespace GP2Engine {

    /**
     * @brief OpenGL framebuffer object wrapper
     *
     * Manages an OpenGL framebuffer with color texture and depth/stencil attachments.
     * Provides a clean interface for render-to-texture operations.
     */
    class Framebuffer {
    public:
        /**
         * @brief Construct framebuffer
         */
        Framebuffer() = default;

        /**
         * @brief Destructor - automatically cleans up OpenGL resources
         */
        ~Framebuffer();

        // Delete copy constructor and assignment 
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        /**
         * @brief Create framebuffer with specified dimensions
         * @param width Width in pixels
         * @param height Height in pixels
         * @return true if creation successful, false otherwise
         */
        bool Create(int width, int height);

        /**
         * @brief Resize framebuffer to new dimensions
         * @param width New width in pixels
         * @param height New height in pixels
         */
        void Resize(int width, int height);

        /**
         * @brief Bind framebuffer for rendering
         */
        void Bind() const;

        /**
         * @brief Unbind framebuffer 
         */
        void Unbind() const;

        /**
         * @brief Get color texture ID for display (e.g., in ImGui)
         * @return OpenGL texture ID
         */
        unsigned int GetTextureID() const { return m_textureColorBuffer; }

        /**
         * @brief Get framebuffer width
         */
        int GetWidth() const { return m_width; }

        /**
         * @brief Get framebuffer height
         */
        int GetHeight() const { return m_height; }

        /**
         * @brief Check if framebuffer is valid
         */
        bool IsValid() const { return m_framebuffer != 0; }

        /**
         * @brief Delete framebuffer and free OpenGL resources
         */
        void Delete();

    private:
        unsigned int m_framebuffer = 0;
        unsigned int m_textureColorBuffer = 0;
        unsigned int m_renderbuffer = 0;
        int m_width = 0;
        int m_height = 0;
    };

} // namespace GP2Engine
