/**
 * @file DebugRenderer.cpp
 * @brief Debug renderer implementation for drawing geometric shapes for debugging purposes
 * @author Asri (100%)
 * 
 * This file contains the implementation of the DebugRenderer class which provides
 * immediate-mode rendering for debugging collision data, AABB boxes,
 * velocity vectors, and other debug visualizations.
 */

#include "DebugRenderer.hpp"
#include "Renderer.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GP2Engine {
    
    /**
     * @brief Default constructor
     * 
     * Creates a debug renderer instance. The renderer must be initialized
     * with Initialize() before use.
     */
    DebugRenderer::DebugRenderer() = default;
    
    /**
     * @brief Destructor
     * 
     * Automatically shuts down the debug renderer and cleans up OpenGL resources.
     */
    DebugRenderer::~DebugRenderer() {
        Shutdown();
    }
    
    /**
     * @brief Initialize the debug renderer
     * 
     * Sets up OpenGL resources including VAOs, VBOs, and shaders for debug rendering.
     * This method must be called before any drawing operations.
     * 
     * The initialization process:
     * 1. Creates debug shader program (vertex + fragment shaders)
     * 2. Sets up OpenGL buffers for lines, triangles, and points
     * 3. Configures vertex attribute pointers
     * 
     * @return true if initialization successful, false otherwise
     */
    bool DebugRenderer::Initialize() {
        if (m_Initialized) {
            return true; // Already initialized, no need to reinitialize
        }
        
        // Create debug shader program
        CreateDebugShader();
        if (m_DebugShaderProgram == 0) {
            std::cerr << "Failed to create debug shader" << std::endl;
            return false;
        }
        
        // Setup OpenGL buffers for different primitive types
        SetupBuffers();
        
        m_Initialized = true;
        return true;
    }
    
    /**
     * @brief Shutdown the debug renderer
     * 
     * Cleans up all OpenGL resources including VAOs, VBOs, and shader programs.
     * This method is automatically called by the destructor.
     */
    void DebugRenderer::Shutdown() {
        if (!m_Initialized) {
            return; // Nothing to clean up
        }
        
        // Clean up OpenGL buffers
        CleanupBuffers();
        
        // Clean up shader program
        if (m_DebugShaderProgram != 0) {
            glDeleteProgram(m_DebugShaderProgram);
            m_DebugShaderProgram = 0;
        }
        
        m_Initialized = false;
    }
    
    /**
     * @brief Begin a new debug frame
     * 
     * Clears all vertex buffers for a new frame. This should be called
     * at the start of each frame before drawing any debug shapes.
     * 
     * The method clears:
     * - Line vertices buffer
     * - Triangle vertices buffer  
     * - Point vertices buffer
     */
    void DebugRenderer::Begin() {
        // Clear vertex buffers for new frame
        m_LineVertices.clear();
        m_TriangleVertices.clear();
        m_PointVertices.clear();
    }
    
    
    /**
     * @brief Flush debug rendering to screen
     * 
     * Renders all accumulated debug shapes to the screen using OpenGL.
     * This method must be called after End() and before Present().
     * 
     * The rendering process:
     * 1. Uses the debug shader program
     * 2. Sets up camera matrices from the main renderer
     * 3. Renders lines, triangles, and points in separate draw calls
     * 4. Uses dynamic vertex buffer updates for each primitive type
     * 
     * @param renderer Reference to the main renderer for camera access
     */
    void DebugRenderer::Flush(const Renderer& renderer) {
        // Early return if not initialized
        if (!m_Initialized || m_DebugShaderProgram == 0) {
            return;
        }
        
        // Use debug shader program
        glUseProgram(m_DebugShaderProgram);
        
        // Get camera matrices from renderer and set uniform
        glm::mat4 viewProjection = renderer.GetCamera().GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(m_DebugShaderProgram, "u_ViewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        
        // Render lines
        if (!m_LineVertices.empty()) {
            glBindVertexArray(m_LineVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
            glBufferData(GL_ARRAY_BUFFER, m_LineVertices.size() * sizeof(DebugVertex), m_LineVertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, static_cast<int>(m_LineVertices.size()));
            glBindVertexArray(0);
        }
        
        // Render triangles (for filled shapes)
        if (!m_TriangleVertices.empty()) {
            glBindVertexArray(m_TriangleVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);
            glBufferData(GL_ARRAY_BUFFER, m_TriangleVertices.size() * sizeof(DebugVertex), m_TriangleVertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_TriangleVertices.size()));
            glBindVertexArray(0);
        }
        
        // Render points
        if (!m_PointVertices.empty()) {
            glBindVertexArray(m_PointVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_PointVBO);
            glBufferData(GL_ARRAY_BUFFER, m_PointVertices.size() * sizeof(DebugVertex), m_PointVertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_POINTS, 0, static_cast<int>(m_PointVertices.size()));
            glBindVertexArray(0);
        }
        
        glUseProgram(0);
    }
    
    /**
     * @brief Draw a debug point
     * 
     * Adds a point to the debug rendering queue. Points are rendered
     * as single pixels or small squares depending on OpenGL point size settings.
     * 
     * @param position Position of the point in world coordinates
     * @param color Color of the point (default: white)
     * @param size Size of the point (default: 2.0f)
     */
    void DebugRenderer::DrawPoint(const Vector2D& position, const Color& color, float size) {
        AddPoint(position, color, size);
    }
    
    /**
     * @brief Draw a debug line
     * 
     * Adds a line to the debug rendering queue. Lines are rendered
     * as connected line segments between two points.
     * 
     * @param start Starting position of the line
     * @param end Ending position of the line
     * @param color Color of the line (default: white)
     * @param thickness Thickness of the line (currently not implemented)
     */
    void DebugRenderer::DrawLine(const Vector2D& start, const Vector2D& end, const Color& color, float thickness) {
        AddLine(start, end, color);
        (void)thickness; // Line thickness is handled by OpenGL line width
    }
    
    /**
     * @brief Draw a debug rectangle
     * 
     * Adds a rectangle to the debug rendering queue. The rectangle can be
     * drawn as either a filled shape or an outline.
     * 
     * For filled rectangles: Uses two triangles to create a solid rectangle
     * For outline rectangles: Uses four line segments to create a wireframe
     * 
     * @param position Center position of the rectangle
     * @param size Size of the rectangle (width, height)
     * @param color Color of the rectangle (default: white)
     * @param filled Whether to draw filled or outline (default: false)
     */
    void DebugRenderer::DrawRectangle(const Vector2D& position, const Vector2D& size, const Color& color, bool filled) {
        Vector2D halfSize = size.Scale(0.5f);
        Vector2D topLeft = position.Subtract(halfSize);
        Vector2D bottomRight = position.Add(halfSize);
        
        if (filled) {
            // Draw as two triangles for filled rectangle
            AddTriangle(topLeft, Vector2D(bottomRight.x, topLeft.y), bottomRight, color);
            AddTriangle(topLeft, bottomRight, Vector2D(topLeft.x, bottomRight.y), color);
        } else {
            // Draw as outline using four line segments
            AddLine(topLeft, Vector2D(bottomRight.x, topLeft.y), color);
            AddLine(Vector2D(bottomRight.x, topLeft.y), bottomRight, color);
            AddLine(bottomRight, Vector2D(topLeft.x, bottomRight.y), color);
            AddLine(Vector2D(topLeft.x, bottomRight.y), topLeft, color);
        }
    }
    
    /**
     * @brief Draw a debug circle
     * 
     * Adds a circle to the debug rendering queue. The circle can be
     * drawn as either a filled shape or an outline.
     * 
     * Circles are approximated using line segments or triangles.
     * More segments provide smoother circles but use more vertices.
     * 
     * @param center Center position of the circle
     * @param radius Radius of the circle
     * @param color Color of the circle (default: white)
     * @param filled Whether to draw filled or outline (default: false)
     * @param segments Number of segments for circle approximation (default: 32)
     */
    void DebugRenderer::DrawCircle(const Vector2D& center, float radius, const Color& color, bool filled, int segments) {
        auto points = GenerateCirclePoints(center, radius, segments);
        
        if (filled) {
            // Draw as triangle fan for filled circle
            for (int i = 0; i < segments; ++i) {
                int next = (i + 1) % segments;
                AddTriangle(center, points[i], points[next], color);
            }
        } else {
            // Draw as line loop for outline circle
            for (int i = 0; i < segments; ++i) {
                int next = (i + 1) % segments;
                AddLine(points[i], points[next], color);
            }
        }
    }
    
    void DebugRenderer::SetupBuffers() {
        // Setup line VAO/VBO
        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);
        
        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        
        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
        
        // Setup triangle VAO/VBO
        glGenVertexArrays(1, &m_TriangleVAO);
        glGenBuffers(1, &m_TriangleVBO);
        
        glBindVertexArray(m_TriangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);
        
        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
        
        // Setup point VAO/VBO
        glGenVertexArrays(1, &m_PointVAO);
        glGenBuffers(1, &m_PointVBO);
        
        glBindVertexArray(m_PointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_PointVBO);
        
        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    void DebugRenderer::CleanupBuffers() {
        if (m_LineVAO != 0) {
            glDeleteVertexArrays(1, &m_LineVAO);
            glDeleteBuffers(1, &m_LineVBO);
            m_LineVAO = m_LineVBO = 0;
        }
        
        if (m_TriangleVAO != 0) {
            glDeleteVertexArrays(1, &m_TriangleVAO);
            glDeleteBuffers(1, &m_TriangleVBO);
            m_TriangleVAO = m_TriangleVBO = 0;
        }
        
        if (m_PointVAO != 0) {
            glDeleteVertexArrays(1, &m_PointVAO);
            glDeleteBuffers(1, &m_PointVBO);
            m_PointVAO = m_PointVBO = 0;
        }
    }
    
    void DebugRenderer::CreateDebugShader() {
        // Vertex shader source
        std::string vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec4 aColor;
            
            uniform mat4 u_ViewProjection;
            
            out vec4 VertexColor;
            
            void main()
            {
                gl_Position = u_ViewProjection * vec4(aPos, 0.0, 1.0);
                VertexColor = aColor;
            }
        )";
        
        // Fragment shader source
        std::string fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec4 VertexColor;
            
            void main()
            {
                FragColor = VertexColor;
            }
        )";
        
        // Compile vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSource = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        
        // Check vertex shader compilation
        int vertexSuccess;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
        if (!vertexSuccess) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR: Debug vertex shader compilation failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            return;
        }
        
        // Compile fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSource = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        
        // Check fragment shader compilation
        int fragmentSuccess;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
        if (!fragmentSuccess) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR: Debug fragment shader compilation failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }
        
        // Create shader program
        m_DebugShaderProgram = glCreateProgram();
        glAttachShader(m_DebugShaderProgram, vertexShader);
        glAttachShader(m_DebugShaderProgram, fragmentShader);
        glLinkProgram(m_DebugShaderProgram);
        
        // Check program linking
        int linkSuccess;
        glGetProgramiv(m_DebugShaderProgram, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            char infoLog[512];
            glGetProgramInfoLog(m_DebugShaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR: Debug shader program linking failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(m_DebugShaderProgram);
            m_DebugShaderProgram = 0;
            return;
        }
        
        // Clean up shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
    }
    
    void DebugRenderer::AddLine(const Vector2D& start, const Vector2D& end, const Color& color) {
        glm::vec4 glmColor(color.r, color.g, color.b, color.a);
        
        m_LineVertices.emplace_back(glm::vec2(start.x, start.y), glmColor);
        m_LineVertices.emplace_back(glm::vec2(end.x, end.y), glmColor);
    }
    
    void DebugRenderer::AddTriangle(const Vector2D& p1, const Vector2D& p2, const Vector2D& p3, const Color& color) {
        glm::vec4 glmColor(color.r, color.g, color.b, color.a);
        
        m_TriangleVertices.emplace_back(glm::vec2(p1.x, p1.y), glmColor);
        m_TriangleVertices.emplace_back(glm::vec2(p2.x, p2.y), glmColor);
        m_TriangleVertices.emplace_back(glm::vec2(p3.x, p3.y), glmColor);
    }
    
    void DebugRenderer::AddPoint(const Vector2D& position, const Color& color, float size) {
        glm::vec4 glmColor(color.r, color.g, color.b, color.a);
        m_PointVertices.emplace_back(glm::vec2(position.x, position.y), glmColor);
        (void)size; // Point size is handled by OpenGL
    }
    
    std::vector<Vector2D> DebugRenderer::GenerateCirclePoints(const Vector2D& center, float radius, int segments) {
        std::vector<Vector2D> points;
        points.reserve(segments);
        
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float x = center.x + radius * std::cos(angle);
            float y = center.y + radius * std::sin(angle);
            points.emplace_back(x, y);
        }
        
        return points;
    }
    
} // namespace GP2Engine