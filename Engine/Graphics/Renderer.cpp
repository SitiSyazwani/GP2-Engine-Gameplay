/**
 * @file Renderer.cpp
 * @brief Main renderer implementation for 2D graphics operations
 * @author Asri (100%)
 * 
 * This file contains the implementation of the Renderer class which provides
 * the main rendering interface for 2D graphics operations. It handles sprite
 * rendering, batch rendering for performance, and integrates with the debug renderer.
 */

#include "Renderer.hpp"
#include "Sprite.hpp"
#include "Shader.hpp"
#include "DebugRenderer.hpp"
#include "Font.hpp"
#include "../ECS/Component.hpp"
#include <stdexcept>
#include <iostream>
#include <cmath>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declarations
namespace GP2Engine {
    class Renderer;
    struct DebugVertex;
}

// GLFW callbacks (outside namespace)
/**
 * @brief GLFW error callback
 * 
 * @param error Error code
 * @param description Error description
 */
static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

/**
 * @brief GLFW window resize callback
 * 
 * @param window GLFW window handle
 * @param width New window width
 * @param height New window height
 */
void GLFWResizeCallback(GLFWwindow* window, int width, int height) {
    (void)window; // Suppress unused parameter warning
    glViewport(0, 0, width, height);
    // Update the internal width and height through the instance
    auto& instance = GP2Engine::Renderer::GetInstance();
    instance.SetWindowSize(width, height);

    // Notify the application about the resize event
    // The application will handle camera updates
    instance.OnWindowResize(width, height);

}

namespace GP2Engine {
    
    // Initialize static members
    std::unique_ptr<Renderer> Renderer::s_Instance = nullptr;
    std::function<void(int, int)> Renderer::s_ResizeCallback = nullptr;
    // Static member for VSync state
    static bool s_VSyncEnabled = false;
    
    // Static shader source constants to reduce code duplication
    static const std::string BASIC_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        void main() {
            gl_Position = u_ViewProjection * u_Transform * vec4(aPos, 0.0, 1.0);
        }
    )";
    
    static const std::string BASIC_FRAGMENT_SHADER = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main() {
            FragColor = u_Color;
        }
    )";
    
    static const std::string TEXTURED_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        out vec2 TexCoord;
        void main() {
            gl_Position = u_ViewProjection * u_Transform * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    static const std::string TEXTURED_FRAGMENT_SHADER = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D u_Texture;
        uniform vec4 u_Color;
        void main() {
            vec4 texColor = texture(u_Texture, TexCoord);
            FragColor = texColor * u_Color;
        }
    )";
    
    static const std::string BATCH_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec4 aColor;
        layout (location = 3) in float aTextureIndex;
        uniform mat4 u_ViewProjection;
        out vec2 TexCoord;
        out vec4 Color;
        out float TextureIndex;
        void main() {
            gl_Position = u_ViewProjection * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
            Color = aColor;
            TextureIndex = aTextureIndex;
        }
    )";
    
    static const std::string BATCH_FRAGMENT_SHADER = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        in vec4 Color;
        in float TextureIndex;
        uniform sampler2D u_Textures[32];
        void main() {
            if (TextureIndex < 0.5) {
                FragColor = Color;
            } else {
                int index = int(TextureIndex);
                FragColor = texture(u_Textures[index], TexCoord) * Color;
            }
        }
    )";
    
    // Helper function to create shader program
    static unsigned int CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vs = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vs, NULL);
        glCompileShader(vertexShader);
        
        int success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR: Vertex shader compilation failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            return 0;
        }
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fs = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fs, NULL);
        glCompileShader(fragmentShader);
        
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR: Fragment shader compilation failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }
        
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "ERROR: Shader program linking failed:\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(program);
            return 0;
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }
    
    bool Renderer::Initialize(GLFWwindow* window) {
        if (s_Instance) {
            return true; // Already initialized
        }

        if (!window) {
            std::cerr << "Cannot initialize Renderer: window is null" << std::endl;
            return false;
        }

        s_Instance = std::unique_ptr<Renderer>(new Renderer());
        s_Instance->m_Window = window;

        // Get window size for internal tracking
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        s_Instance->SetWindowSize(width, height);

        glfwSetErrorCallback(GLFWErrorCallback);
        glfwSetFramebufferSizeCallback(window, GLFWResizeCallback);
        
        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        
        // Configure global OpenGL state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        

        // Set up screen-space camera (origin at top-left, like typical 2D graphics)
        // This allows using pixel coordinates directly
        s_Instance->m_Camera.SetOrthographic(0.0f, static_cast<float>(width),
                                           static_cast<float>(height), 0.0f);


        // Initialize debug renderer
        s_Instance->m_DebugRenderer = std::make_unique<DebugRenderer>();
        if (!s_Instance->m_DebugRenderer->Initialize()) {
            std::cerr << "Failed to initialize DebugRenderer" << std::endl;
            return false;
        }

        return true;
    }
    
    void Renderer::Shutdown() {
        if (s_Instance) {
            // Shutdown debug renderer
            if (s_Instance->m_DebugRenderer) {
                s_Instance->m_DebugRenderer->Shutdown();
                s_Instance->m_DebugRenderer.reset();
            }
            
            // Note: Window is owned by Application, so we don't destroy it here
            s_Instance->m_Window = nullptr;
            s_Instance.reset();
        }
    }
    
    Renderer& Renderer::GetInstance() {
        if (!s_Instance) {
            throw std::runtime_error("Renderer not initialized. Call Renderer::Initialize() first.");
        }
        return *s_Instance;
    }
    
    void Renderer::OnWindowResize(int width, int height) {
        // This method is called by the GLFW resize callback
        // Call the registered callback if it exists
        if (s_ResizeCallback) {
            s_ResizeCallback(width, height);
        }
    }
    
    void Renderer::SetResizeCallback(std::function<void(int, int)> callback) {
        s_ResizeCallback = callback;
    }
    
    void Renderer::Clear() const {
        // Set up basic OpenGL state for 2D rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D
        glDisable(GL_CULL_FACE);  // Disable face culling for 2D
        
        // Set viewport first
        int width, height;
        GetWindowSize(width, height);
        glViewport(0, 0, width, height);
        
        // Clear the screen with a distinct color for debugging
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // Slightly different blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    void Renderer::Present() const {
        // Flush debug rendering before presenting
        if (m_DebugRenderer) {
            m_DebugRenderer->Flush(*this);
        }
        
        glfwSwapBuffers(m_Window);
    }
    
    void Renderer::GetWindowSize(int& width, int& height) const {
        if (m_Window) {
            glfwGetWindowSize(m_Window, &width, &height);
        } else {
            width = m_Width;
            height = m_Height;
        }
    }
    
    void Renderer::SetCamera(const Camera& camera) {
        m_Camera = camera;
    }
    
    bool Renderer::ShouldClose() const {
        return m_Window ? glfwWindowShouldClose(m_Window) : true;
    }
    
    
    void Renderer::DrawSprite(const Sprite& sprite) {
        if (!sprite.IsVisible() || !sprite.GetTexture()) {
            return;
        }

        // Draw textured sprite
        auto position = sprite.GetPosition();
        auto size = sprite.GetSize();
        auto color = sprite.GetColor();
        auto texture = sprite.GetTexture();

        // Validate texture before drawing
        if (!texture->IsValid() || texture->GetTextureID() == 0) {
            std::cerr << "ERROR: Invalid texture in sprite!" << std::endl;
            // Fallback to colored quad
            DrawQuad(glm::vec2(position.x, position.y), glm::vec2(size.x, size.y),
                     glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Magenta for error
            return;
        }

        // Get source rect and flip texture coordinates if needed
        glm::vec4 texCoords = sprite.GetSourceRect();

        // Flip texture coordinates by swapping edges
        // texCoords format: (u0, v0, uWidth, vHeight)
        if (sprite.GetFlipX()) {
            // Swap left (u0) and right (u1) texture coordinates
            float u0 = texCoords.x;
            float u1 = texCoords.x + texCoords.z;
            float temp = u0;
            u0 = u1;
            u1 = temp;
            texCoords.x = u0;
            texCoords.z = u1 - u0; // Negative width
        }
        if (sprite.GetFlipY()) {
            // Swap top (v0) and bottom (v1) texture coordinates
            float v0 = texCoords.y;
            float v1 = texCoords.y + texCoords.w;
            float temp = v0;
            v0 = v1;
            v1 = temp;
            texCoords.y = v0;
            texCoords.w = v1 - v0; // Negative height
        }

        // Use textured quad renderer with the sprite's texture
        DrawTexturedQuad(glm::vec2(position.x, position.y), glm::vec2(size.x, size.y),
                         texture->GetTextureID(), texCoords, color);
    }

    void Renderer::DrawSprite(const Sprite& sprite, const Transform2D* transform) {
        if (!sprite.IsVisible() || !sprite.GetTexture()) {
            return;
        }

        auto texture = sprite.GetTexture();
        auto color = sprite.GetColor();

        // Validate texture before drawing
        if (!texture->IsValid() || texture->GetTextureID() == 0) {
            std::cerr << "ERROR: Invalid texture in sprite!" << std::endl;
            return;
        }

        // Use transform if provided, otherwise use sprite's internal transform
        Vector2D position;
        Vector2D size = sprite.GetSize();
        float rotation = 0.0f;

        if (transform) {
            position = transform->position;
            size.x *= transform->scale.x;
            size.y *= transform->scale.y;
            rotation = transform->rotation;
        } else {
            position = sprite.GetPosition();
            Vector2D spriteScale = sprite.GetScale();
            size.x *= spriteScale.x;
            size.y *= spriteScale.y;
            // sprite rotation would be here if supported
        }

        // Get source rect and flip texture coordinates if needed
        glm::vec4 texCoords = sprite.GetSourceRect();

        // Flip texture coordinates by swapping edges
        // texCoords format: (u0, v0, uWidth, vHeight)
        if (sprite.GetFlipX()) {
            // Swap left (u0) and right (u1) texture coordinates
            float u0 = texCoords.x;
            float u1 = texCoords.x + texCoords.z;
            float temp = u0;
            u0 = u1;
            u1 = temp;
            texCoords.x = u0;
            texCoords.z = u1 - u0; // Negative width
        }
        if (sprite.GetFlipY()) {
            // Swap top (v0) and bottom (v1) texture coordinates
            float v0 = texCoords.y;
            float v1 = texCoords.y + texCoords.w;
            float temp = v0;
            v0 = v1;
            v1 = temp;
            texCoords.y = v0;
            texCoords.w = v1 - v0; // Negative height
        }

        // Use textured quad renderer with rotation support
        DrawTexturedQuad(glm::vec2(position.x, position.y), glm::vec2(size.x, size.y), rotation,
                         texture->GetTextureID(), texCoords, color);
    }
    
    void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        // Simple working quad renderer using basic OpenGL
        static bool initialized = false;
        static unsigned int VAO, VBO, EBO, shaderProgram;
        
        if (!initialized) {
            // Create basic vertex data for a unit quad (centered at origin)
            float vertices[] = {
                // positions     
                -0.5f, -0.5f,   // bottom left
                 0.5f, -0.5f,   // bottom right
                 0.5f,  0.5f,   // top right
                -0.5f,  0.5f    // top left
            };
            
            unsigned int indices[] = {
                0, 1, 2,   // first triangle
                2, 3, 0    // second triangle
            };
            
            // Generate VAO and VBO
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            
            glBindVertexArray(VAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            
            // Position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            
            glBindVertexArray(0);
            
            // Create a basic shader inline
            shaderProgram = CreateShaderProgram(BASIC_VERTEX_SHADER, BASIC_FRAGMENT_SHADER);
            if (shaderProgram == 0) return;
            
            initialized = true;
        }
        
        // Create transformation matrix
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(position.x, position.y, 0.0f));
        transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));
        
        // Safety check: ensure shader is valid
        if (shaderProgram == 0) {
            std::cerr << "ERROR: Shader program is invalid (0)" << std::endl;
            return;
        }
        
        // Use shader and set uniforms
        glUseProgram(shaderProgram);
        
        // Use camera matrices properly
        glm::mat4 viewProjection = m_Camera.GetViewProjectionMatrix();
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_ViewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Transform"), 1, GL_FALSE, &transform[0][0]);
        glUniform4f(glGetUniformLocation(shaderProgram, "u_Color"), color.r, color.g, color.b, color.a);
        
        // Draw quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        static bool errorReported = false;
        if (error != GL_NO_ERROR && !errorReported) {
            std::cerr << "OpenGL Error during rendering: " << error << std::endl;
            errorReported = true;
        }
        
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Vector2D overloads
    void Renderer::DrawQuad(const Vector2D& position, const Vector2D& size, const glm::vec4& color) {
        DrawQuad(glm::vec2(position.x, position.y), glm::vec2(size.x, size.y), color);
    }

    void Renderer::DrawQuad(const Vector2D& position, const Vector2D& size, float rotation, const glm::vec4& color) {
        DrawQuad(glm::vec2(position.x, position.y), glm::vec2(size.x, size.y), rotation, color);
    }

    void Renderer::DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size,
                                   unsigned int textureID, const glm::vec4& texCoords, const glm::vec4& color) {
        // Textured quad renderer using OpenGL
        static bool initialized = false;
        static unsigned int VAO, VBO, EBO, shaderProgram;

        if (!initialized) {
            unsigned int indices[] = {
                0, 1, 2,   // first triangle
                2, 3, 0    // second triangle
            };

            // Generate VAO and VBO
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // Use GL_DYNAMIC_DRAW since we'll be updating UV coords every frame
            glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Texture coordinate attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

            // Create textured shader
            shaderProgram = CreateShaderProgram(TEXTURED_VERTEX_SHADER, TEXTURED_FRAGMENT_SHADER);
            if (shaderProgram == 0) return;

            initialized = true;
        }

        // Update vertex data with current UV coordinates from texCoords parameter
        // texCoords.x = u offset, texCoords.y = v offset, texCoords.z = u size, texCoords.w = v size
        float vertices[] = {
            // positions     // texture coords
            -0.5f, -0.5f,    texCoords.x, texCoords.y,                              // bottom left
             0.5f, -0.5f,    texCoords.x + texCoords.z, texCoords.y,                // bottom right
             0.5f,  0.5f,    texCoords.x + texCoords.z, texCoords.y + texCoords.w,  // top right
            -0.5f,  0.5f,    texCoords.x, texCoords.y + texCoords.w                 // top left
        };

        // Update VBO with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        
        // Create transformation matrix
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(position.x, position.y, 0.0f));
        transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Check if texture binding was successful
        GLenum bindError = glGetError();
        if (bindError != GL_NO_ERROR) {
            std::cerr << "ERROR: Texture binding failed: " << bindError << " (TextureID: " << textureID << ")" << std::endl;
        }
        
        // Use shader and set uniforms
        glUseProgram(shaderProgram);
        
        glm::mat4 viewProjection = m_Camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_ViewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Transform"), 1, GL_FALSE, &transform[0][0]);
        glUniform4f(glGetUniformLocation(shaderProgram, "u_Color"), color.r, color.g, color.b, color.a);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_Texture"), 0);
        
        
        // Draw textured quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        
        // Cleanup
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    // Rotation overloads
    void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color) {
        // Same as regular DrawQuad but with rotation added to the transformation matrix
        static bool initialized = false;
        static unsigned int VAO, VBO, EBO, shaderProgram;

        if (!initialized) {
            // Same initialization as regular DrawQuad...
            // Create basic vertex data for a unit quad (centered at origin)
            float vertices[] = {
                // positions
                -0.5f, -0.5f,   // bottom left
                 0.5f, -0.5f,   // bottom right
                 0.5f,  0.5f,   // top right
                -0.5f,  0.5f    // top left
            };

            unsigned int indices[] = {
                0, 1, 2,   // first triangle
                2, 3, 0    // second triangle
            };

            // Generate VAO and VBO
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);

            // Create a basic shader inline (same as regular DrawQuad)
            shaderProgram = CreateShaderProgram(BASIC_VERTEX_SHADER, BASIC_FRAGMENT_SHADER);
            if (shaderProgram == 0) return;
            
            initialized = true;
        }

        // Create transformation matrix WITH ROTATION
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(position.x, position.y, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));

        // Use shader and set uniforms
        glUseProgram(shaderProgram);

        glm::mat4 viewProjection = m_Camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_ViewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Transform"), 1, GL_FALSE, &transform[0][0]);
        glUniform4f(glGetUniformLocation(shaderProgram, "u_Color"), color.r, color.g, color.b, color.a);

        // Draw quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }


    void Renderer::DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                   unsigned int textureID, const glm::vec4& texCoords, const glm::vec4& color) {
        // Copy most of the existing DrawTexturedQuad but add rotation to transform
        static bool initialized = false;
        static unsigned int VAO, VBO, EBO, shaderProgram;

        if (!initialized) {
            unsigned int indices[] = {
                0, 1, 2,   // first triangle
                2, 3, 0    // second triangle
            };

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // Use GL_DYNAMIC_DRAW since we'll be updating UV coords every frame
            glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Texture coordinate attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

            // Create shaders
            shaderProgram = CreateShaderProgram(TEXTURED_VERTEX_SHADER, TEXTURED_FRAGMENT_SHADER);
            if (shaderProgram == 0) return;

            initialized = true;
        }

        // Update vertex data with current UV coordinates from texCoords parameter
        // texCoords.x = u offset, texCoords.y = v offset, texCoords.z = u size, texCoords.w = v size
        float vertices[] = {
            // positions     // texture coords
            -0.5f, -0.5f,    texCoords.x, texCoords.y,                              // bottom left
             0.5f, -0.5f,    texCoords.x + texCoords.z, texCoords.y,                // bottom right
             0.5f,  0.5f,    texCoords.x + texCoords.z, texCoords.y + texCoords.w,  // top right
            -0.5f,  0.5f,    texCoords.x, texCoords.y + texCoords.w                 // top left
        };

        // Update VBO with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Create transformation matrix WITH ROTATION
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(position.x, position.y, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));

        // Use shader
        glUseProgram(shaderProgram);

        // Set uniforms
        glm::mat4 viewProjection = m_Camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_ViewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Transform"), 1, GL_FALSE, &transform[0][0]);
        glUniform4f(glGetUniformLocation(shaderProgram, "u_Color"), color.r, color.g, color.b, color.a);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_Texture"), 0);

        // Draw quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void Renderer::BeginBatch() {
        m_BatchStarted = true;
        m_QuadVertices.clear();
        m_QuadVertices.reserve(MAX_VERTICES); // Pre-allocate for performance
        m_TextureSlots.clear();
        m_TextureSlots.reserve(MAX_TEXTURE_SLOTS);
        m_CurrentTextureSlot = 0;
        m_VertexBufferOffset = 0;
    }
    
    void Renderer::EndBatch() {
        if (m_BatchStarted) {
            FlushBatch();
            m_BatchStarted = false;
        }
    }
    
    
    void Renderer::DrawTexturedQuadBatch(const glm::vec2& position, const glm::vec2& size, float rotation,
                                        unsigned int textureID, const glm::vec4& texCoords, const glm::vec4& color) {
        // No frustum culling needed - all objects are within viewport
        
        // Helper function to check if texture is in batch
        auto isTextureInBatch = [this](unsigned int textureID) -> bool {
            for (unsigned int id : m_TextureSlots) {
                if (id == textureID) return true;
            }
            return false;
        };
        
        // Helper function to get texture slot
        auto getTextureSlot = [this](unsigned int textureID) -> int {
            // Check if texture is already in batch
            for (size_t i = 0; i < m_TextureSlots.size(); ++i) {
                if (m_TextureSlots[i] == textureID) {
                    return static_cast<int>(i);
                }
            }
            
            // Add new texture if there's room
            if (m_CurrentTextureSlot < MAX_TEXTURE_SLOTS) {
                m_TextureSlots.push_back(textureID);
                return m_CurrentTextureSlot++;
            }
            
            return -1; // No room
        };
        
        // Check if we need to flush the batch
        if (m_QuadVertices.size() >= MAX_QUADS * 4 || 
            (m_CurrentTextureSlot >= MAX_TEXTURE_SLOTS && !isTextureInBatch(textureID))) {
            FlushBatch();
        }
        
        // Find or add texture slot
        int textureSlot = getTextureSlot(textureID);
        if (textureSlot == -1) {
            // No room for new texture, flush and try again
            FlushBatch();
            textureSlot = getTextureSlot(textureID);
        }
        
        // Optimized: Direct vertex calculation (avoid matrix multiplication)
        float cosRot = cos(glm::radians(rotation));
        float sinRot = sin(glm::radians(rotation));
        float halfWidth = size.x * 0.5f;
        float halfHeight = size.y * 0.5f;
        
        // Calculate rotated corners directly
        glm::vec2 positions[4] = {
            glm::vec2(-halfWidth, -halfHeight), // bottom left
            glm::vec2( halfWidth, -halfHeight), // bottom right
            glm::vec2( halfWidth,  halfHeight), // top right
            glm::vec2(-halfWidth,  halfHeight)  // top left
        };
        
        // Apply rotation and translation
        for (int i = 0; i < 4; ++i) {
            float x = positions[i].x;
            float y = positions[i].y;
            positions[i].x = x * cosRot - y * sinRot + position.x;
            positions[i].y = x * sinRot + y * cosRot + position.y;
        }
        
        // Add vertices to batch
        for (int i = 0; i < 4; ++i) {
            QuadVertex vertex;
            vertex.position = positions[i];
            vertex.texCoords = glm::vec2(
                (i == 0 || i == 3) ? texCoords.x : texCoords.z, // left or right
                (i < 2) ? texCoords.y : texCoords.w             // top or bottom
            );
            vertex.color = color;
            vertex.textureIndex = static_cast<float>(textureSlot);
            m_QuadVertices.push_back(vertex);
        }
    }
    
    void Renderer::DrawQuadBatch(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color) {
        // No frustum culling needed - all objects are within viewport
        
        // Check if we need to flush the batch
        if (m_QuadVertices.size() >= MAX_QUADS * 4) {
            FlushBatch();
        }
        
        // Use texture slot 0 for colored quads (no texture)
        int textureSlot = 0;
        
        // Optimized: Direct vertex calculation (avoid matrix multiplication)
        float cosRot = cos(glm::radians(rotation));
        float sinRot = sin(glm::radians(rotation));
        float halfWidth = size.x * 0.5f;
        float halfHeight = size.y * 0.5f;
        
        // Calculate rotated corners directly
        glm::vec2 positions[4] = {
            glm::vec2(-halfWidth, -halfHeight), // bottom left
            glm::vec2( halfWidth, -halfHeight), // bottom right
            glm::vec2( halfWidth,  halfHeight), // top right
            glm::vec2(-halfWidth,  halfHeight)  // top left
        };
        
        // Apply rotation and translation
        for (int i = 0; i < 4; ++i) {
            float x = positions[i].x;
            float y = positions[i].y;
            positions[i].x = x * cosRot - y * sinRot + position.x;
            positions[i].y = x * sinRot + y * cosRot + position.y;
        }
        
        // Add vertices to batch
        for (int i = 0; i < 4; ++i) {
            QuadVertex vertex;
            vertex.position = positions[i];
            vertex.texCoords = glm::vec2(0.0f, 0.0f); // No texture coords for colored quads
            vertex.color = color;
            vertex.textureIndex = static_cast<float>(textureSlot);
            m_QuadVertices.push_back(vertex);
        }
    }
    
    
    void Renderer::FlushBatch() {
        if (m_QuadVertices.empty()) {
            return;
        }
        
        // Create batch rendering shader if not exists
        static bool batchShaderInitialized = false;
        static unsigned int batchShaderProgram = 0;
        static unsigned int batchVAO = 0, batchVBO = 0, batchEBO = 0;
        static int viewProjectionLocation = -1;
        static int textureLocations[32] = {-1};
        
        if (!batchShaderInitialized) {
            // Create batch rendering shader
            batchShaderProgram = CreateShaderProgram(BATCH_VERTEX_SHADER, BATCH_FRAGMENT_SHADER);
            if (batchShaderProgram == 0) return;
            
            // Create batch VAO/VBO/EBO
            glGenVertexArrays(1, &batchVAO);
            glGenBuffers(1, &batchVBO);
            glGenBuffers(1, &batchEBO);
            
            glBindVertexArray(batchVAO);
            glBindBuffer(GL_ARRAY_BUFFER, batchVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchEBO);
            
            // Generate indices for quads
            std::vector<unsigned int> indices;
            indices.reserve(MAX_INDICES);
            for (unsigned int i = 0; i < MAX_QUADS; ++i) {
                unsigned int baseIndex = i * 4;
                indices.insert(indices.end(), {
                    baseIndex, baseIndex + 1, baseIndex + 2,
                    baseIndex + 2, baseIndex + 3, baseIndex
                });
            }
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            
            // Vertex attributes
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, texCoords));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, color));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, textureIndex));
            glEnableVertexAttribArray(3);
            
            glBindVertexArray(0);
            
            // Cache uniform locations for performance
            viewProjectionLocation = glGetUniformLocation(batchShaderProgram, "u_ViewProjection");
            for (int i = 0; i < 32; ++i) {
                std::string uniformName = "u_Textures[" + std::to_string(i) + "]";
                textureLocations[i] = glGetUniformLocation(batchShaderProgram, uniformName.c_str());
            }
            
            batchShaderInitialized = true;
        }
        
        // Use batch shader
        glUseProgram(batchShaderProgram);
        
        // Set view projection matrix using cached location
        glm::mat4 viewProjection = m_Camera.GetViewProjectionMatrix();
        glUniformMatrix4fv(viewProjectionLocation, 1, GL_FALSE, &viewProjection[0][0]);
        
        // Bind textures using cached locations
        // Always bind a white texture to slot 0 for colored quads
        static unsigned int whiteTexture = 0;
        if (whiteTexture == 0) {
            glGenTextures(1, &whiteTexture);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            unsigned char whitePixel[] = {255, 255, 255, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        glUniform1i(textureLocations[0], 0);
        
        // Bind other textures using cached locations
        for (size_t i = 1; i < m_TextureSlots.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
            glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);
            glUniform1i(textureLocations[i], static_cast<GLint>(i));
        }
        
        // Upload vertex data with optimized usage hint
        glBindVertexArray(batchVAO);
        glBindBuffer(GL_ARRAY_BUFFER, batchVBO);
        glBufferData(GL_ARRAY_BUFFER, m_QuadVertices.size() * sizeof(QuadVertex), m_QuadVertices.data(), GL_STREAM_DRAW);
        
        // Draw all quads in one call
        int quadCount = static_cast<int>(m_QuadVertices.size() / 4);
        glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, 0);
        
        // Update performance counters
        m_DrawCallsThisFrame++;
        m_QuadsDrawnThisFrame += quadCount;
        
        glBindVertexArray(0);
        glUseProgram(0);
        
        // Clear for next frame
        m_QuadVertices.clear();
        m_TextureSlots.clear();
        m_CurrentTextureSlot = 0;
    }
    
    void Renderer::SetVSync(bool enabled) {
        s_VSyncEnabled = enabled;

        if (s_Instance && s_Instance->m_Window) {
            glfwSwapInterval(enabled ? 1 : 0);
            std::cout << "VSync " << (enabled ? "enabled" : "disabled") << std::endl;
        }
    }
    
    bool Renderer::IsVSyncEnabled() {
        return s_VSyncEnabled;
    }

    // ===================================================================
    // TEXT RENDERING IMPLEMENTATION
    // ===================================================================

    static const std::string TEXT_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
        out vec2 TexCoords;

        uniform mat4 u_Projection;

        void main() {
            gl_Position = u_Projection * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

    static const std::string TEXT_FRAGMENT_SHADER = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec4 textColor;

        void main() {
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
            color = textColor * sampled;
        }
    )";

    bool Renderer::InitializeTextRendering() {
        if (m_TextRenderingInitialized) {
            return true;
        }

        // Create text shader
        unsigned int textShaderProgram = CreateShaderProgram(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
        if (textShaderProgram == 0) {
            std::cerr << "ERROR::RENDERER: Failed to create text shader" << std::endl;
            return false;
        }

        // Wrap in Shader class (we'll create a simple wrapper)
        // For now, we'll store the program ID directly
        // Note: You might want to create a proper Shader object here

        // Create VAO and VBO for text rendering
        glGenVertexArrays(1, &m_TextVAO);
        glGenBuffers(1, &m_TextVBO);

        glBindVertexArray(m_TextVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Store shader program (we'll use it directly for now)
        glUseProgram(textShaderProgram);

        // Set up orthographic projection for text rendering
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_Width),
                                         0.0f, static_cast<float>(m_Height));
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "u_Projection"),
                          1, GL_FALSE, glm::value_ptr(projection));

        glUseProgram(0);

        // Create a simple shader wrapper to store the program
        struct SimpleTextShader {
            unsigned int programID;
        };
        // We'll store the program ID in m_TextShader by creating a minimal Shader-like object
        // For simplicity, we'll just track the program ID
        // In a real implementation, you'd want to properly integrate with your Shader class

        m_TextRenderingInitialized = true;
        std::cout << "Text rendering initialized successfully" << std::endl;

        return true;
    }

    void Renderer::DrawText(Font* font, const std::string& text, float x, float y,
                           float scale, const glm::vec4& color) {
        if (!font || !font->IsValid()) {
            std::cerr << "ERROR::RENDERER: Invalid font" << std::endl;
            return;
        }

        if (!m_TextRenderingInitialized) {
            if (!InitializeTextRendering()) {
                return;
            }
        }

        // Enable blending for text transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Activate corresponding render state
        // Note: We need to find the text shader program ID
        // For now, we'll retrieve it (this is a simplified approach)
        // In production, store the program ID as a member variable

        // Find or create text shader
        static unsigned int textShaderProgram = 0;
        if (textShaderProgram == 0) {
            textShaderProgram = CreateShaderProgram(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
        }

        glUseProgram(textShaderProgram);

        // Use camera projection matrix for consistent coordinate system with entities
        glm::mat4 projection = m_Camera.GetProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "u_Projection"),
                          1, GL_FALSE, glm::value_ptr(projection));

        glUniform4f(glGetUniformLocation(textShaderProgram, "textColor"),
                   color.r, color.g, color.b, color.a);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_TextVAO);

        // Iterate through all characters
        float xPos = x;
        for (const char& c : text) {
            const Character& ch = font->GetCharacter(c);

            if (ch.textureID == 0) {
                continue; // Skip invalid characters
            }

            float xpos = xPos + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            // Update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },

                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }
            };

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Now advance cursors for next glyph
            xPos += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        // Disable blending
        glDisable(GL_BLEND);

        m_DrawCallsThisFrame++;
    }

    void Renderer::DrawText(Font* font, const std::string& text, const glm::vec2& position,
                           float scale, const glm::vec4& color) {
        DrawText(font, text, position.x, position.y, scale, color);
    }

    void Renderer::DrawText(Font* font, const std::string& text, const Transform2D* transform,
                           float textScale, const glm::vec4& color) {
        if (!font || !font->IsValid() || !transform) {
            std::cerr << "ERROR::RENDERER: Invalid font or transform" << std::endl;
            return;
        }

        if (!m_TextRenderingInitialized) {
            if (!InitializeTextRendering()) {
                return;
            }
        }

        // Enable blending for text transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Find or create text shader
        static unsigned int textShaderProgram = 0;
        if (textShaderProgram == 0) {
            textShaderProgram = CreateShaderProgram(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
        }

        glUseProgram(textShaderProgram);

        // Use camera projection matrix for consistent coordinate system with entities
        glm::mat4 projection = m_Camera.GetProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "u_Projection"),
                          1, GL_FALSE, glm::value_ptr(projection));

        glUniform4f(glGetUniformLocation(textShaderProgram, "textColor"),
                   color.r, color.g, color.b, color.a);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_TextVAO);

        // Calculate rotation matrix
        float cosRot = cos(glm::radians(transform->rotation));
        float sinRot = sin(glm::radians(transform->rotation));

        // Combined scale from transform and text scale
        float finalScaleX = transform->scale.x * textScale;
        float finalScaleY = transform->scale.y * textScale;

        // Iterate through all characters
        float xOffset = 0.0f;
        for (const char& c : text) {
            const Character& ch = font->GetCharacter(c);

            if (ch.textureID == 0) {
                continue; // Skip invalid characters
            }

            // Character metrics with combined scale
            float xpos = xOffset + ch.bearing.x * finalScaleX;
            float ypos = -(ch.size.y - ch.bearing.y) * finalScaleY;

            float w = ch.size.x * finalScaleX;
            float h = ch.size.y * finalScaleY;

            // Character vertices (before rotation)
            glm::vec2 positions[4] = {
                glm::vec2(xpos,     ypos + h),  // Bottom-left (Y-down)
                glm::vec2(xpos,     ypos),      // Top-left (Y-down)
                glm::vec2(xpos + w, ypos),      // Top-right (Y-down)
                glm::vec2(xpos + w, ypos + h)   // Bottom-right (Y-down)
            };

            // Apply rotation around origin (0,0) and then translate to transform position
            for (int i = 0; i < 4; ++i) {
                float rotatedX = positions[i].x * cosRot - positions[i].y * sinRot;
                float rotatedY = positions[i].x * sinRot + positions[i].y * cosRot;
                positions[i].x = rotatedX + transform->position.x;
                positions[i].y = rotatedY + transform->position.y;
            }

            // Update VBO for each character
            // Texture V coordinates flipped for Y-down camera projection
            float vertices[6][4] = {
                { positions[0].x, positions[0].y,   0.0f, 1.0f },
                { positions[1].x, positions[1].y,   0.0f, 0.0f },
                { positions[2].x, positions[2].y,   1.0f, 0.0f },

                { positions[0].x, positions[0].y,   0.0f, 1.0f },
                { positions[2].x, positions[2].y,   1.0f, 0.0f },
                { positions[3].x, positions[3].y,   1.0f, 1.0f }
            };

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursor for next glyph
            xOffset += (ch.advance >> 6) * finalScaleX;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        // Disable blending
        glDisable(GL_BLEND);

        m_DrawCallsThisFrame++;
    }

    float Renderer::MeasureTextWidth(Font* font, const std::string& text, float scale) const {
        if (!font || !font->IsValid()) {
            return 0.0f;
        }

        return font->CalculateTextWidth(text, scale);
    }

    Renderer::~Renderer() {
        // Clean up text rendering resources
        if (m_TextVAO != 0) {
            glDeleteVertexArrays(1, &m_TextVAO);
        }
        if (m_TextVBO != 0) {
            glDeleteBuffers(1, &m_TextVBO);
        }

        // Other cleanup is handled by Shutdown()
    }

} // namespace GP2Engine
