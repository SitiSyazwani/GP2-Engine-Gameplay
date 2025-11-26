/**
 * @file EditorViewport.hpp
 * @author Adi (100%)
 * @brief Viewport for visualizing and manipulating the scene
 *
 * Renders all entities in the scene with visual feedback,
 * allows camera manipulation, and provides visual selection.
 */

#pragma once

#include <Engine.hpp>
#include "../DebugRender/EditorDebugRenderer.hpp"

class EditorViewport {
public:
    EditorViewport() = default;
    ~EditorViewport();

    // Gizmo controls
    enum class GizmoOperation { TRANSLATE, ROTATE, SCALE };
    enum class GizmoMode { LOCAL, WORLD };

    /**
     * @brief Initialize the viewport
     * @param registry Pointer to ECS registry
     * @param camera Pointer to editor camera
     * @param sceneWidth Fixed scene width from config
     * @param sceneHeight Fixed scene height from config
     */
    void Initialize(GP2Engine::Registry* registry, GP2Engine::Camera* camera, int sceneWidth, int sceneHeight);

    /**
     * @brief Update viewport state
     * @param deltaTime Time since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Enable or disable camera controls (for play mode)
     * @param enabled Whether camera controls should be active
     */
    void SetCameraControlsEnabled(bool enabled) { m_cameraControlsEnabled = enabled; }

    /**
     * @brief Enable or disable drag-drop from Asset Browser (for play mode)
     * @param enabled Whether drag-drop should be active
     */
    void SetDragDropEnabled(bool enabled) { m_dragDropEnabled = enabled; }

    /**
     * @brief Set selected entity pointer from ContentEditorUI
     * @param selectedEntity Pointer to selected entity ID
     */
    void SetSelectedEntityPtr(GP2Engine::EntityID* selectedEntity) { m_selectedEntityPtr = selectedEntity; }

    /**
     * @brief Set debug visualization flags
     * @param showCollisionBoxes Pointer to collision box visibility flag
     */
    void SetDebugVisualization(bool* showCollisionBoxes) { m_showCollisionBoxes = showCollisionBoxes; }

    /**
     * @brief Get dragged texture path (for receiving drops from AssetBrowser)
     */
    const std::string& GetDraggedTexturePath() const { return m_draggedTexturePath; }
    bool HasDraggedTexture() const { return !m_draggedTexturePath.empty(); }
    void ClearDraggedTexture() { m_draggedTexturePath.clear(); }

    /**
     * @brief Render the viewport window
     */
    void Render();

    /**
     * @brief Check if viewport is hovered by mouse
     */
    bool IsHovered() const { return m_isHovered; }

    /**
     * @brief Get mouse position in scene space (0-1024, 0-768)
     * @return Scene-space coordinates, or (-1,-1) if not in viewport
     */
    GP2Engine::Vector2D GetSceneMousePosition() const;


private:
    GP2Engine::Registry* m_registry = nullptr;
    GP2Engine::Camera* m_camera = nullptr;

    // Viewport state
    bool m_isHovered = false;
    bool m_isFocused = false;

    // Viewport panel bounds (updated each frame in Render)
    float m_viewportPanelX = 0.0f;
    float m_viewportPanelY = 0.0f;
    float m_viewportPanelWidth = 0.0f;
    float m_viewportPanelHeight = 0.0f;

    // Camera control
    GP2Engine::Vector2D m_lastMousePos{ 0.0f, 0.0f };
    float m_cameraPanSpeed = 300.0f;
    float m_cameraZoomSpeed = 0.1f;
    bool m_cameraControlsEnabled = true;
    bool m_dragDropEnabled = true;

    // Camera zoom limits
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    static constexpr float ZOOM_SPEED_MULTIPLIER = 10.0f;

    // Entity selection and dragging
    GP2Engine::EntityID* m_selectedEntityPtr = nullptr;
    bool m_isDraggingEntity = false;
    GP2Engine::Vector2D m_dragOffset{ 0.0f, 0.0f };

    // Drag-drop from AssetBrowser
    std::string m_draggedTexturePath;

    // Grid rendering
    bool m_showGrid = true;

    // Rendering system for scene entities
    GP2Engine::RenderSystem m_renderSystem;

    // Scene dimensions
    int m_sceneWidth = 1024;
    int m_sceneHeight = 768;

    // Framebuffer for viewport texture rendering
    GP2Engine::Framebuffer m_framebuffer;

    // Debug visualization
    EditorDebugRenderer m_debugRenderer;
    bool* m_showCollisionBoxes = nullptr;

    // Gizmo variables
    GizmoOperation m_currentGizmoOperation = GizmoOperation::TRANSLATE;
    GizmoMode m_currentGizmoMode = GizmoMode::WORLD;
    GP2Engine::Vector2D m_gizmoDragStart{ 0.0f, 0.0f };
    GP2Engine::Transform2D m_gizmoTransformStart;
    
    bool m_wasEntityClicked = false;// Gizmo dragging state
    bool m_isGizmoDragging = false;
    GP2Engine::Vector2D m_clickPosition{ 0.0f, 0.0f };

    /**
    * @brief Render translation gizmo for selected entity
    * @param gizmoScale Scale factor for gizmo size
    */
    void RenderTranslateGizmo(float gizmoScale = 1.0f);

    /**
     * @brief Render rotation gizmo for selected entity
     * @param gizmoScale Scale factor for gizmo size
    */
    void RenderRotateGizmo(float gizmoScale = 1.0f);

    /**
     * @brief Render scale gizmo for selected entity
    * @param gizmoScale Scale factor for gizmo size
     */
    void RenderScaleGizmo(float gizmoScale = 1.0f);

    /**
     * @brief Automatically select gizmo tool based on click position
    * @param entity Selected entity ID
    * @param clickPos Click position in scene coordinates
    */
    void AutoSelectGizmoTool(GP2Engine::EntityID entity, const GP2Engine::Vector2D& clickPos);

    /**
    * @brief Handle gizmo dragging interaction
     * @param sceneMousePos Current mouse position in scene coordinates
     */
    void HandleGizmoDragging(const GP2Engine::Vector2D& sceneMousePos);

    /**
     * @brief Handle viewport input (camera pan/zoom, entity selection/dragging)
     */
    void HandleInput(float deltaTime);

    /**
     * @brief Pick entity at scene position
     * @param scenePos Position in scene coordinates
     * @return EntityID of picked entity, or INVALID_ENTITY
     */
    GP2Engine::EntityID PickEntityAtPosition(const GP2Engine::Vector2D& scenePos);

    /**
     * @brief Render scene entities
     */
    void RenderScene();

    /**
     * @brief Render grid overlay
     */
    void RenderGrid();

    /**
     * @brief Render entity name labels
     */
    void RenderEntityLabels();

    /**
    * @brief Render all gizmos for selected entity
    */
    void RenderGizmo();

};
