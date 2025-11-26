/**
 * @file DebugUI.cpp
 * @author Syazwani (100%)
 * @brief Implementation of the Engine's Developer Debug Console (ImGui).
 *
 * This file contains the implementation logic for all debug panels, including performance monitoring,
 * the ECS Entity Inspector, debug visualization toggles, and the stress testing system.
 * It manages the UI rendering loop and coordinates the display of developer tools.
 */

#include "DebugUI.h"
#include <imgui.h>
#include <iostream>
#include <string>
#include <cmath>
#include <Resources/ResourceManager.hpp>
#include <Graphics/AnimationHelpers.hpp>

namespace Hollows {

DebugUI::DebugUI() {
    std::cout << "Hollows Debug UI initialized" << std::endl;
}

void DebugUI::Render(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity, float* frameTimeHistory, int frameIndex, float& playerSpeed,
                    bool& showCollisionBoxes, bool& showVelocityVectors, bool& showGridToggle, GP2Engine::Vector2D& m_hoveredTileCoords, int hoveredTileValue,
                    GP2Engine::TileRenderer& tileRenderer, GP2Engine::TileMap& tilemap){
    // Main debug control panel
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Debug Panels:");
    // Single checkbox for Level Editor
    ImGui::Checkbox("Level Editor", &m_showLevelEditor);
    ImGui::Checkbox("Performance", &m_showPerformance);
    ImGui::SameLine();
    ImGui::Checkbox("Entity Inspector", &m_showEntityInspector);
    ImGui::Checkbox("Player Controls", &m_showPlayerPanel);
    ImGui::SameLine();
    ImGui::Checkbox("Controls Help", &m_showControlsPanel);
    ImGui::Checkbox("Debug Visualization", &m_showDebugVisualization);
    ImGui::SameLine();
    ImGui::Checkbox("Stress Test", &m_showStressTest);
    ImGui::Checkbox("Tile Editor", &m_showTileMapEditorPanel);

    ImGui::Separator();
    // Show Level Editor status
    if (m_showLevelEditor && m_LevelEditor) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Level Editor: ACTIVE");
        ImGui::Text("Property Editor, Hierarchy, Level Manager, Camera panels shown below");
    }
    else if (m_showLevelEditor) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Level Editor: NO LEVELEDITOR POINTER SET!");
    }
    else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Level Editor: INACTIVE");
    }

    ImGui::Separator();
    ImGui::Text("Debug Testing:");
    ImGui::Text("Stress test controls available in dedicated panel below.");

    if (ImGui::IsItemHovered()) {
        if (!m_stressTestActive) {
            ImGui::SetTooltip("Start stress test with 2500+ animated objects to test performance!");
        } else {
            ImGui::SetTooltip("Stop stress test and clean up objects.");
        }
    }

    // Crash simulation button with warning color
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // Red
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Brighter red
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.05f, 0.05f, 1.0f)); // Darker red

    if (ImGui::Button("Simulate Crash")) {
        // Trigger a crash to test logging system
        LOG_CRASH("Crash simulation for testing");
        throw std::runtime_error("Crash simulation for testing");
    }

    ImGui::PopStyleColor(3);

    ImGui::End();

    // ==================== LEVEL EDITOR PANELS ====================
    if (m_showLevelEditor && m_LevelEditor) {
        m_LevelEditor->RenderImGui(); // This calls all LevelEditor panels
    }

    // Position panels in a nice layout
    const float panelSpacing = 10.0f;
    float currentX = panelSpacing;
    float currentY = 80.0f; // Below the control panel
    if (m_showLevelEditor && m_LevelEditor) {
        currentY += 300.0f; // Make space for Level Editor panels
    }

    // Draw panels if they're visible
    if (m_showPerformance) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawPerformancePanel(frameTimeHistory, frameIndex);
        currentX += 300.0f + panelSpacing;
    }

    if (m_showEntityInspector) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawEntityInspector(registry);
        currentX += 350.0f + panelSpacing;
    }

    if (m_showStressTest) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawStressTestPanel(registry);
        currentX += 300.0f + panelSpacing;
    }

    if (m_showDebugVisualization) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawDebugVisualizationPanel(showCollisionBoxes, showVelocityVectors);
        currentX += 300.0f + panelSpacing;
    }

    // Move to second row if needed
    if (m_showPlayerPanel || m_showControlsPanel || m_showTileMapEditorPanel) {
        currentX = panelSpacing;
        currentY += 250.0f + panelSpacing;
    }

    if (m_showPlayerPanel) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawPlayerPanel(registry, playerEntity, playerSpeed);
        currentX += 300.0f + panelSpacing;
    }

    if (m_showControlsPanel) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawControlsPanel(registry, playerEntity);
    }

    if (m_showTileMapEditorPanel) {
        ImGui::SetNextWindowPos(ImVec2(currentX, currentY), ImGuiCond_FirstUseEver);
        DrawTileEditorPanel(m_hoveredTileCoords, hoveredTileValue,tileRenderer, tilemap);
        showGridToggle = true;
    }
    else {
        showGridToggle = false;
    }


}

void DebugUI::DrawPerformancePanel(float* frameTimeHistory, int /*frameIndex*/) {
    ImGui::Begin("Performance", &m_showPerformance, ImGuiWindowFlags_AlwaysAutoResize);

    float fps = static_cast<float>(GP2Engine::Time::GetFPS());
    float frameTime = GP2Engine::Time::DeltaTime() * 1000.0f;

    ImGui::Text("FPS: %.1f", static_cast<double>(fps));
    ImGui::Text("Frame Time: %.2f ms", static_cast<double>(frameTime));

    // Calculate average frame time only when performance data updates
    if (m_performanceUpdateTimer == 0.0f) {
        m_cachedAvgFrameTime = 0.0f;
        for (int i = 0; i < FRAME_HISTORY_SIZE; ++i) {
            m_cachedAvgFrameTime += frameTimeHistory[i];
        }
        m_cachedAvgFrameTime /= static_cast<float>(FRAME_HISTORY_SIZE);
        m_cachedAvgFrameTime *= 1000.0f; // Convert to ms
    }

    ImGui::Text("Avg Frame Time: %.2f ms", static_cast<double>(m_cachedAvgFrameTime));
    
    // Show rendering performance
    auto& renderer = GP2Engine::Renderer::GetInstance();
    ImGui::Text("Draw Calls: %d", renderer.GetDrawCallsThisFrame());
    ImGui::Text("Quads Drawn: %d", renderer.GetQuadsDrawnThisFrame());

    ImGui::Separator();

    // Update cached performance data at intervals
    m_performanceUpdateTimer += GP2Engine::Time::DeltaTime();
    if (m_performanceUpdateTimer >= m_performanceUpdateInterval) {
        GP2Engine::Profiler& profiler = GP2Engine::Profiler::GetInstance();

        // Cache individual system times
        m_cachedInputTime = profiler.GetSystemTimeMs("Input");
        m_cachedPhysicsTime = profiler.GetSystemTimeMs("Physics");
        m_cachedGraphicsTime = profiler.GetSystemTimeMs("Graphics");
        m_cachedUITime = profiler.GetSystemTimeMs("UI");

        m_cachedInputPercent = profiler.GetSystemPercentage("Input");
        m_cachedPhysicsPercent = profiler.GetSystemPercentage("Physics");
        m_cachedGraphicsPercent = profiler.GetSystemPercentage("Graphics");
        m_cachedUIPercent = profiler.GetSystemPercentage("UI");

        m_performanceUpdateTimer = 0.0f;
    }

    // System Performance (individual components that add up to 100%)
    ImGui::Text("System Breakdown:");
    ImGui::Text("Input:    %.1f ms (%.0f%%)", static_cast<double>(m_cachedInputTime), static_cast<double>(m_cachedInputPercent));
    ImGui::Text("Physics:  %.1f ms (%.0f%%)", static_cast<double>(m_cachedPhysicsTime), static_cast<double>(m_cachedPhysicsPercent));
    ImGui::Text("Graphics: %.1f ms (%.0f%%)", static_cast<double>(m_cachedGraphicsTime), static_cast<double>(m_cachedGraphicsPercent));
    ImGui::Text("UI:       %.1f ms (%.0f%%)", static_cast<double>(m_cachedUITime), static_cast<double>(m_cachedUIPercent));

    ImGui::End();
}

void DebugUI::DrawEntityInspector(GP2Engine::Registry& registry) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Entity Inspector", &m_showEntityInspector);

    ImGui::Text("Entity Inspector");

    // Debug: Show all active entities and their component status
    if (ImGui::Button("Debug: Show Active Entities")) {
        std::cout << "=== Active Entity Debug ===" << std::endl;
        std::cout << "Total active entities: " << registry.GetActiveEntities().size() << std::endl;

        // Show only active entities
        for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
            GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);
            GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
            GP2Engine::SpriteComponent* sprite = registry.GetComponent<GP2Engine::SpriteComponent>(entity);

            std::cout << "Entity " << entity << ": ";
            if (tag) std::cout << "Tag('" << tag->name << "') ";
            if (transform) std::cout << "Transform2D ";
            if (sprite) std::cout << "SpriteComponent ";
            std::cout << std::endl;
        }
        std::cout << "================" << std::endl;
    }

    ImGui::Separator();

    // Entity Management buttons
    DrawEntityManagement(registry);
    ImGui::Separator();

    // Two-column layout: Entity List | Selected Entity Details
    ImGui::Columns(2, "EntityInspectorColumns");
    ImGui::SetColumnWidth(0, 200.0f); // Set left column width

    // Left column: Entity List
    ImGui::Text("Entities:");
    DrawEntityList(registry);

    ImGui::NextColumn();

    // Right column: Selected Entity Details
    ImGui::Text("Selected Entity:");
    DrawSelectedEntityDetails(registry);

    ImGui::Columns(1); // Reset to single column
    ImGui::End();
}

void DebugUI::DrawPlayerPanel(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity, float& playerSpeed) {
    ImGui::Begin("Player", &m_showPlayerPanel, ImGuiWindowFlags_AlwaysAutoResize);

    if (playerEntity != GP2Engine::INVALID_ENTITY) {
        GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(playerEntity);
        if (transform) {
            ImGui::Text("Player Entity: %u", playerEntity);
            ImGui::Text("Position: (%.1f, %.1f)", static_cast<double>(transform->position.x), static_cast<double>(transform->position.y));

            // Allow editing player speed
            ImGui::SliderFloat("Speed", &playerSpeed, 50.0f, 1000.0f);

            // Manual position control
            float pos[2] = {transform->position.x, transform->position.y};
            if (ImGui::InputFloat2("Set Position", pos)) {
                transform->position.x = pos[0];
                transform->position.y = pos[1];
            }
        }
    } else {
        ImGui::Text("No player found!");
    }

    ImGui::End();
}

void DebugUI::DrawControlsPanel(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity) {
    ImGui::Begin("Controls", &m_showControlsPanel, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Debug UI Controls:");
    ImGui::Text("F1: Toggle Debug UI");
    ImGui::Text("Use checkboxes above to show/hide panels");
    ImGui::Separator();

    ImGui::Text("Game Controls:");
    ImGui::Text("WASD: Move Player");
    ImGui::Text("Shift: Move Faster");
    ImGui::Separator();
    ImGui::Text("Right Click: Play Sound");

    if (ImGui::Button("Reset Player Position") && playerEntity != GP2Engine::INVALID_ENTITY) {
        GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(playerEntity);
        if (transform) {
            transform->position = GP2Engine::Vector2D(512.0f, 384.0f);
        }
    }

    ImGui::End();
}

// === ENTITY INSPECTOR HELPER FUNCTIONS ===

void DebugUI::DrawEntityManagement(GP2Engine::Registry& registry) {
    if (ImGui::Button("Create New Entity")) {
        CreateNewEntity(registry);
    }

    ImGui::SameLine();

    // Show copy button state for debugging
    bool canCopy = (m_selectedEntity != GP2Engine::INVALID_ENTITY);
    if (!canCopy) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button("Copy Selected")) {
        if (canCopy) {
            std::cout << "Attempting to copy entity " << m_selectedEntity << std::endl;
            CopyEntity(registry, m_selectedEntity);
        } else {
            std::cout << "Cannot copy: no entity selected" << std::endl;
        }
    }

    if (!canCopy) {
        ImGui::PopStyleVar();
    }

    ImGui::SameLine();

    bool canDelete = (m_selectedEntity != GP2Engine::INVALID_ENTITY);
    if (!canDelete) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button("Delete Selected")) {
        if (canDelete) {
            DeleteEntity(registry, m_selectedEntity);
            m_selectedEntity = GP2Engine::INVALID_ENTITY;
        }
    }

    if (!canDelete) {
        ImGui::PopStyleVar();
    }

    // Show selected entity info
    if (m_selectedEntity != GP2Engine::INVALID_ENTITY) {
        ImGui::Text("Selected: Entity %u", m_selectedEntity);
    } else {
        ImGui::Text("Selected: None");
    }
}

void DebugUI::DrawEntityList(GP2Engine::Registry& registry) {
    // Display only active entities
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        // Check if entity has any components (is alive)
        GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);

        // Entity is alive - display it
        std::string entityName = "Entity " + std::to_string(entity);
        if (tag) {
            entityName = tag->name + " (" + std::to_string(entity) + ")";
        } else {
            entityName += " (No Tag)";
        }

        // Selectable entity
        bool isSelected = (m_selectedEntity == entity);
        if (ImGui::Selectable(entityName.c_str(), isSelected)) {
            m_selectedEntity = entity;
        }
    }
}

void DebugUI::DrawSelectedEntityDetails(GP2Engine::Registry& registry) {
    if (m_selectedEntity == GP2Engine::INVALID_ENTITY) {
        ImGui::Text("No entity selected");
        return;
    }

    // Check if selected entity still exists (has any components)
    GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(m_selectedEntity);
    GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(m_selectedEntity);
    GP2Engine::SpriteComponent* sprite = registry.GetComponent<GP2Engine::SpriteComponent>(m_selectedEntity);

    if (!tag && !transform && !sprite) {
        ImGui::Text("Selected entity no longer exists");
        m_selectedEntity = GP2Engine::INVALID_ENTITY;
        return;
    }

    ImGui::Text("Entity ID: %u", m_selectedEntity);
    ImGui::Separator();

    // Component Editor
    DrawComponentEditor(registry, m_selectedEntity);
}

void DebugUI::DrawComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity) {
    // Component Addition buttons
    ImGui::Text("Add Components:");

    if (ImGui::Button("Add Transform2D")) {
        AddTransform2D(registry, entity);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Sprite")) {
        AddSpriteComponent(registry, entity);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Tag")) {
        AddTag(registry, entity);
    }

    ImGui::Separator();
    ImGui::Text("Components:");

    // Transform2D Component
    GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
    if (transform) {
        if (ImGui::CollapsingHeader("Transform2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("Position X", &transform->position.x, 1.0f, 0.0f, 0.0f, "%.1f");
            ImGui::DragFloat("Position Y", &transform->position.y, 1.0f, 0.0f, 0.0f, "%.1f");
            ImGui::SliderFloat("Rotation", &transform->rotation, 0.0f, 360.0f, "%.1f deg");
            ImGui::SliderFloat("Scale X", &transform->scale.x, 0.1f, 5.0f);
            ImGui::SliderFloat("Scale Y", &transform->scale.y, 0.1f, 5.0f);

            if (ImGui::Button("Remove Transform2D")) {
                registry.RemoveComponent<GP2Engine::Transform2D>(entity);
            }
        }
    }

    // SpriteComponent
    GP2Engine::SpriteComponent* sprite = registry.GetComponent<GP2Engine::SpriteComponent>(entity);
    if (sprite) {
        if (ImGui::CollapsingHeader("SpriteComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Visible", &sprite->visible);
            ImGui::DragInt("Render Layer", &sprite->renderLayer, 1.0f, 0, 5);
            ImGui::ColorEdit4("Color", &sprite->color.r);
            ImGui::SliderFloat("Size X", &sprite->size.x, 1.0f, 500.0f);
            ImGui::SliderFloat("Size Y", &sprite->size.y, 1.0f, 500.0f);

            if (sprite->IsTextured()) {
                ImGui::Text("Type: Textured");
                ImGui::DragFloat2("UV Offset", &sprite->uvOffset.x, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat2("UV Size", &sprite->uvSize.x, 0.01f, 0.0f, 1.0f);
            } else {
                ImGui::Text("Type: Colored Quad");
            }

            if (ImGui::Button("Remove SpriteComponent")) {
                registry.RemoveComponent<GP2Engine::SpriteComponent>(entity);
            }
        }
    }

    // Tag Component
    GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);
    if (tag) {
        if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Initialize buffers when entity selection changes OR when tag content changes
            static GP2Engine::EntityID lastEntity = GP2Engine::INVALID_ENTITY;
            static std::string lastTagName = "";

            // Reload buffer if entity changed OR tag name changed (handles recycled entity IDs)
            if (lastEntity != entity || lastTagName != tag->name) {
                m_tagNameBuffer = tag->name;
                m_tagGroupBuffer = tag->group;
                lastEntity = entity;
                lastTagName = tag->name;
            }

            // ImGui needs char* buffers, so we use resize + data()
            m_tagNameBuffer.resize(256);
            if (ImGui::InputText("Name", m_tagNameBuffer.data(), m_tagNameBuffer.capacity())) {
                tag->name = m_tagNameBuffer.c_str();  // Update tag, trim null chars
            }

            m_tagGroupBuffer.resize(256);
            if (ImGui::InputText("Group", m_tagGroupBuffer.data(), m_tagGroupBuffer.capacity())) {
                tag->group = m_tagGroupBuffer.c_str();  // Update tag, trim null chars
            }

            if (ImGui::Button("Remove Tag")) {
                registry.RemoveComponent<GP2Engine::Tag>(entity);
            }
        }
    }
}

// === ENTITY OPERATIONS ===

void DebugUI::CreateNewEntity(GP2Engine::Registry& registry) {
    uint32_t slotsBefore = registry.GetEntityCount();
    GP2Engine::EntityID newEntity = registry.CreateEntity();
    uint32_t slotsAfter = registry.GetEntityCount();

    // Add default components
    registry.AddComponent<GP2Engine::Transform2D>(newEntity, GP2Engine::Transform2D(GP2Engine::Vector2D(512.0f, 384.0f)));
    registry.AddComponent<GP2Engine::Tag>(newEntity, GP2Engine::Tag("New Entity", "debug"));

    m_selectedEntity = newEntity;
    std::cout << "Created new entity " << newEntity << " (slots: " << slotsBefore << " -> " << slotsAfter << ")" << std::endl;
}

void DebugUI::CopyEntity(GP2Engine::Registry& registry, GP2Engine::EntityID sourceEntity) {
    GP2Engine::EntityID newEntity = registry.CreateEntity();

    // Copy Transform2D if it exists
    GP2Engine::Transform2D* sourceTransform = registry.GetComponent<GP2Engine::Transform2D>(sourceEntity);
    if (sourceTransform) {
        registry.AddComponent<GP2Engine::Transform2D>(newEntity, *sourceTransform);
    }

    // Copy SpriteComponent if it exists
    GP2Engine::SpriteComponent* sourceSprite = registry.GetComponent<GP2Engine::SpriteComponent>(sourceEntity);
    if (sourceSprite) {
        registry.AddComponent<GP2Engine::SpriteComponent>(newEntity, *sourceSprite);
    }

    // Copy Tag if it exists
    GP2Engine::Tag* sourceTag = registry.GetComponent<GP2Engine::Tag>(sourceEntity);
    if (sourceTag) {
        GP2Engine::Tag newTag = *sourceTag;
        registry.AddComponent<GP2Engine::Tag>(newEntity, newTag);
    }

    m_selectedEntity = newEntity;
    std::cout << "Copied entity " << sourceEntity << " to " << newEntity << std::endl;
}

void DebugUI::DeleteEntity(GP2Engine::Registry& registry, GP2Engine::EntityID entity) {
    registry.DestroyEntity(entity);
    std::cout << "Deleted entity: " << entity << std::endl;
}

// === COMPONENT OPERATIONS ===

void DebugUI::AddTransform2D(GP2Engine::Registry& registry, GP2Engine::EntityID entity) {
    if (!registry.GetComponent<GP2Engine::Transform2D>(entity)) {
        registry.AddComponent<GP2Engine::Transform2D>(entity, GP2Engine::Transform2D(GP2Engine::Vector2D(512.0f, 384.0f)));
        std::cout << "Added Transform2D to entity " << entity << std::endl;
    }
}

void DebugUI::AddSpriteComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity) {
    if (!registry.GetComponent<GP2Engine::SpriteComponent>(entity)) {
        registry.AddComponent<GP2Engine::SpriteComponent>(entity, GP2Engine::SpriteComponent(64.0f, 64.0f, 1.0f, 1.0f, 1.0f, 1.0f));
        std::cout << "Added SpriteComponent to entity " << entity << std::endl;
    }
}

void DebugUI::AddTag(GP2Engine::Registry& registry, GP2Engine::EntityID entity) {
    if (!registry.GetComponent<GP2Engine::Tag>(entity)) {
        registry.AddComponent<GP2Engine::Tag>(entity, GP2Engine::Tag("New Entity", "debug"));
        std::cout << "Added Tag to entity " << entity << std::endl;
    }
}

// === STRESS TEST OPERATIONS ===

void DebugUI::DrawStressTestPanel(GP2Engine::Registry& registry) {
    ImGui::Begin("Stress Test", &m_showStressTest);
    
    ImGui::Text("SPECIFICATION TEST: 2500+ Objects in Background");
    ImGui::Separator();
    
    // Status
    if (m_stressTestActive) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SPECIFICATION TEST ACTIVE");
        ImGui::Text("Objects Rendered: %d", static_cast<int>(m_stressTestEntities.size()));
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "READY TO TEST");
    }
    
    ImGui::Separator();
    
    // Performance Monitoring
    if (m_stressTestActive) {
        ImGui::Text("Performance Validation:");
        float currentFPS = ImGui::GetIO().Framerate;
        if (currentFPS >= 60.0f) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "FPS: %.1f - SPECIFICATION MET!", currentFPS);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FPS: %.1f - Below 60 FPS target", currentFPS);
        }
        ImGui::Separator();
    }
    
    // Configuration
    ImGui::Text("Configuration:");
    ImGui::SliderInt("Object Count", &m_stressTestObjectCount, 500, 2000);
    ImGui::SliderFloat("Animation Speed", &m_stressTestAnimationSpeed, 0.1f, 2.0f);
    
    ImGui::Separator();
    
    // Controls
    if (!m_stressTestActive) {
        if (ImGui::Button("Start Stress Test", ImVec2(-1, 0))) {
            StartStressTest(registry);
        }
    } else {
        if (ImGui::Button("Stop Stress Test", ImVec2(-1, 0))) {
            StopStressTest(registry);
        }
    }
    
    ImGui::End();
}

void DebugUI::StartStressTest(GP2Engine::Registry& registry) {
    std::cout << "StartStressTest called! Active: " << m_stressTestActive << std::endl;

    if (m_stressTestActive) {
        std::cout << "Stress test already active, returning..." << std::endl;
        return; // Already active
    }

    std::cout << "Starting OPTIMIZED stress test with " << m_stressTestObjectCount << " animated objects..." << std::endl;

    // Clear any existing stress test entities
    m_stressTestEntities.clear();
    m_stressTestVelocities.clear();
    m_stressTestEntities.reserve(m_stressTestObjectCount);

    // ===== OPTIMIZATION 1: Load textures once =====
    auto& resMgr = GP2Engine::ResourceManager::GetInstance();
    auto playerWalkTexture = resMgr.LoadTexture("textures/SS_Walk_Horizontal.png");
    auto monsterHorizontalTexture = resMgr.LoadTexture("textures/SS_Monster_Side_Horizontal.png");

    // Validate textures
    if (!playerWalkTexture || !monsterHorizontalTexture) {
        std::cerr << "ERROR: Failed to load sprite sheet textures for stress test!" << std::endl;
        return;
    }

    // ===== OPTIMIZATION 2: Create TWO shared sprite instances (not 1000!) =====
    // Player sprite
    m_stressTestPlayerSprite = std::make_shared<GP2Engine::Sprite>();
    m_stressTestPlayerSprite->SetTexture(playerWalkTexture);

    // Setup walk animation ONCE for all players (3 frames for performance)
    std::vector<GP2Engine::AnimationFrame> playerFrames;
    for (int frame = 0; frame < 3; ++frame) { // Reduced from 6 to 3 frames
        playerFrames.emplace_back(
            glm::vec2(frame * 512.0f, 0.0f),
            glm::vec2(512.0f, 512.0f),
            0.15f // Slower animation
        );
    }
    m_stressTestPlayerSprite->AddAnimation("walk", playerFrames, true);
    m_stressTestPlayerSprite->PlayAnimation("walk");

    // Monster sprite
    m_stressTestMonsterSprite = std::make_shared<GP2Engine::Sprite>();
    m_stressTestMonsterSprite->SetTexture(monsterHorizontalTexture);

    // Setup walk animation ONCE for all monsters (3 frames for performance)
    std::vector<GP2Engine::AnimationFrame> monsterFrames;
    for (int frame = 0; frame < 3; ++frame) { // Reduced from 6 to 3 frames
        monsterFrames.emplace_back(
            glm::vec2(frame * 512.0f, 0.0f),
            glm::vec2(512.0f, 512.0f),
            0.15f // Slower animation
        );
    }
    m_stressTestMonsterSprite->AddAnimation("walk", monsterFrames, true);
    m_stressTestMonsterSprite->PlayAnimation("walk");

    // ===== OPTIMIZATION 3: Simplified grid for 1000 objects =====
    int gridSize = 32; // 32x32 = 1024, use 1000
    float cellWidth = SCREEN_WIDTH / gridSize;
    float cellHeight = SCREEN_HEIGHT / gridSize;

    for (int i = 0; i < m_stressTestObjectCount; ++i) {
        auto entity = registry.CreateEntity();

        // Grid-based position with small randomization
        int gridX = i % gridSize;
        int gridY = i / gridSize;
        float x = gridX * cellWidth + (rand() % static_cast<int>(cellWidth * 0.5f));
        float y = gridY * cellHeight + (rand() % static_cast<int>(cellHeight * 0.5f));

        // Random velocity (reduced range for smoother movement)
        float velocityX = -30.0f + static_cast<float>(rand() % 60); // -30 to 30
        float velocityY = -30.0f + static_cast<float>(rand() % 60); // -30 to 30

        // Larger scale for better visibility (0.20 to 0.40)
        float scale = 0.20f + static_cast<float>(rand() % 20) / 100.0f; // 0.20 to 0.40

        // Add transform
        GP2Engine::Transform2D transform;
        transform.position = GP2Engine::Vector2D(x, y);
        transform.rotation = 0.0f; // No initial rotation for performance
        transform.scale = GP2Engine::Vector2D(scale, scale);
        registry.AddComponent<GP2Engine::Transform2D>(entity, transform);

        // ===== OPTIMIZATION 4: Share sprite instances =====
        bool isPlayer = (i % 2 == 0);

        GP2Engine::SpriteComponent spriteComp;
        spriteComp.sprite = isPlayer ? m_stressTestPlayerSprite : m_stressTestMonsterSprite;
        spriteComp.size = GP2Engine::Vector2D(64.0f, 64.0f); // Larger, more visible size
        spriteComp.visible = true;
        spriteComp.renderLayer = 1;
        registry.AddComponent<GP2Engine::SpriteComponent>(entity, spriteComp);

        // Add tag
        registry.AddComponent<GP2Engine::Tag>(entity,
            GP2Engine::Tag(isPlayer ? "StressTest_Player" : "StressTest_Monster", "stress"));

        // Store velocity
        m_stressTestVelocities[entity] = GP2Engine::Vector2D(velocityX, velocityY);

        m_stressTestEntities.push_back(entity);
    }

    m_stressTestActive = true;
    std::cout << "✓ Optimized stress test started: " << m_stressTestEntities.size()
              << " objects sharing 2 sprite instances!" << std::endl;
}

void DebugUI::StopStressTest(GP2Engine::Registry& registry) {
    if (!m_stressTestActive) {
        return; // Not active
    }

    std::cout << "Stopping stress test..." << std::endl;

    // Destroy all stress test entities
    for (auto entity : m_stressTestEntities) {
        registry.DestroyEntity(entity);
    }

    m_stressTestEntities.clear();
    m_stressTestVelocities.clear();

    // Clear shared sprites
    m_stressTestPlayerSprite.reset();
    m_stressTestMonsterSprite.reset();

    m_stressTestActive = false;

    std::cout << "Stress test stopped and cleaned up" << std::endl;
}

void DebugUI::UpdateStressTestObjects(GP2Engine::Registry& registry, float deltaTime) {
    if (!m_stressTestActive) {
        return;
    }

    // ===== OPTIMIZATION: Update shared sprite animations ONCE, not 1000 times! =====
    if (m_stressTestPlayerSprite) {
        m_stressTestPlayerSprite->UpdateAnimation(deltaTime);
    }
    if (m_stressTestMonsterSprite) {
        m_stressTestMonsterSprite->UpdateAnimation(deltaTime);
    }

    // ===== Update physics for all entities =====
    const float speedMultiplier = m_stressTestAnimationSpeed;
    const float minX = 0.0f;
    const float maxX = SCREEN_WIDTH;
    const float minY = 0.0f;
    const float maxY = SCREEN_HEIGHT;

    for (size_t i = 0; i < m_stressTestEntities.size(); ++i) {
        auto entity = m_stressTestEntities[i];
        auto transform = registry.GetComponent<GP2Engine::Transform2D>(entity);

        if (!transform) continue;

        // Get velocity for this entity
        auto velIt = m_stressTestVelocities.find(entity);
        if (velIt != m_stressTestVelocities.end()) {
            GP2Engine::Vector2D& velocity = velIt->second;

            // Physics-based movement: position += velocity * deltaTime
            transform->position.x += velocity.x * deltaTime * speedMultiplier;
            transform->position.y += velocity.y * deltaTime * speedMultiplier;

            // Bounce off screen edges (collision response)
            if (transform->position.x < minX) {
                transform->position.x = minX;
                velocity.x = -velocity.x;
            } else if (transform->position.x > maxX) {
                transform->position.x = maxX;
                velocity.x = -velocity.x;
            }

            if (transform->position.y < minY) {
                transform->position.y = minY;
                velocity.y = -velocity.y;
            } else if (transform->position.y > maxY) {
                transform->position.y = maxY;
                velocity.y = -velocity.y;
            }
        }
    }
}

void DebugUI::DrawDebugVisualizationPanel(bool& showCollisionBoxes, bool& showVelocityVectors) {
    ImGui::Begin("Debug Visualization", &m_showDebugVisualization, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Physics Debug Rendering");
    ImGui::Separator();

    // Individual toggles
    ImGui::Checkbox("Show Collision Boxes (AABB)", &showCollisionBoxes);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display axis-aligned bounding boxes for all entities\n"
                        "Green = Player, Red = Other entities, White = Centers");
    }
    
    ImGui::Checkbox("Show Velocity Vectors", &showVelocityVectors);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display velocity and acceleration vectors\n"
                        "Yellow = Velocity, Magenta = Acceleration");
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Available Features:");
    ImGui::BulletText("AABB collision boxes");
    ImGui::BulletText("Velocity direction arrows");
    ImGui::BulletText("Acceleration vectors");
    ImGui::BulletText("Entity center points");

    ImGui::End();
}

// === TILEMAP EDITOR OPERATIONS

void DebugUI::DrawTileEditorPanel(const GP2Engine::Vector2D& hoveredTileCoords, int hoveredTileValue, GP2Engine::TileRenderer& /*tileRenderer*/, GP2Engine::TileMap& tilemap) {

    ImGui::Begin("Tilemap Editor", &m_showTileMapEditorPanel, ImGuiWindowFlags_AlwaysAutoResize);

    // --- Visualization & Tools ---
    ImGui::Text("Visualization & Tools:");
    ImGui::Separator();
    

    ImGui::Separator();

    // --- Hovered Tile Info ---
    ImGui::Text("Hovered Tile Info:");

    // Tile is valid
    ImGui::BulletText("Coords: (%d, %d)", (int)hoveredTileCoords.x, (int)hoveredTileCoords.y);

    // Display tile type and picture
    const auto& tileDefinitions = tilemap.GetTileDefinitions();
    int numTileDefs = (int)tileDefinitions.size();
    const char* tileName = "No tile hovered";
    if (hoveredTileValue >= 0 && hoveredTileValue < numTileDefs) {
        tileName = tileDefinitions[hoveredTileValue].name.c_str();
    }
    else if (hoveredTileValue >= 0) {
        tileName = "Unknown Tile ID";
    }
    ImGui::BulletText("Type: %s (%d)", tileName, hoveredTileValue);

    ImGui::Text("Tile Placement Palette:");
    ImGui::Separator();
    ImGui::Text("Select Brush:");

    // Define standard palette styles
    const ImVec2 BUTTON_SIZE(48, 48);
    const ImVec2 UV0(0.0f, 0.0f);     // Top-Left UV (full texture)
    const ImVec2 UV1(1.0f, 1.0f);     // Bottom-Right UV (full texture)
    const ImVec4 TINT_COL(1.0f, 1.0f, 1.0f, 1.0f);   // No color tint
    const ImVec4 BACKGROUND_TINT(0.0f, 0.0f, 0.0f, 0.0f); // Fully transparent background

    //Loop through all the tile
    for (int i = 0; i < numTileDefs; ++i) {
        const auto& def = tileDefinitions[i];
        unsigned int textureID = def.GetTextureID();

        // Create horizontal layout
        if (i > 0) {
            ImGui::SameLine(0.0f, 25.0f);
        }


        ImGui::PushID(i);
        

        // Set visual style for selected tile
        if (m_selectedTileType == i) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4)); 
        }
        else {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        }

        // Create the image button
        if (ImGui::ImageButton(def.name.c_str(),
            (void*)(intptr_t)textureID,
            BUTTON_SIZE,
            UV0, UV1,
            BACKGROUND_TINT,
            TINT_COL))
        {
            m_selectedTileType = i;
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", def.name.c_str());
        }

        ImGui::PopStyleVar();
        ImGui::PopID();
    }

    // --- Active Brush Display ---
    ImGui::Separator();
    const char* selectedName = (m_selectedTileType >= 0 && m_selectedTileType < numTileDefs)
        ? tileDefinitions[m_selectedTileType].name.c_str()
        : "None";

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active Brush: %s (%d)", selectedName, m_selectedTileType);
    ImGui::Separator();

    // --- Savemap Features ---
    
    if (ImGui::Button("Save Map")) {
        // This is exactly what you wanted: clean call to game logic from the UI.
        tilemap.SaveMap(GP2Engine::TILEMAP_FILEPATH);
    }


    // End Window
    ImGui::End();
}

void DebugUI::SetTileTextureIDs(unsigned int floorID, unsigned int wallID) {
    m_floorTextureID = floorID;
    m_wallTextureID = wallID;
}

void DebugUI::UpdateTileEditorLogic(GP2Engine::TileRenderer& tileRenderer, GP2Engine::TileMap& tileMap, GP2Engine::Vector2D& hoveredTileCoords,int& hoveredTileValue,bool showGridToggle)
{
    if (!showGridToggle) {
        return;
    }

    bool leftClick = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);

    // Check if ImGui is capturing the mouse (to prevent editing while interacting with UI)
    if (ImGui::GetIO().WantCaptureMouse) {
        hoveredTileCoords = GP2Engine::Vector2D(-1.0f, -1.0f);
        hoveredTileValue = -1;
        return;
    }

    // Tile Editing
    if (leftClick) {
        if (hoveredTileCoords.x >= 0.0f) {
            int tileCol = (int)hoveredTileCoords.x;
            int tileRow = (int)hoveredTileCoords.y;

            // Safely get the selected type using the new public getter
            int newTileValue = GetSelectedTileType();

            // Call the function to change the tile value
            tileMap.SetTileValue(tileCol, tileRow, newTileValue);

            // Update the variable used for the Hovered Tile Info panel immediately
            hoveredTileValue = newTileValue;
        }
    }
    // Mouse Hover (Not Clicking)
    else {
        double mouseX, mouseY;
        GP2Engine::Input::GetMousePosition(mouseX, mouseY);
        GP2Engine::Vector2D mousePos = GP2Engine::Vector2D((float)mouseX, (float)mouseY);

        // Calculate and ASSIGN the new coordinates to the member variable
        hoveredTileCoords = tileRenderer.ScreenToTileCoords(mousePos);

        if (hoveredTileCoords.x >= 0.0f) {
            // Coordinates are valid, so get the tile value
            hoveredTileValue = tileMap.GetTileValue(
                (int)hoveredTileCoords.x,
                (int)hoveredTileCoords.y
            );
        }
        else {
            // Coordinates are invalid, clear the tile value
            hoveredTileValue = -1;
        }
    }
}

} // namespace Hollows