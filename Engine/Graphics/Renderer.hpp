
/**
 * @file Renderer.hpp
 * @brief Main renderer class for 2D graphics operations
 * @author Asri (100%)
 * 
 * This file contains the Renderer class definition which provides the main
 * rendering interface for 2D graphics operations. It handles sprite rendering,
 * batch rendering for performance, and integrates with the debug renderer.
 */

#pragma once

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "Camera.hpp"

namespace GP2Engine {
    
    // Forward declarations
    class Sprite;
    class DebugRenderer;
    struct Transform2D;
   // void GLFWResizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Main renderer class for 2D graphics operations
     * 
     * Provides the main rendering interface for 2D graphics operations.
     * Handles sprite rendering, batch rendering for performance optimization,
     * and integrates with the debug renderer. Supports both immediate-mode
     * and batch rendering for different performance requirements.
     * 
     * Satisfies Rubric 1209 requirements for optimized rendering of 2500+ objects.
     * 
     * @author Asri (100%)
     */
    class Renderer {
    public:
        /**
         * @brief Delete copy constructor and assignment operator
         */
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /**
         * @brief Initialize the renderer with an existing window
         * 
         * Sets up OpenGL context, shaders, and debug renderer.
         * Must be called before any rendering operations.
         * 
         * @param window GLFW window handle
         * @return true if initialization successful, false otherwise
         */
        static bool Initialize(GLFWwindow* window);
        
        /**
         * @brief Clean up renderer resources
         * 
         * Destroys OpenGL resources and shaders.
         */
        static void Shutdown();
        
        /**
         * @brief Get the singleton instance
         * 
         * @return Reference to the renderer instance
         * @throws std::runtime_error if renderer not initialized
         */
        static Renderer& GetInstance();
        
        /**
         * @brief Clear the screen
         * 
         * Clears the color and depth buffers with the background color.
         */
        void Clear() const;
        
        /**
         * @brief Present the rendered content
         * 
         * Swaps the front and back buffers to display rendered content.
         * Also flushes debug rendering.
         */
        void Present() const;
        
        /**
         * @brief Get the GLFW window
         * 
         * @return Pointer to the GLFW window
         */
        GLFWwindow* GetWindow() const { return m_Window; }
        
        /**
         * @brief Get window dimensions
         * 
         * @param width Reference to store window width
         * @param height Reference to store window height
         */
        void GetWindowSize(int& width, int& height) const;
        
        /**
         * @brief Get window width
         * 
         * @return Window width in pixels
         */
        int GetWidth() const { return m_Width; }
        
        /**
         * @brief Get window height
         * 
         * @return Window height in pixels
         */
        int GetHeight() const { return m_Height; }
        
        /**
         * @brief Set window size (for internal use)
         * 
         * @param width Window width
         * @param height Window height
         */
        void SetWindowSize(int width, int height) { m_Width = width; m_Height = height; }
        
        /**
         * @brief Set the current camera
         * 
         * @param camera Camera to use for rendering
         */
        void SetCamera(const Camera& camera);
        
        /**
         * @brief Get the current camera
         * 
         * @return Reference to the current camera
         */
        const Camera& GetCamera() const { return m_Camera; }
        
        /**
         * @brief Check if window should close
         * 
         * @return true if window should close, false otherwise
         */
        bool ShouldClose() const;
        
        /**
         * @brief Draw a sprite using immediate mode rendering
         * 
         * @param sprite Sprite to draw
         */
        void DrawSprite(const GP2Engine::Sprite& sprite);
        
        /**
         * @brief Draw a sprite with external transform
         * 
         * @param sprite Sprite to draw
         * @param transform External transform to apply
         */
        void DrawSprite(const GP2Engine::Sprite& sprite, const Transform2D* transform);
        
        /**
         * @brief Draw a colored quad using glm::vec2
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param color Color of the quad
         */
        void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Draw a colored quad using Vector2D
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param color Color of the quad
         */
        void DrawQuad(const Vector2D& position, const Vector2D& size, const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Draw a colored quad with rotation using glm::vec2
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param rotation Rotation angle in degrees
         * @param color Color of the quad
         */
        void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Draw a colored quad with rotation using Vector2D
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param rotation Rotation angle in degrees
         * @param color Color of the quad
         */
        void DrawQuad(const Vector2D& position, const Vector2D& size, float rotation, const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Draw a textured quad using glm::vec2
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param textureID OpenGL texture ID
         * @param texCoords Texture coordinates (x, y, width, height)
         * @param color Color tint
         */
        void DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size,
                             unsigned int textureID, const glm::vec4& texCoords = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                             const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Draw a textured quad with rotation using glm::vec2
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param rotation Rotation angle in degrees
         * @param textureID OpenGL texture ID
         * @param texCoords Texture coordinates (x, y, width, height)
         * @param color Color tint
         */
        void DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                             unsigned int textureID, const glm::vec4& texCoords = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                             const glm::vec4& color = glm::vec4(1.0f));
        
        /**
         * @brief Begin batch rendering
         */
        void BeginBatch();
        
        /**
         * @brief End batch rendering
         */
        void EndBatch();
        
        /**
         * @brief Flush batch rendering
         */
        void FlushBatch();
        
        
        /**
         * @brief Draw textured quad in batch mode
         * 
         * @param position Position of the quad
         * @param size Size of the quad
         * @param rotation Rotation angle in degrees
         * @param textureID OpenGL texture ID
         * @param texCoords Texture coordinates
         * @param color Color tint
         */
        void DrawTexturedQuadBatch(const glm::vec2& position, const glm::vec2& size, float rotation,
                                  unsigned int textureID, const glm::vec4& texCoords, const glm::vec4& color);
        
        /**
         * @brief Draw colored quad in batch mode
         *
         * @param position Position of the quad
         * @param size Size of the quad
         * @param rotation Rotation angle in degrees
         * @param color Color of the quad
         */
        void DrawQuadBatch(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);


        /**
         * @brief Draw text string
         *
         * @param font Font to use for rendering
         * @param text Text string to render
         * @param x X position (in screen coordinates)
         * @param y Y position (in screen coordinates)
         * @param scale Scale factor (1.0 = normal size)
         * @param color Text color (RGBA)
         */
        void DrawText(class Font* font, const std::string& text, float x, float y,
                     float scale = 1.0f, const glm::vec4& color = glm::vec4(1.0f));

        /**
         * @brief Draw text string using glm::vec2
         *
         * @param font Font to use for rendering
         * @param text Text string to render
         * @param position Position (in screen coordinates)
         * @param scale Scale factor (1.0 = normal size)
         * @param color Text color (RGBA)
         */
        void DrawText(class Font* font, const std::string& text, const glm::vec2& position,
                     float scale = 1.0f, const glm::vec4& color = glm::vec4(1.0f));

        /**
         * @brief Draw text with Transform2D 
         *
         * @param font Font to use for rendering
         * @param text Text string to render
         * @param transform Transform2D for position, rotation, and scale
         * @param textScale Additional text scale multiplier
         * @param color Text color (RGBA)
         */
        void DrawText(class Font* font, const std::string& text, const Transform2D* transform,
                     float textScale = 1.0f, const glm::vec4& color = glm::vec4(1.0f));

        /**
         * @brief Calculate text width
         *
         * @param font Font to use
         * @param text Text to measure
         * @param scale Scale factor
         * @return Text width in pixels
         */
        float MeasureTextWidth(class Font* font, const std::string& text, float scale = 1.0f) const;


        /**
         * @brief Get debug renderer
         *
         * @return Reference to debug renderer
         * @throws std::runtime_error if debug renderer not initialized
         */
        DebugRenderer& GetDebugRenderer() { 
            if (!m_DebugRenderer) {
                throw std::runtime_error("DebugRenderer not initialized");
            }
            return *m_DebugRenderer; 
        }
        
        /**
         * @brief Get debug renderer (const version)
         * 
         * @return Const reference to debug renderer
         * @throws std::runtime_error if debug renderer not initialized
         */
        const DebugRenderer& GetDebugRenderer() const { 
            if (!m_DebugRenderer) {
                throw std::runtime_error("DebugRenderer not initialized");
            }
            return *m_DebugRenderer; 
        }
        
        /**
         * @brief Get number of draw calls this frame
         * 
         * @return Number of draw calls
         */
        int GetDrawCallsThisFrame() const { return m_DrawCallsThisFrame; }
        
        /**
         * @brief Get number of quads drawn this frame
         * 
         * @return Number of quads drawn
         */
        int GetQuadsDrawnThisFrame() const { return m_QuadsDrawnThisFrame; }
        
        /**
         * @brief Reset performance counters
         */
        void ResetPerformanceCounters() const { m_DrawCallsThisFrame = 0; m_QuadsDrawnThisFrame = 0; }
        
        /**
         * @brief Handle window resize event
         * 
         * @param width New window width
         * @param height New window height
         */
        void OnWindowResize(int width, int height);
        
        /**
         * @brief Set callback for window resize events
         * 
         * @param callback Function to call on window resize
         */
        static void SetResizeCallback(std::function<void(int, int)> callback);


        /**
        * @brief Enable or disable VSync
        *
        * @param enabled True to enable VSync, false to disable
        */
        static void SetVSync(bool enabled);

        /**
         * @brief Check if VSync is currently enabled
         *
         * @return True if VSync is enabled, false otherwise
         */
        static bool IsVSyncEnabled();

        
    private:
        /**
         * @brief Private constructor (singleton pattern)
         */
        Renderer() = default;
        
    public:
        /**
         * @brief Destructor
         */
        ~Renderer();
        
        // Static members
        static std::unique_ptr<Renderer> s_Instance;                    ///< Singleton instance
        static std::function<void(int, int)> s_ResizeCallback;         ///< Window resize callback
        
        // Core members
        GLFWwindow* m_Window{nullptr};                                  ///< GLFW window handle
        Camera m_Camera;                                               ///< Current camera
        int m_Width{0};                                                ///< Window width
        int m_Height{0};                                               ///< Window height
        
        /**
         * @brief Vertex structure for batch rendering
         */
        struct QuadVertex {
            glm::vec2 position;        ///< Vertex position
            glm::vec2 texCoords;       ///< Texture coordinates
            glm::vec4 color;           ///< Vertex color
            float textureIndex;        ///< Texture slot index
        };
        
        // Batch rendering system for performance (Rubric 1209)
        static const unsigned int MAX_QUADS = 50000;        ///< Maximum quads per batch
        static const unsigned int MAX_VERTICES = MAX_QUADS * 4;  ///< Maximum vertices per batch
        static const unsigned int MAX_INDICES = MAX_QUADS * 6;   ///< Maximum indices per batch
        static const unsigned int MAX_TEXTURE_SLOTS = 32;        ///< Maximum texture slots
        
        unsigned int m_QuadVAO{0}, m_QuadVBO{0}, m_QuadEBO{0};  ///< Quad VAO/VBO/EBO
        std::vector<QuadVertex> m_QuadVertices;                 ///< Quad vertices buffer
        std::vector<unsigned int> m_TextureSlots;               ///< Texture slots for batch
        unsigned int m_CurrentTextureSlot{0};                   ///< Current texture slot
        
        // Persistent buffer mapping for performance
        QuadVertex* m_MappedVertexBuffer{nullptr};              ///< Mapped vertex buffer
        unsigned int m_VertexBufferOffset{0};                   ///< Vertex buffer offset
        
        std::shared_ptr<class Shader> m_SpriteShader;          ///< Sprite shader
        bool m_BatchStarted{false};                             ///< Batch rendering flag

        // Debug rendering
        mutable std::unique_ptr<DebugRenderer> m_DebugRenderer; ///< Debug renderer instance

        // Text rendering
        unsigned int m_TextVAO{0}, m_TextVBO{0};                ///< Text VAO/VBO
        std::shared_ptr<class Shader> m_TextShader;            ///< Text shader
        bool m_TextRenderingInitialized{false};                 ///< Text rendering initialized flag

        // Performance monitoring
        mutable int m_DrawCallsThisFrame{0};                    ///< Draw calls this frame
        mutable int m_QuadsDrawnThisFrame{0};                   ///< Quads drawn this frame

        /**
         * @brief Initialize text rendering system
         * @return true if successful, false otherwise
         */
        bool InitializeTextRendering();
    };
    
} // namespace GP2Engine
