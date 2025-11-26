/**
 * @file EditorViewport.cpp
 * @author Adi (100%)
 * @brief Implementation of the editor viewport rendering and interaction
 *
 * Manages scene visualization through an ImGui viewport panel. Renders the scene
 * through framebuffer and displays it in a resizable ImGui window.
 * Handles camera controls, entity selection/dragging and asset drag-drop from
 * the asset browser.
 */

#include "EditorViewport.hpp"
#include <imgui.h>
#include <glad/glad.h>
#include <limits>

EditorViewport::~EditorViewport() {
    // Framebuffer automatically cleans up resources in its destructor
}

void EditorViewport::Initialize(GP2Engine::Registry* registry, GP2Engine::Camera* camera, int sceneWidth, int sceneHeight) {
    m_registry = registry;
    m_camera = camera;

    // Store scene dimensions from config
    m_sceneWidth = sceneWidth;
    m_sceneHeight = sceneHeight;

    // Create framebuffer at fixed scene resolution for texture rendering
    // Scene is always rendered at full resolution, then stretched to fit viewport panel
    if (!m_framebuffer.Create(sceneWidth, sceneHeight)) {
        LOG_ERROR("Failed to create viewport framebuffer!");
        return;
    }

    LOG_INFO("EditorViewport initialized - Framebuffer at " + std::to_string(sceneWidth) + "x" + std::to_string(sceneHeight));
}

void EditorViewport::Update(float deltaTime) {
    HandleInput(deltaTime);
}

void EditorViewport::Render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    // Get viewport info
    m_isHovered = ImGui::IsWindowHovered();
    m_isFocused = ImGui::IsWindowFocused();

    // Render scene to framebuffer
    RenderScene();

    // Get available space in viewport panel
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    // Store viewport panel bounds for mouse coordinate transformation
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    m_viewportPanelX = viewportPos.x;
    m_viewportPanelY = viewportPos.y;
    m_viewportPanelWidth = viewportPanelSize.x;
    m_viewportPanelHeight = viewportPanelSize.y;

    // Display the framebuffer texture - stretch to fill panel
    if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
        ImGui::Image((void*)(intptr_t)m_framebuffer.GetTextureID(), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

        RenderGizmo();

        // Drag-drop target for assets from AssetBrowser (only in edit mode)
        if (m_dragDropEnabled && ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                const char* path = (const char*)payload->Data;
                m_draggedTexturePath = std::string(path);
                LOG_INFO("Texture dropped onto viewport: " + m_draggedTexturePath);
            }
            ImGui::EndDragDropTarget();
        }
    }

    ImGui::End();
}

void EditorViewport::HandleInput(float deltaTime) {
    // Only handle camera controls if viewport is focused and controls are enabled
    if (!m_isFocused || !m_isHovered || !m_cameraControlsEnabled) {
        return;
    }

    //  Gizmo input handling
    GP2Engine::Vector2D sceneMousePos = GetSceneMousePosition();

    // Handle entity selection and gizmo auto-detection
    if (GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left)) {
        GP2Engine::EntityID pickedEntity = PickEntityAtPosition(sceneMousePos);

        if (pickedEntity != GP2Engine::INVALID_ENTITY) {
            *m_selectedEntityPtr = pickedEntity;
            m_wasEntityClicked = true;
            m_clickPosition = sceneMousePos;

            // AUTO-DETECT which gizmo tool to use based on click position
            AutoSelectGizmoTool(pickedEntity, sceneMousePos);

            LOG_INFO("Selected entity: " + std::to_string(pickedEntity) + " - Tool: " +
                (m_currentGizmoOperation == GizmoOperation::TRANSLATE ? "Move" :
                    m_currentGizmoOperation == GizmoOperation::ROTATE ? "Rotate" : "Scale"));
        }
        else {
            *m_selectedEntityPtr = GP2Engine::INVALID_ENTITY;
            m_wasEntityClicked = false;
        }
    }

    // Handle gizmo dragging for selected entity
    HandleGizmoDragging(sceneMousePos);

    // Reset dragging states on mouse release
    if (GP2Engine::Input::IsMouseButtonReleased(GP2Engine::MouseButton::Left)) {
        m_wasEntityClicked = false;
        m_isGizmoDragging = false;
        m_isDraggingEntity = false;
    }

    // Middle mouse button for panning
    if (GP2Engine::Input::IsMouseButtonHeld(GP2Engine::MouseButton::Middle)) {
        double mouseX, mouseY;
        GP2Engine::Input::GetMousePosition(mouseX, mouseY);
        GP2Engine::Vector2D currentMousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

        if (m_lastMousePos.x != 0.0f || m_lastMousePos.y != 0.0f) {
            GP2Engine::Vector2D delta = currentMousePos - m_lastMousePos;
            // Pan camera
            m_camera->Move(GP2Engine::Vector2D(-delta.x * m_cameraPanSpeed * deltaTime, -delta.y * m_cameraPanSpeed * deltaTime));
        }

        m_lastMousePos = currentMousePos;
    } else {
        // Reset mouse tracking when not dragging
        m_lastMousePos = GP2Engine::Vector2D(0.0f, 0.0f);
    }

    // Q/Z keys for zooming 
    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::Q)) {
        float currentZoom = m_camera->GetZoom();
        float newZoom = currentZoom + (m_cameraZoomSpeed * deltaTime * ZOOM_SPEED_MULTIPLIER); // Zoom in
        newZoom = GP2Engine::MathUtils::Clamp(newZoom, MIN_ZOOM, MAX_ZOOM);
        m_camera->SetZoom(newZoom);
    }

    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::Z)) {
        float currentZoom = m_camera->GetZoom();
        float newZoom = currentZoom - (m_cameraZoomSpeed * deltaTime * ZOOM_SPEED_MULTIPLIER); // Zoom out
        newZoom = GP2Engine::MathUtils::Clamp(newZoom, MIN_ZOOM, MAX_ZOOM);
        m_camera->SetZoom(newZoom);
    }

    // === MODIFIED: Only handle basic entity dragging if not using gizmo ===
    if (m_selectedEntityPtr && m_cameraControlsEnabled && m_currentGizmoOperation == GizmoOperation::TRANSLATE && !m_isGizmoDragging) {

        // Start dragging on left mouse press
        if (GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left)) {
            GP2Engine::EntityID pickedEntity = PickEntityAtPosition(sceneMousePos);

            if (pickedEntity != GP2Engine::INVALID_ENTITY) {
                *m_selectedEntityPtr = pickedEntity;
                m_isDraggingEntity = true;

                // Calculate drag offset (mouse position relative to entity position)
                if (auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(pickedEntity)) {
                    m_dragOffset = sceneMousePos - transform->position;
                }

                LOG_INFO("Selected entity: " + std::to_string(pickedEntity));
            } else {
                *m_selectedEntityPtr = GP2Engine::INVALID_ENTITY;
            }
        }

        // Drag entity while left mouse is held
        if (m_isDraggingEntity && GP2Engine::Input::IsMouseButtonHeld(GP2Engine::MouseButton::Left)) {
            if (*m_selectedEntityPtr != GP2Engine::INVALID_ENTITY) {
                if (auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr)) {
                    // Move entity to mouse position 
                    transform->position = sceneMousePos - m_dragOffset;
                }
            }
        }

        // Stop dragging on left mouse release
        if (GP2Engine::Input::IsMouseButtonReleased(GP2Engine::MouseButton::Left)) {
            m_isDraggingEntity = false;
        }
    }
}

void EditorViewport::AutoSelectGizmoTool(GP2Engine::EntityID entity, const GP2Engine::Vector2D& clickPos) {
    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(entity);
    if (!transform) {
        m_currentGizmoOperation = GizmoOperation::TRANSLATE;
        return;
    }

    auto* spriteComp = m_registry->GetComponent<GP2Engine::SpriteComponent>(entity);
    auto* textComp = m_registry->GetComponent<GP2Engine::TextComponent>(entity);

    // SCALING Consistent with RenderGizmo
    float inverseZoom = 1.0f / m_camera->GetZoom();
    float gizmoScale;
    float entityWidth = 0.0f;
    float entityHeight = 0.0f;

    if (spriteComp) {
        entityWidth = spriteComp->size.x * transform->scale.x;
        entityHeight = spriteComp->size.y * transform->scale.y;
    }
    else if (textComp && textComp->font) {
        entityWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale * transform->scale.x);
        entityHeight = textComp->font->GetFontSize() * textComp->scale * transform->scale.y;
    }

    if (entityWidth > 0.0f && entityHeight > 0.0f) {
        float entityMaxDimension = std::max(entityWidth, entityHeight);

        if (entityMaxDimension < 20.0f) {
            gizmoScale = inverseZoom * 0.8f; // Very small entity: Use smaller gizmo (0.8x normal size)

        }
        else if (entityMaxDimension < 50.0f) {
            gizmoScale = inverseZoom * 1.0f; // Small entity: Use normal size

        }
        else {
            gizmoScale = inverseZoom * 1.2f; // Large entity: Slightly larger gizmo

        }
    }
    else {
        // No sprite or text - use default scaling
        gizmoScale = inverseZoom;
    }

    // Convert positions to screen space
    glm::vec2 screenPos = m_camera->WorldToScreen(
        glm::vec2(transform->position.x, transform->position.y),
        glm::vec2(m_sceneWidth, m_sceneHeight)
    );
    float screenX = m_viewportPanelX + screenPos.x * (m_viewportPanelWidth / m_sceneWidth);
    float screenY = m_viewportPanelY + screenPos.y * (m_viewportPanelHeight / m_sceneHeight);

    glm::vec2 clickScreenPos = m_camera->WorldToScreen(
        glm::vec2(clickPos.x, clickPos.y),
        glm::vec2(m_sceneWidth, m_sceneHeight)
    );
    float clickScreenX = m_viewportPanelX + clickScreenPos.x * (m_viewportPanelWidth / m_sceneWidth);
    float clickScreenY = m_viewportPanelY + clickScreenPos.y * (m_viewportPanelHeight / m_sceneHeight);

    // Gizmo detection with consistent scaling
    float handleSize = 20.0f * gizmoScale;
    float offset_25 = 25.0f * gizmoScale;
    float offset_18 = 18.0f * gizmoScale;
    float rotationRadius = 25.0f * gizmoScale;
    float arrowLength = 30.0f * gizmoScale;
    float arrowThicknessTolerance = 5.0f * gizmoScale;

    // Scale handles (Highest priority - smallest targets)
    if (clickScreenX >= screenX + offset_25 - handleSize / 2 && clickScreenX <= screenX + offset_25 + handleSize / 2 &&
        clickScreenY >= screenY - handleSize / 2 && clickScreenY <= screenY + handleSize / 2) {
        m_currentGizmoOperation = GizmoOperation::SCALE;
        LOG_INFO("Selected SCALE tool (X handle)");
        return;
    }

    if (clickScreenX >= screenX - handleSize / 2 && clickScreenX <= screenX + handleSize / 2 &&
        clickScreenY >= screenY - offset_25 - handleSize / 2 && clickScreenY <= screenY - offset_25 + handleSize / 2) {
        m_currentGizmoOperation = GizmoOperation::SCALE;
        LOG_INFO("Selected SCALE tool (Y handle)");
        return;
    }

    if (clickScreenX >= screenX + offset_18 - handleSize / 2 && clickScreenX <= screenX + offset_18 + handleSize / 2 &&
        clickScreenY >= screenY - offset_18 - handleSize / 2 && clickScreenY <= screenY - offset_18 + handleSize / 2) {
        m_currentGizmoOperation = GizmoOperation::SCALE;
        LOG_INFO("Selected SCALE tool (Uniform handle)");
        return;
    }

    // Rotate gizmo - with adaptive tolerance for small entities
    float distToCenter = std::sqrtf(std::powf(clickScreenX - screenX, 2) + std::powf(clickScreenY - screenY, 2));

    // Adaptive tolerance: more forgiving for small entities
    float rotationTolerance = 8.0f * gizmoScale;
    if (entityWidth > 0.0f && entityHeight > 0.0f) {
        float entityMaxDimension = std::max(entityWidth, entityHeight);
        if (entityMaxDimension < 20.0f) {
            rotationTolerance = inverseZoom * 1.5f; // 50% more tolerance for very small entities
        }
        else if (entityMaxDimension < 35.0f) {
            rotationTolerance = inverseZoom * 1.25f; // 25% more tolerance for small entities
        }
    }

    if (distToCenter >= rotationRadius - rotationTolerance && distToCenter <= rotationRadius + rotationTolerance) {
        m_currentGizmoOperation = GizmoOperation::ROTATE;
        LOG_INFO("Selected ROTATE tool (clicked on blue circle)");
        return;
    }

    // Translate arrows (Lower priority - larger targets)
    if (clickScreenX >= screenX && clickScreenX <= screenX + arrowLength + arrowThicknessTolerance &&
        clickScreenY >= screenY - arrowThicknessTolerance && clickScreenY <= screenY + arrowThicknessTolerance) {
        m_currentGizmoOperation = GizmoOperation::TRANSLATE;
        LOG_INFO("Selected MOVE tool (clicked on red arrow)");
        return;
    }

    if (clickScreenX >= screenX - arrowThicknessTolerance && clickScreenX <= screenX + arrowThicknessTolerance &&
        clickScreenY >= screenY - arrowLength - arrowThicknessTolerance && clickScreenY <= screenY) {
        m_currentGizmoOperation = GizmoOperation::TRANSLATE;
        LOG_INFO("Selected MOVE tool (clicked on green arrow)");
        return;
    }

    //  Entity center area for small entities (Lowest priority)
    if (entityWidth > 0.0f && entityHeight > 0.0f) {
        float entityMaxDimension = std::max(entityWidth, entityHeight);

        if (entityMaxDimension < 20.0f) {
            // only very small entities, provided a generous click area around the center
            float centerTolerance = 15.0f * gizmoScale;
            if (std::abs(clickScreenX - screenX) <= centerTolerance &&
                std::abs(clickScreenY - screenY) <= centerTolerance) {
                m_currentGizmoOperation = GizmoOperation::TRANSLATE;
                LOG_INFO("Selected MOVE tool (small entity center)");
                return;
            }
        }
    }

    // Default fallback: Translate
    m_currentGizmoOperation = GizmoOperation::TRANSLATE;
    LOG_INFO("Selected MOVE tool (default)");
}


void EditorViewport::HandleGizmoDragging(const GP2Engine::Vector2D& sceneMousePos) {
    if (!m_wasEntityClicked || !m_selectedEntityPtr || *m_selectedEntityPtr == GP2Engine::INVALID_ENTITY)
        return;

    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
    if (!transform) return;

    // Start dragging check (unchanged)
    if (GP2Engine::Input::IsMouseButtonHeld(GP2Engine::MouseButton::Left) && !m_isGizmoDragging) {
        m_isGizmoDragging = true;
        m_gizmoDragStart = sceneMousePos;
        m_gizmoTransformStart = *transform;
    }

    // Handle dragging
    if (m_isGizmoDragging && GP2Engine::Input::IsMouseButtonHeld(GP2Engine::MouseButton::Left)) {
        GP2Engine::Vector2D delta = sceneMousePos - m_gizmoDragStart;

        switch (m_currentGizmoOperation) {
        case GizmoOperation::TRANSLATE:
            transform->position = m_gizmoTransformStart.position + delta;
            break;

        case GizmoOperation::ROTATE:
        {
            // Use the entity's position at the start of the drag as the rotation center
            GP2Engine::Vector2D center = m_gizmoTransformStart.position;

            // Get the starting and current mouse vectors relative to the center
            GP2Engine::Vector2D startVector = m_gizmoDragStart - center;
            GP2Engine::Vector2D currentVector = sceneMousePos - center;

            // Avoid division by zero and very small vectors
            if (startVector.length() > 1.0f && currentVector.length() > 1.0f) {
                // Normalize vectors
                const float minRotationRadius = 20.0f; // Enforce a minimum radius for rotation handle

                if (startVector.length() < minRotationRadius)
                    startVector = startVector.normalized() * minRotationRadius;

                if (currentVector.length() < minRotationRadius)
                    currentVector = currentVector.normalized() * minRotationRadius;

                // normalize
                startVector = startVector.normalized();
                currentVector = currentVector.normalized();

                // Calculate the angle using atan2
                float startAngle = std::atan2(startVector.y, startVector.x);
                float currentAngle = std::atan2(currentVector.y, currentVector.x);

                // Calculate the change in angle (radians to degrees)
                float angleChange = (currentAngle - startAngle) * (180.0f / 3.14159265358979323846f);

                // Apply the rotation with smoothing
                transform->rotation = m_gizmoTransformStart.rotation + angleChange;

                // rotation in 0-360 range
                if (transform->rotation < 0) transform->rotation += 360.0f;
                if (transform->rotation >= 360.0f) transform->rotation -= 360.0f;
            }
        }
        break;

        case GizmoOperation::SCALE:
        {
            // Calculate scale based on distance from center
            GP2Engine::Vector2D center = m_gizmoTransformStart.position;

            float startDistance = (m_gizmoDragStart - center).length();
            float currentDistance = (sceneMousePos - center).length();

            // Avoid division by zero
            if (startDistance > 0.1f) {
                float scaleFactor = currentDistance / startDistance;

                // Apply scaling with minimum limits
                transform->scale.x = std::max(0.1f, m_gizmoTransformStart.scale.x * scaleFactor);
                transform->scale.y = std::max(0.1f, m_gizmoTransformStart.scale.y * scaleFactor);
            }
        }
        break;
        }
    }
}


void EditorViewport::RenderGizmo() {
    if (!m_selectedEntityPtr || *m_selectedEntityPtr == GP2Engine::INVALID_ENTITY)
        return;

    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
    if (!transform) return;

    auto* spriteComp = m_registry->GetComponent<GP2Engine::SpriteComponent>(*m_selectedEntityPtr);
    auto* textComp = m_registry->GetComponent<GP2Engine::TextComponent>(*m_selectedEntityPtr);

    float inverseZoom = 1.0f / m_camera->GetZoom();

    // Smaller gizmos for smaller entities
    float gizmoScale;
    float entityWidth = 0.0f;
    float entityHeight = 0.0f;

    if (spriteComp) {
        entityWidth = spriteComp->size.x * transform->scale.x;
        entityHeight = spriteComp->size.y * transform->scale.y;
    }
    else if (textComp && textComp->font) {
        entityWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale * transform->scale.x);
        entityHeight = textComp->font->GetFontSize() * textComp->scale * transform->scale.y;
    }

    if (entityWidth > 0.0f && entityHeight > 0.0f) {
        float entityMaxDimension = std::max(entityWidth, entityHeight);

        if (entityMaxDimension < 20.0f) {
            // for very small entity, use smaller gizmo (0.8x normal size)
            gizmoScale = inverseZoom * 0.8f;
        }
        else if (entityMaxDimension < 50.0f) {
            // for small entity, use normal size
            gizmoScale = inverseZoom * 1.0f;
        }
        else {
            // for large entity, use slightly larger gizmo
            gizmoScale = inverseZoom * 1.2f;
        }
    }
    else {
        // No sprite or text - use default scaling
        gizmoScale = inverseZoom;
    }

    // Render different gizmo types with consistent scaling
    RenderTranslateGizmo(gizmoScale);
    RenderRotateGizmo(gizmoScale);
    RenderScaleGizmo(gizmoScale);
}

void EditorViewport::RenderTranslateGizmo(float gizmoScale) {
    if (!m_selectedEntityPtr || *m_selectedEntityPtr == GP2Engine::INVALID_ENTITY) return;

    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
    if (!transform) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    glm::vec2 screenPos = m_camera->WorldToScreen(
        glm::vec2(transform->position.x, transform->position.y),
        glm::vec2(m_sceneWidth, m_sceneHeight)
    );

    float screenX = m_viewportPanelX + screenPos.x * (m_viewportPanelWidth / m_sceneWidth);
    float screenY = m_viewportPanelY + screenPos.y * (m_viewportPanelHeight / m_sceneHeight);

    float arrowLength = 30.0f * gizmoScale;
    float thickness = 2.0f * gizmoScale;

    // X arrow (red) move
    drawList->AddLine(
        ImVec2(screenX, screenY),
        ImVec2(screenX + arrowLength, screenY),
        IM_COL32(255, 0, 0, 255), thickness
    );

    // Y arrow (green) move
    drawList->AddLine(
        ImVec2(screenX, screenY),
        ImVec2(screenX, screenY - arrowLength),
        IM_COL32(0, 255, 0, 255), thickness
    );
}

void EditorViewport::RenderRotateGizmo(float gizmoScale) {
    if (!m_selectedEntityPtr || *m_selectedEntityPtr == GP2Engine::INVALID_ENTITY) return;

    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
    if (!transform) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    glm::vec2 screenPos = m_camera->WorldToScreen(
        glm::vec2(transform->position.x, transform->position.y),
        glm::vec2(m_sceneWidth, m_sceneHeight)
    );

    float screenX = m_viewportPanelX + screenPos.x * (m_viewportPanelWidth / m_sceneWidth);
    float screenY = m_viewportPanelY + screenPos.y * (m_viewportPanelHeight / m_sceneHeight);

    float radius = 25.0f * gizmoScale;
    float thickness = 2.0f * gizmoScale;

    //rotation  circle
    drawList->AddCircle(
        ImVec2(screenX, screenY), radius,
        IM_COL32(0, 100, 255, 255), 0, thickness
    );
}

void EditorViewport::RenderScaleGizmo(float gizmoScale) {
    if (!m_selectedEntityPtr || *m_selectedEntityPtr == GP2Engine::INVALID_ENTITY) return;

    auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
    if (!transform) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    glm::vec2 screenPos = m_camera->WorldToScreen(
        glm::vec2(transform->position.x, transform->position.y),
        glm::vec2(m_sceneWidth, m_sceneHeight)
    );

    float screenX = m_viewportPanelX + screenPos.x * (m_viewportPanelWidth / m_sceneWidth);
    float screenY = m_viewportPanelY + screenPos.y * (m_viewportPanelHeight / m_sceneHeight);

    float handleSize = 12.0f * gizmoScale;
    float offset_25 = 25.0f * gizmoScale;
    float uniformOffset = 12.0f * gizmoScale;

    // X scale handle (red) move
    drawList->AddRectFilled(
        ImVec2(screenX + offset_25 - handleSize / 2, screenY - handleSize / 2),
        ImVec2(screenX + offset_25 + handleSize / 2, screenY + handleSize / 2),
        IM_COL32(255, 0, 0, 255)
    );

    // Y scale handle (green) move
    drawList->AddRectFilled(
        ImVec2(screenX - handleSize / 2, screenY - offset_25 - handleSize / 2),
        ImVec2(screenX + handleSize / 2, screenY - offset_25 + handleSize / 2),
        IM_COL32(0, 255, 0, 255)
    );

    // Uniform scale handle (blue) 
    drawList->AddRectFilled(
        ImVec2(screenX + uniformOffset - handleSize / 2, screenY - uniformOffset - handleSize / 2),
        ImVec2(screenX + uniformOffset + handleSize / 2, screenY - uniformOffset + handleSize / 2),
        IM_COL32(0, 100, 255, 255)
    );
}


void EditorViewport::RenderScene() {
    if (!m_registry || !m_camera) return;

    // Camera projection remains fixed to scene bounds, but position/zoom controlled externally
    m_camera->SetOrthographic(0.0f, (float)m_sceneWidth, (float)m_sceneHeight, 0.0f);

    // Bind framebuffer for texture rendering
    m_framebuffer.Bind();

    // Clear with scene background color
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up OpenGL state for 2D rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Render all entities at 1:1 scale
    m_renderSystem.Render(*m_registry, *m_camera);

    // Render debug collision boxes if enabled
    if (m_showCollisionBoxes && *m_showCollisionBoxes) {
        m_debugRenderer.RenderCollisionBoxes(*m_registry);
    }

    // Render selection outline (always visible when entity is selected)
    if (m_selectedEntityPtr && *m_selectedEntityPtr != GP2Engine::INVALID_ENTITY) {
        auto& renderer = GP2Engine::Renderer::GetInstance();
        auto& debugRenderer = renderer.GetDebugRenderer();

        auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(*m_selectedEntityPtr);
        auto* spriteComp = m_registry->GetComponent<GP2Engine::SpriteComponent>(*m_selectedEntityPtr);
        auto* textComp = m_registry->GetComponent<GP2Engine::TextComponent>(*m_selectedEntityPtr);

        if (transform) {
            debugRenderer.Begin();

            GP2Engine::Vector2D position;
            GP2Engine::Vector2D size;

            if (spriteComp) {
                // Sprite entity outline
                float baseSizeX = spriteComp->size.x * spriteComp->uvSize.x;
                float baseSizeY = spriteComp->size.y * spriteComp->uvSize.y;

                position = transform->position;
                size = GP2Engine::Vector2D(
                    baseSizeX * transform->scale.x,
                    baseSizeY * transform->scale.y
                );

                // Draw bright cyan outline for selected sprite entity
                debugRenderer.DrawRectangle(position, size, GP2Engine::Color::GetCyan(), false);
            }
            else if (textComp && textComp->font) {
                // Text entity outline
                float textWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale * transform->scale.x);
                float textHeight = textComp->font->GetFontSize() * textComp->scale * transform->scale.y;

                // Apply offset from TextComponent
                position = GP2Engine::Vector2D(
                    transform->position.x + textComp->offset.x,
                    transform->position.y + textComp->offset.y
                );
                size = GP2Engine::Vector2D(textWidth, textHeight);

                // Text is rendered from top-left, so adjust rectangle center position
                GP2Engine::Vector2D centerPos(
                    position.x + size.x * 0.5f,
                    position.y + size.y * 0.5f
                );

                // Draw bright cyan outline for selected text entity
                debugRenderer.DrawRectangle(centerPos, size, GP2Engine::Color::GetCyan(), false);
            }

            debugRenderer.Flush(renderer);
        }
    }

    // Unbind framebuffer
    m_framebuffer.Unbind();
}

void EditorViewport::RenderGrid() {
    // TODO: Implement grid rendering
}

void EditorViewport::RenderEntityLabels() {
    if (!m_registry) return;

    // Get draw list for overlay rendering
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Render entity names near their positions
    for (GP2Engine::EntityID entity : m_registry->GetActiveEntities()) {
        GP2Engine::Transform2D* transform = m_registry->GetComponent<GP2Engine::Transform2D>(entity);
        GP2Engine::Tag* tag = m_registry->GetComponent<GP2Engine::Tag>(entity);

        if (transform && tag) {
            // Convert world position to screen position
            ImVec2 screenPos(transform->position.x, transform->position.y);

            // Draw entity name
            std::string label = tag->name;
            drawList->AddText(screenPos, IM_COL32(255, 255, 0, 200), label.c_str());
        }
    }
}

GP2Engine::EntityID EditorViewport::PickEntityAtPosition(const GP2Engine::Vector2D& scenePos) {
    if (!m_registry) return GP2Engine::INVALID_ENTITY;

    // Iterate through all entities and check if mouse is within their bounds
    // Pick the top most entity (highest render layer)
    GP2Engine::EntityID pickedEntity = GP2Engine::INVALID_ENTITY;
    int highestRenderLayer = std::numeric_limits<int>::min();

    for (GP2Engine::EntityID entity : m_registry->GetActiveEntities()) {
        auto* transform = m_registry->GetComponent<GP2Engine::Transform2D>(entity);
        if (!transform) continue;

        auto* spriteComp = m_registry->GetComponent<GP2Engine::SpriteComponent>(entity);
        auto* textComp = m_registry->GetComponent<GP2Engine::TextComponent>(entity);

        // Skip entities with neither sprite nor text
        if (!spriteComp && !textComp) continue;

        // Calculate entity bounds based on component type
        float left, right, top, bottom;
        int renderLayer = 0;

        if (spriteComp) {
            // Sprite entity bounds
            GP2Engine::Vector2D entityPos = transform->position;
            GP2Engine::Vector2D entitySize(
                spriteComp->size.x * transform->scale.x,
                spriteComp->size.y * transform->scale.y
            );

            left = entityPos.x - entitySize.x * 0.5f;
            right = entityPos.x + entitySize.x * 0.5f;
            top = entityPos.y - entitySize.y * 0.5f;
            bottom = entityPos.y + entitySize.y * 0.5f;
            renderLayer = spriteComp->renderLayer;
        }
        else if (textComp && textComp->font) {
            // Text entity bounds
            // Calculate text dimensions
            float textWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale * transform->scale.x);
            float textHeight = textComp->font->GetFontSize() * textComp->scale * transform->scale.y;

            // Apply offset from TextComponent
            GP2Engine::Vector2D entityPos(
                transform->position.x + textComp->offset.x,
                transform->position.y + textComp->offset.y
            );

            // Text is rendered from top-left corner, so adjust bounds accordingly
            left = entityPos.x;
            right = entityPos.x + textWidth;
            top = entityPos.y;
            bottom = entityPos.y + textHeight;
            renderLayer = textComp->renderLayer;
        }
        else {
            continue; // Skip if text entity has no font
        }

        // Check if mouse is within bounds
        if (scenePos.x >= left && scenePos.x <= right &&
            scenePos.y >= top && scenePos.y <= bottom) {

            // Pick entity with highest render layer (drawn on top)
            if (renderLayer > highestRenderLayer) {
                pickedEntity = entity;
                highestRenderLayer = renderLayer;
            }
        }
    }

    return pickedEntity;
}

GP2Engine::Vector2D EditorViewport::GetSceneMousePosition() const {
    if (!m_camera) return GP2Engine::Vector2D(-1.0f, -1.0f);

    // Get mouse position in screen space
    double mouseX, mouseY;
    GP2Engine::Input::GetMousePosition(mouseX, mouseY);

    // Convert to viewport panel relative coordinates
    float panelX = static_cast<float>(mouseX) - m_viewportPanelX;
    float panelY = static_cast<float>(mouseY) - m_viewportPanelY;

    // Check if mouse is within viewport bounds
    if (panelX < 0 || panelY < 0 || panelX >= m_viewportPanelWidth || panelY >= m_viewportPanelHeight) {
        return GP2Engine::Vector2D(-1.0f, -1.0f);  // Outside viewport
    }

    // Normalize to 0-1 range within viewport panel
    float normalizedX = panelX / m_viewportPanelWidth;
    float normalizedY = panelY / m_viewportPanelHeight;

    // Convert to scene space coordinates
    float sceneX = normalizedX * static_cast<float>(m_sceneWidth);
    float sceneY = normalizedY * static_cast<float>(m_sceneHeight);

    // Use Camera's ScreenToWorld to properly account for zoom and pan
    glm::vec2 screenPos(sceneX, sceneY);
    glm::vec2 screenSize(m_sceneWidth, m_sceneHeight);
    glm::vec2 worldPos = m_camera->ScreenToWorld(screenPos, screenSize);

    return GP2Engine::Vector2D(worldPos.x, worldPos.y);
}
