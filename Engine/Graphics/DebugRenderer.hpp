/**
 * @file DebugRenderer.hpp
 * @brief Debug renderer for drawing geometric shapes for debugging purposes
 * @author Asri (100%)
 * 
 * This file contains the DebugRenderer class definition which provides
 * immediate-mode rendering for debugging collision data, AABB boxes,
 * velocity vectors, and other debug visualizations.
 */

#pragma once

#include "../Math/Vector2D.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace GP2Engine {
    
    // Forward declarations
    class Renderer;
    
    /**
     * @brief Color structure for debug rendering
     * 
     * Represents RGBA color values for debug drawing operations.
     * Provides static methods for common colors.
     * 
     * @author Asri (100%)
     */
    struct Color {
        float r, g, b, a;  ///< Red, Green, Blue, Alpha components (0.0 to 1.0)
        
        /**
         * @brief Default constructor (white)
         */
        Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        
        /**
         * @brief Constructor with RGBA values
         * 
         * @param red Red component (0.0 to 1.0)
         * @param green Green component (0.0 to 1.0)
         * @param blue Blue component (0.0 to 1.0)
         * @param alpha Alpha component (0.0 to 1.0)
         */
        Color(float red, float green, float blue, float alpha = 1.0f) 
            : r(red), g(green), b(blue), a(alpha) {}
        
        
        // Inline definitions to avoid incomplete type issues
        static Color GetRed() { return Color(1.0f, 0.0f, 0.0f, 1.0f); }     ///< Get red color
        static Color GetGreen() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }   ///< Get green color
        static Color GetBlue() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }    ///< Get blue color
        static Color GetWhite() { return Color(1.0f, 1.0f, 1.0f, 1.0f); }   ///< Get white color
        static Color GetBlack() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }   ///< Get black color
        static Color GetYellow() { return Color(1.0f, 1.0f, 0.0f, 1.0f); }  ///< Get yellow color
        static Color GetMagenta() { return Color(1.0f, 0.0f, 1.0f, 1.0f); } ///< Get magenta color
        static Color GetCyan() { return Color(0.0f, 1.0f, 1.0f, 1.0f); }    ///< Get cyan color
    };
    
    /**
     * @brief Debug renderer for drawing geometric shapes for debugging purposes
     * 
     * Provides immediate-mode rendering for debugging collision data, AABB boxes,
     * velocity vectors, and other debug visualizations. All shapes are drawn
     * without textures using simple geometric primitives.
     * 
     * Satisfies Rubric 1202 requirements for drawing points, lines, rectangles, and circles.
     * 
     * @author Asri (100%)
     */
    class DebugRenderer {
    public:
        /**
         * @brief Constructor
         */
        DebugRenderer();
        
        /**
         * @brief Destructor
         */
        ~DebugRenderer();
        
        /**
         * @brief Delete copy constructor and assignment operator
         */
        DebugRenderer(const DebugRenderer&) = delete;
        DebugRenderer& operator=(const DebugRenderer&) = delete;
        
        /**
         * @brief Initialize the debug renderer
         * 
         * Sets up OpenGL resources and shaders for debug rendering.
         * Must be called before using any drawing functions.
         * 
         * @return true if initialization successful, false otherwise
         */
        bool Initialize();
        
        /**
         * @brief Shutdown the debug renderer
         * 
         * Cleans up OpenGL resources and shaders.
         */
        void Shutdown();
        
        /**
         * @brief Begin a new debug frame
         * 
         * Clears vertex buffers for new frame. Call this at the start
         * of each frame before drawing debug shapes.
         */
        void Begin();
        
        
        /**
         * @brief Flush debug rendering to screen
         * 
         * Renders all accumulated debug shapes to the screen.
         * 
         * @param renderer Reference to the main renderer for camera access
         */
        void Flush(const Renderer& renderer);
        
        /**
         * @brief Draw a debug point
         * 
         * @param position Position of the point
         * @param color Color of the point
         * @param size Size of the point
         */
        void DrawPoint(const Vector2D& position, const Color& color = Color::GetWhite(), float size = 2.0f);
        
        /**
         * @brief Draw a debug line
         * 
         * @param start Starting position of the line
         * @param end Ending position of the line
         * @param color Color of the line
         * @param thickness Thickness of the line
         */
        void DrawLine(const Vector2D& start, const Vector2D& end, const Color& color = Color::GetWhite(), float thickness = 1.0f);
        
        /**
         * @brief Draw a debug rectangle
         * 
         * @param position Center position of the rectangle
         * @param size Size of the rectangle
         * @param color Color of the rectangle
         * @param filled Whether to draw filled or outline
         */
        void DrawRectangle(const Vector2D& position, const Vector2D& size, const Color& color = Color::GetWhite(), bool filled = false);
        
        /**
         * @brief Draw a debug circle
         * 
         * @param center Center position of the circle
         * @param radius Radius of the circle
         * @param color Color of the circle
         * @param filled Whether to draw filled or outline
         * @param segments Number of segments for circle approximation
         */
        void DrawCircle(const Vector2D& center, float radius, const Color& color = Color::GetWhite(), bool filled = false, int segments = 32);
        
        
    private:
        /**
         * @brief Vertex structure for debug rendering
         */
        struct DebugVertex {
            glm::vec2 position;  ///< Vertex position
            glm::vec4 color;     ///< Vertex color
            
            /**
             * @brief Default constructor
             */
            DebugVertex() = default;
            
            /**
             * @brief Constructor with position and color
             * 
             * @param pos Vertex position
             * @param col Vertex color
             */
            DebugVertex(const glm::vec2& pos, const glm::vec4& col) 
                : position(pos), color(col) {}
        };
        
        // Render data - separate buffers for different primitive types
        std::vector<DebugVertex> m_LineVertices;      ///< Line vertices buffer
        std::vector<DebugVertex> m_TriangleVertices;  ///< Triangle vertices buffer
        std::vector<DebugVertex> m_PointVertices;    ///< Point vertices buffer
        
        // OpenGL resources
        unsigned int m_LineVAO{0}, m_LineVBO{0};           ///< Line VAO/VBO
        unsigned int m_TriangleVAO{0}, m_TriangleVBO{0};   ///< Triangle VAO/VBO
        unsigned int m_PointVAO{0}, m_PointVBO{0};         ///< Point VAO/VBO
        
        // Shader program for debug rendering
        unsigned int m_DebugShaderProgram{0};  ///< Debug shader program ID
        
        // Configuration
        bool m_Initialized{false};    ///< Initialization flag
        
        /**
         * @brief Setup OpenGL buffers for debug rendering
         */
        void SetupBuffers();
        
        /**
         * @brief Cleanup OpenGL buffers
         */
        void CleanupBuffers();
        
        /**
         * @brief Create debug shader program
         */
        void CreateDebugShader();
        
        /**
         * @brief Add line vertices to buffer
         * 
         * @param start Starting position
         * @param end Ending position
         * @param color Line color
         */
        void AddLine(const Vector2D& start, const Vector2D& end, const Color& color);
        
        /**
         * @brief Add triangle vertices to buffer
         * 
         * @param p1 First vertex position
         * @param p2 Second vertex position
         * @param p3 Third vertex position
         * @param color Triangle color
         */
        void AddTriangle(const Vector2D& p1, const Vector2D& p2, const Vector2D& p3, const Color& color);
        
        /**
         * @brief Add point vertex to buffer
         * 
         * @param position Point position
         * @param color Point color
         * @param size Point size
         */
        void AddPoint(const Vector2D& position, const Color& color, float size);
        
        /**
         * @brief Generate circle points for rendering
         * 
         * @param center Circle center
         * @param radius Circle radius
         * @param segments Number of segments
         * @return Vector of circle points
         */
        std::vector<Vector2D> GenerateCirclePoints(const Vector2D& center, float radius, int segments);
    };
    
} // namespace GP2Engine