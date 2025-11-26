/**
 * @file Shader.cpp
 * @brief Shader class implementation for OpenGL shader management
 * @author Asri (100%)
 * 
 * This file contains the implementation of the Shader class which provides
 * OpenGL shader compilation, linking, and uniform management.
 */

#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

namespace GP2Engine {
    
    /**
     * @brief Constructor - reads and builds shader from files
     * 
     * Creates a shader program by reading vertex and fragment shader files,
     * compiling them, and linking them into a complete shader program.
     * 
     * The process:
     * 1. Read vertex shader source from file
     * 2. Read fragment shader source from file
     * 3. Compile both shaders
     * 4. Link them into a shader program
     * 5. Store the program ID for later use
     * 
     * @param vertexPath Path to vertex shader file
     * @param fragmentPath Path to fragment shader file
     */
    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
        : m_VertexFilePath(vertexPath), m_FragmentFilePath(fragmentPath), m_RendererID(0)
    {
        // Read shader source code from files
        std::string vertexSource = ParseShader(vertexPath);
        std::string fragmentSource = ParseShader(fragmentPath);
        
        // Create shader program from source code
        m_RendererID = CreateShader(vertexSource, fragmentSource);
    }
    
    /**
     * @brief Destructor - cleans up OpenGL shader program
     * 
     * Deletes the OpenGL shader program to free GPU resources.
     * This is automatically called when the shader object is destroyed.
     */
    Shader::~Shader() {
        glDeleteProgram(m_RendererID);
    }
    
    /**
     * @brief Bind/activate the shader program
     * 
     * Makes this shader program the active OpenGL shader program.
     * All subsequent rendering calls will use this shader until another
     * shader is bound or Unbind() is called.
     */
    void Shader::Bind() const {
        glUseProgram(m_RendererID);
    }
    
    /**
     * @brief Unbind/deactivate the shader program
     * 
     * Deactivates the current shader program. This is useful when
     * switching between different shaders or when finishing rendering.
     */
    void Shader::Unbind() const {
        glUseProgram(0);
    }
    
    /**
     * @brief Set integer uniform
     * 
     * Sets an integer uniform variable in the shader program.
     * The uniform location is cached for performance.
     * 
     * @param name Name of the uniform variable in the shader
     * @param value Integer value to set
     */
    void Shader::SetUniform1i(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }
    
    /**
     * @brief Set float uniform
     * 
     * Sets a float uniform variable in the shader program.
     * The uniform location is cached for performance.
     * 
     * @param name Name of the uniform variable in the shader
     * @param value Float value to set
     */
    void Shader::SetUniform1f(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }
    
    /**
     * @brief Set 2D float vector uniform
     * 
     * Sets a 2D float vector uniform variable in the shader program.
     * 
     * @param name Name of the uniform variable in the shader
     * @param v0 First component of the vector
     * @param v1 Second component of the vector
     */
    void Shader::SetUniform2f(const std::string& name, float v0, float v1) {
        glUniform2f(GetUniformLocation(name), v0, v1);
    }
    
    /**
     * @brief Set 3D float vector uniform
     * 
     * Sets a 3D float vector uniform variable in the shader program.
     * 
     * @param name Name of the uniform variable in the shader
     * @param v0 First component of the vector
     * @param v1 Second component of the vector
     * @param v2 Third component of the vector
     */
    void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
        glUniform3f(GetUniformLocation(name), v0, v1, v2);
    }
    
    /**
     * @brief Set 4D float vector uniform
     * 
     * Sets a 4D float vector uniform variable in the shader program.
     * 
     * @param name Name of the uniform variable in the shader
     * @param v0 First component of the vector
     * @param v1 Second component of the vector
     * @param v2 Third component of the vector
     * @param v3 Fourth component of the vector
     */
    void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
        glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    }
    
    /**
     * @brief Set 4x4 matrix uniform
     * 
     * Sets a 4x4 matrix uniform variable in the shader program.
     * This is commonly used for transformation matrices (model, view, projection).
     * 
     * @param name Name of the uniform variable in the shader
     * @param matrix 4x4 matrix to set
     */
    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
    
    /**
     * @brief Parse shader file and return source code
     * 
     * Reads a shader file from disk and returns its contents as a string.
     * Handles file opening errors and provides error messages.
     * 
     * @param filepath Path to the shader file
     * @return Shader source code as string
     */
    std::string Shader::ParseShader(const std::string& filepath) {
        std::ifstream stream(filepath);
        std::string line;
        std::stringstream ss;
        
        if (stream.is_open()) {
            // Read all lines from the file
            while (getline(stream, line)) {
                ss << line << '\n';
            }
            stream.close();
        } else {
            std::cerr << "ERROR: Could not open shader file: " << filepath << std::endl;
            return "";
        }
        
        return ss.str();
    }
    
    /**
     * @brief Compile shader from source code
     * 
     * Compiles a shader from source code and returns the shader ID.
     * Handles compilation errors and provides detailed error messages.
     * 
     * @param type Shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
     * @param source Shader source code
     * @return Compiled shader ID, or 0 if compilation failed
     */
    unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
        // Create shader object
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        
        // Set shader source and compile
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        
        // Check compilation status
        int result = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            // Get error message
            int length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);
            
            // Print error message
            std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
            std::cerr << message << std::endl;
            
            // Clean up failed shader
            glDeleteShader(id);
            return 0;
        }
        
        return id;
    }
    
    /**
     * @brief Create shader program from vertex and fragment shaders
     * 
     * Links vertex and fragment shaders into a complete shader program.
     * Handles linking errors and provides error messages.
     * 
     * @param vertexShader Vertex shader source code
     * @param fragmentShader Fragment shader source code
     * @return Linked shader program ID, or 0 if linking failed
     */
    unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
        // Create shader program
        unsigned int program = glCreateProgram();
        
        // Compile shaders
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
        
        // Attach shaders to program
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        
        // Link program
        glLinkProgram(program);
        
        // Validate program
        glValidateProgram(program);
        
        // Clean up individual shaders (they're now part of the program)
        glDeleteShader(vs);
        glDeleteShader(fs);
        
        return program;
    }
    
    /**
     * @brief Get uniform location with caching
     * 
     * Gets the location of a uniform variable in the shader program.
     * Locations are cached to avoid repeated OpenGL calls for performance.
     * 
     * @param name Name of the uniform variable
     * @return Uniform location, or -1 if not found
     */
    int Shader::GetUniformLocation(const std::string& name) {
        // Check cache first
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];
            
        // Get location from OpenGL
        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
            std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
            
        // Cache the location
        m_UniformLocationCache[name] = location;
        return location;
    }
    
} // namespace GP2Engine
