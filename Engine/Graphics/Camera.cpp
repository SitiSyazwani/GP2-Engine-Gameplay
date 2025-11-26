/**
 * @file Camera.cpp
 * @brief Camera class implementation for 2D and 3D rendering operations
 * @author Asri (100%)
 * 
 * This file contains the implementation of the Camera class which provides
 * view and projection matrices for rendering operations. It supports both
 * orthographic (2D) and perspective (3D) projections with transform operations.
 */

#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace GP2Engine {
    
    /**
     * @brief Default constructor - creates camera with default orthographic projection
     * 
     * Initializes camera with default orthographic projection bounds (-10 to 10 on both axes).
     * This provides a standard 2D camera setup suitable for most 2D rendering scenarios.
     */
    Camera::Camera() {
        SetOrthographic(-10.0f, 10.0f, -10.0f, 10.0f);
    }
    
    /**
     * @brief Orthographic camera constructor
     * 
     * Creates a camera with custom orthographic projection bounds.
     * Useful for setting up specific viewport dimensions or coordinate systems.
     * 
     * @param left Left boundary of the orthographic projection
     * @param right Right boundary of the orthographic projection  
     * @param bottom Bottom boundary of the orthographic projection
     * @param top Top boundary of the orthographic projection
     */
    Camera::Camera(float left, float right, float bottom, float top) {
        SetOrthographic(left, right, bottom, top);
    }
    
    /**
     * @brief Set orthographic projection parameters
     * 
     * Configures the camera to use orthographic projection with the specified bounds.
     * Orthographic projection is ideal for 2D rendering as it maintains parallel lines
     * and doesn't have perspective distortion.
     * 
     * @param left Left boundary of the orthographic projection
     * @param right Right boundary of the orthographic projection
     * @param bottom Bottom boundary of the orthographic projection
     * @param top Top boundary of the orthographic projection
     */
    void Camera::SetOrthographic(float left, float right, float bottom, float top) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_Left = left;
        m_Right = right;
        m_Bottom = bottom;
        m_Top = top;
        m_ProjectionMatrixNeedsUpdate = true; // Mark matrix for recalculation
    }
    
    /**
     * @brief Set perspective projection parameters
     * 
     * Configures the camera to use perspective projection with the specified parameters.
     * Perspective projection is used for 3D rendering and provides realistic depth perception.
     * 
     * @param fov Field of view in degrees (typically 45-90 degrees)
     * @param aspectRatio Aspect ratio (width/height) of the viewport
     * @param nearPlane Distance to near clipping plane (must be > 0)
     * @param farPlane Distance to far clipping plane (must be > nearPlane)
     */
    void Camera::SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
        m_ProjectionType = ProjectionType::Perspective;
        m_FOV = fov;
        m_AspectRatio = aspectRatio;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
        m_ProjectionMatrixNeedsUpdate = true; // Mark matrix for recalculation
    }
    
    /**
     * @brief Set camera position using Vector2D
     * 
     * Sets the camera position in 2D space. The Z component remains unchanged.
     * This is useful for 2D camera movement while maintaining the current Z depth.
     * 
     * @param position 2D position vector
     */
    void Camera::SetPosition(const Vector2D& position) {
        m_Position = glm::vec3(position.x, position.y, m_Position.z);
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Set camera position using glm::vec3
     * 
     * Sets the camera position in 3D space. This allows full 3D camera positioning.
     * 
     * @param position 3D position vector
     */
    void Camera::SetPosition(const glm::vec3& position) {
        m_Position = position;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Set camera rotation around Z-axis (for 2D)
     * 
     * Sets the camera rotation around the Z-axis, which is the primary rotation
     * used in 2D rendering scenarios.
     * 
     * @param rotation Rotation angle in degrees
     */
    void Camera::SetRotation(float rotation) {
        m_Rotation.z = rotation;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Set camera rotation using 3D Euler angles
     * 
     * Sets the camera rotation using full 3D Euler angles for complete 3D camera control.
     * 
     * @param rotation 3D rotation vector (Euler angles in degrees)
     */
    void Camera::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Set camera zoom level
     * 
     * Sets the camera zoom factor. Values greater than 1.0 zoom in,
     * values less than 1.0 zoom out. The minimum zoom is clamped to 0.1
     * to prevent invalid zoom levels.
     * 
     * @param zoom Zoom factor (must be > 0)
     */
    //void Camera::SetZoom(float zoom) {
    //    m_Zoom = glm::max(0.1f, zoom); // Prevent negative or zero zoom
    //    m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    //}
    void Camera::SetZoom(float zoom) {
        m_Zoom = glm::max(0.1f, zoom);
        m_ProjectionMatrixNeedsUpdate = true; // Zoom affects projection
        m_ViewMatrixNeedsUpdate = true; // Zoom also affects view matrix for 2D cameras
    }
    
    /**
     * @brief Move camera by offset using Vector2D
     * 
     * Moves the camera by the specified 2D offset. This is useful for
     * smooth camera movement in 2D games.
     * 
     * @param offset 2D movement offset
     */
    void Camera::Move(const Vector2D& offset) {
        m_Position.x += offset.x;
        m_Position.y += offset.y;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Move camera by offset using glm::vec3
     * 
     * Moves the camera by the specified 3D offset. This allows full 3D camera movement.
     * 
     * @param offset 3D movement offset
     */
    void Camera::Move(const glm::vec3& offset) {
        m_Position += offset;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Rotate camera by delta angle
     * 
     * Rotates the camera by the specified delta angle around the Z-axis.
     * This is useful for smooth camera rotation animations.
     * 
     * @param deltaRotation Rotation delta in degrees
     */
    void Camera::Rotate(float deltaRotation) {
        m_Rotation.z += deltaRotation;
        m_ViewMatrixNeedsUpdate = true; // Mark view matrix for recalculation
    }
    
    /**
     * @brief Zoom camera by delta factor
     * 
     * Zooms the camera by the specified delta factor. This is useful for
     * smooth zoom animations.
     * 
     * @param deltaZoom Zoom delta factor
     */
    void Camera::Zoom(float deltaZoom) {
        SetZoom(m_Zoom + deltaZoom);
    }
    
    /**
     * @brief Convert screen coordinates to world coordinates
     * 
     * Transforms screen pixel coordinates to world space coordinates.
     * This is essential for mouse input handling and UI-to-world coordinate conversion.
     * 
     * The conversion process:
     * 1. Normalize screen coordinates to [-1, 1] range
     * 2. Apply inverse view-projection transformation
     * 3. Return world space coordinates
     * 
     * @param screenPos Screen position in pixels
     * @param screenSize Screen dimensions in pixels
     * @return World position
     */
    glm::vec2 Camera::ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize) const {
        // Update matrices if needed
        if (m_ViewMatrixNeedsUpdate) UpdateViewMatrix();
        if (m_ProjectionMatrixNeedsUpdate) UpdateProjectionMatrix();
        
        // Convert screen coordinates to normalized device coordinates (-1 to 1)
        glm::vec2 ndc;
        ndc.x = (screenPos.x / screenSize.x) * 2.0f - 1.0f;
        ndc.y = 1.0f - (screenPos.y / screenSize.y) * 2.0f; // Flip Y axis
        
        // Get inverse view-projection matrix
        glm::mat4 invViewProj = glm::inverse(GetViewProjectionMatrix());
        
        // Transform to world coordinates
        glm::vec4 worldPos = invViewProj * glm::vec4(ndc, 0.0f, 1.0f);
        
        return glm::vec2(worldPos.x, worldPos.y);
    }
    
    /**
     * @brief Convert world coordinates to screen coordinates
     * 
     * Transforms world space coordinates to screen pixel coordinates.
     * This is useful for positioning UI elements or determining screen visibility.
     * 
     * The conversion process:
     * 1. Apply view-projection transformation
     * 2. Convert to normalized device coordinates
     * 3. Scale to screen pixel coordinates
     * 
     * @param worldPos World position
     * @param screenSize Screen dimensions in pixels
     * @return Screen position in pixels
     */
    glm::vec2 Camera::WorldToScreen(const glm::vec2& worldPos, const glm::vec2& screenSize) const {
        // Update matrices if needed
        if (m_ViewMatrixNeedsUpdate) UpdateViewMatrix();
        if (m_ProjectionMatrixNeedsUpdate) UpdateProjectionMatrix();
        
        // Transform world coordinates to clip space
        glm::vec4 clipSpace = GetViewProjectionMatrix() * glm::vec4(worldPos, 0.0f, 1.0f);
        
        // Convert to normalized device coordinates
        glm::vec2 ndc = glm::vec2(clipSpace) / clipSpace.w;
        
        // Convert to screen coordinates
        glm::vec2 screenPos;
        screenPos.x = (ndc.x + 1.0f) * 0.5f * screenSize.x;
        screenPos.y = (1.0f - ndc.y) * 0.5f * screenSize.y; // Flip Y axis
        
        return screenPos;
    }
    
    /**
     * @brief Update the view matrix
     * 
     * Recalculates the view matrix based on current position, rotation, and zoom.
     * The view matrix transforms world coordinates to camera space.
     * 
     * The transformation order is:
     * 1. Translate by camera position
     * 2. Rotate by camera rotation
     * 3. Scale by camera zoom
     * 4. Invert to get view matrix
     */
    void Camera::UpdateViewMatrix() const {
        // Create transform matrix
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position);
        transform = glm::rotate(transform, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
        transform = glm::scale(transform, glm::vec3(m_Zoom, m_Zoom, 1.0f));
        
        // View matrix is the inverse of the transform
        m_ViewMatrix = glm::inverse(transform);
        m_ViewMatrixNeedsUpdate = false;
    }
    
    /**
     * @brief Update the projection matrix
     * 
     * Recalculates the projection matrix based on current projection type and parameters.
     * The projection matrix transforms camera space coordinates to clip space.
     * 
     * For orthographic projection: Creates a parallel projection with no perspective distortion.
     * For perspective projection: Creates a perspective projection with realistic depth perception.
     */
    void Camera::UpdateProjectionMatrix() const {
        if (m_ProjectionType == ProjectionType::Orthographic) {
            m_ProjectionMatrix = glm::ortho(m_Left, m_Right, m_Bottom, m_Top);
        } else {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
        }
        m_ProjectionMatrixNeedsUpdate = false;
    }
    
} // namespace GP2Engine