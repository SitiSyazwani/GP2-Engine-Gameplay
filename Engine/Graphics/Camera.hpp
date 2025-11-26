/**
 * @file Camera.hpp
 * @brief Camera class for 2D and 3D rendering operations
 * @author Asri (100%)
 * 
 * This file contains the Camera class definition which provides view and projection
 * matrices for rendering operations. It supports both orthographic (2D) and 
 * perspective (3D) projections with transform operations.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Math/Vector2D.hpp"

namespace GP2Engine {
    
    /**
     * @brief Camera class for 2D and 3D rendering
     * 
     * Provides view and projection matrices for rendering operations.
     * Supports both orthographic (2D) and perspective (3D) projections.
     * Handles camera transformations including position, rotation, and 
     
     
     .
     * 
     
     */
    class Camera {
    public:
        /**
         * @brief Projection types supported by the camera
         */
        enum class ProjectionType {
            Orthographic,  ///< Orthographic projection for 2D rendering
            Perspective   ///< Perspective projection for 3D rendering
        };
        
        /**
         * @brief Default constructor
         * 
         * Creates a camera with default orthographic projection (-10 to 10 on both axes)
         */
        Camera();
        
        /**
         * @brief Orthographic camera constructor
         * 
         * @param left Left boundary of the orthographic projection
         * @param right Right boundary of the orthographic projection
         * @param bottom Bottom boundary of the orthographic projection
         * @param top Top boundary of the orthographic projection
         */
        Camera(float left, float right, float bottom, float top);
        
        /**
         * @brief Destructor
         */
        ~Camera() = default;
        
        /**
         * @brief Set orthographic projection parameters
         * 
         * @param left Left boundary of the orthographic projection
         * @param right Right boundary of the orthographic projection
         * @param bottom Bottom boundary of the orthographic projection
         * @param top Top boundary of the orthographic projection
         */
        void SetOrthographic(float left, float right, float bottom, float top);
        
        /**
         * @brief Set perspective projection parameters
         * 
         * @param fov Field of view in degrees
         * @param aspectRatio Aspect ratio (width/height)
         * @param nearPlane Near clipping plane distance
         * @param farPlane Far clipping plane distance
         */
        void SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
        
        /**
         * @brief Set camera position using Vector2D
         * 
         * @param position 2D position vector
         */
        void SetPosition(const Vector2D& position);
        
        /**
         * @brief Set camera position using glm::vec3
         * 
         * @param position 3D position vector
         */
        void SetPosition(const glm::vec3& position);
        
        /**
         * @brief Set camera rotation around Z-axis (for 2D)
         * 
         * @param rotation Rotation angle in degrees
         */
        void SetRotation(float rotation);
        
        /**
         * @brief Set camera rotation using 3D Euler angles
         * 
         * @param rotation 3D rotation vector (Euler angles in degrees)
         */
        void SetRotation(const glm::vec3& rotation);
        
        /**
         * @brief Set camera zoom level
         * 
         * @param zoom Zoom factor (must be > 0)
         */
        void SetZoom(float zoom);
        
        /**
         * @brief Move camera by offset using Vector2D
         * 
         * @param offset 2D movement offset
         */
        void Move(const Vector2D& offset);
        
        /**
         * @brief Move camera by offset using glm::vec3
         * 
         * @param offset 3D movement offset
         */
        void Move(const glm::vec3& offset);
        
        /**
         * @brief Rotate camera by delta angle
         * 
         * @param deltaRotation Rotation delta in degrees
         */
        void Rotate(float deltaRotation);
        
        /**
         * @brief Zoom camera by delta factor
         * 
         * @param deltaZoom Zoom delta factor
         */
        void Zoom(float deltaZoom);
        
        /**
         * @brief Get the view matrix
         * 
         * @return Reference to the view matrix
         */
        const glm::mat4& GetViewMatrix() const { 
            if (m_ViewMatrixNeedsUpdate) UpdateViewMatrix();
            return m_ViewMatrix; 
        }
        
        /**
         * @brief Get the projection matrix
         * 
         * @return Reference to the projection matrix
         */
        const glm::mat4& GetProjectionMatrix() const { 
            if (m_ProjectionMatrixNeedsUpdate) UpdateProjectionMatrix();
            return m_ProjectionMatrix; 
        }
        
        /**
         * @brief Get the combined view-projection matrix
         * 
         * @return Combined view-projection matrix
         */
        glm::mat4 GetViewProjectionMatrix() const { 
            if (m_ViewMatrixNeedsUpdate) UpdateViewMatrix();
            if (m_ProjectionMatrixNeedsUpdate) UpdateProjectionMatrix();
            return m_ProjectionMatrix * m_ViewMatrix; 
        }
        
        /**
         * @brief Get camera position
         * 
         * @return Reference to the position vector
         */
        const glm::vec3& GetPosition() const { return m_Position; }
        
        /**
         * @brief Get camera rotation
         * 
         * @return Reference to the rotation vector
         */
        const glm::vec3& GetRotation() const { return m_Rotation; }
        
        /**
         * @brief Get camera zoom level
         * 
         * @return Current zoom factor
         */
        float GetZoom() const { return m_Zoom; }
        
        /**
         * @brief Get projection type
         * 
         * @return Current projection type
         */
        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        
        /**
         * @brief Convert screen coordinates to world coordinates
         * 
         * @param screenPos Screen position
         * @param screenSize Screen dimensions
         * @return World position
         */
        glm::vec2 ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize) const;
        
        /**
         * @brief Convert world coordinates to screen coordinates
         * 
         * @param worldPos World position
         * @param screenSize Screen dimensions
         * @return Screen position
         */
        glm::vec2 WorldToScreen(const glm::vec2& worldPos, const glm::vec2& screenSize) const;
        
    private:
        // Transform properties
        glm::vec3 m_Position{0.0f, 0.0f, 0.0f};     ///< Camera position in world space
        glm::vec3 m_Rotation{0.0f, 0.0f, 0.0f};     ///< Camera rotation (Euler angles in degrees)
        float m_Zoom{1.0f};                          ///< Camera zoom factor
        
        // Projection properties
        ProjectionType m_ProjectionType{ProjectionType::Orthographic}; ///< Current projection type
        
        // Orthographic properties
        float m_Left{-10.0f}, m_Right{10.0f}, m_Bottom{-10.0f}, m_Top{10.0f}; ///< Orthographic bounds
        
        // Perspective properties
        float m_FOV{45.0f}, m_AspectRatio{1.0f}, m_NearPlane{0.1f}, m_FarPlane{100.0f}; ///< Perspective parameters
        
        // Matrices
        mutable glm::mat4 m_ViewMatrix{1.0f};        ///< View matrix
        mutable glm::mat4 m_ProjectionMatrix{1.0f};  ///< Projection matrix
        mutable bool m_ViewMatrixNeedsUpdate{true}; ///< Flag indicating view matrix needs update
        mutable bool m_ProjectionMatrixNeedsUpdate{true}; ///< Flag indicating projection matrix needs update
        
        /**
         * @brief Update the view matrix
         * 
         * Recalculates the view matrix based on current position, rotation, and zoom
         */
        void UpdateViewMatrix() const;
        
        /**
         * @brief Update the projection matrix
         * 
         * Recalculates the projection matrix based on current projection type and parameters
         */
        void UpdateProjectionMatrix() const;
    };
    
} // namespace GP2Engine