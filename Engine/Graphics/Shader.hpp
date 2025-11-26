/**
 * @file Shader.hpp
 * @brief Shader class for OpenGL shader management
 * @author Asri (100%)
 * 
 * This file contains the Shader class definition which provides
 * OpenGL shader compilation, linking, and uniform management.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace GP2Engine {
    
    /**
     * @brief Shader class for OpenGL shader management
     * 
     * Handles compilation and linking of vertex and fragment shaders.
     * Provides uniform management with caching for performance.
     * Supports common uniform types including matrices and vectors.
     * 
     * @author Asri (100%)
     */
    class Shader {
    public:
        /**
         * @brief Constructor reads and builds the shader
         * 
         * @param vertexPath Path to vertex shader file
         * @param fragmentPath Path to fragment shader file
         */
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        
        /**
         * @brief Destructor
         */
        ~Shader();
        
        /**
         * @brief Use/activate the shader
         */
        void Bind() const;
        
        /**
         * @brief Deactivate the shader
         */
        void Unbind() const;
        
        /**
         * @brief Set integer uniform
         * 
         * @param name Uniform name
         * @param value Integer value
         */
        void SetUniform1i(const std::string& name, int value);
        
        /**
         * @brief Set float uniform
         * 
         * @param name Uniform name
         * @param value Float value
         */
        void SetUniform1f(const std::string& name, float value);
        
        /**
         * @brief Set 2D float vector uniform
         * 
         * @param name Uniform name
         * @param v0 First component
         * @param v1 Second component
         */
        void SetUniform2f(const std::string& name, float v0, float v1);
        
        /**
         * @brief Set 3D float vector uniform
         * 
         * @param name Uniform name
         * @param v0 First component
         * @param v1 Second component
         * @param v2 Third component
         */
        void SetUniform3f(const std::string& name, float v0, float v1, float v2);
        
        /**
         * @brief Set 4D float vector uniform
         * 
         * @param name Uniform name
         * @param v0 First component
         * @param v1 Second component
         * @param v2 Third component
         * @param v3 Fourth component
         */
        void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
        
        /**
         * @brief Set 4x4 matrix uniform
         * 
         * @param name Uniform name
         * @param matrix 4x4 matrix
         */
        void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
        
    private:
        unsigned int m_RendererID;                                    ///< OpenGL shader program ID
        std::string m_VertexFilePath;                                 ///< Vertex shader file path
        std::string m_FragmentFilePath;                               ///< Fragment shader file path
        std::unordered_map<std::string, int> m_UniformLocationCache; ///< Uniform location cache
        
        /**
         * @brief Parse shader file
         * 
         * @param filepath Path to shader file
         * @return Shader source code
         */
        std::string ParseShader(const std::string& filepath);
        
        /**
         * @brief Compile shader
         * 
         * @param type Shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
         * @param source Shader source code
         * @return Compiled shader ID
         */
        unsigned int CompileShader(unsigned int type, const std::string& source);
        
        /**
         * @brief Create shader program
         * 
         * @param vertexShader Vertex shader source
         * @param fragmentShader Fragment shader source
         * @return Linked shader program ID
         */
        unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
        
        /**
         * @brief Get uniform location with caching
         * 
         * @param name Uniform name
         * @return Uniform location
         */
        int GetUniformLocation(const std::string& name);
    };
    
} // namespace GP2Engine
