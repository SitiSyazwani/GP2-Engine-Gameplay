/**
 * @file ContentEditorUI.cpp
 * @author Adi (100%)
 * @brief Implementation of Content Editor UI panels and logic
 *
 * Manages the main editor UI panels:
 * - Hierarchy Panel: Lists all entities in the scene with selection support
 * - Properties Panel: Component editor for selected entity
 * - Asset Browser Panel: Browse and apply textures/fonts
 *
 * All property changes automatically mark the scene as unsaved.
 */

#include "ContentEditorUI.hpp"
#include <filesystem>


void ContentEditorUI::Initialize() {
    // Initialize texture picker widget with asset path
    m_texturePicker.Initialize("../../Sandbox/assets/textures");

    // Initialize font picker widget with default font size
    m_fontPicker.Initialize("../../Sandbox/assets/fonts", 48);

    LOG_INFO("ContentEditorUI initialized");
}

void ContentEditorUI::Update(float /*deltaTime*/, GP2Engine::Registry& /*registry*/, const std::string& /*currentScenePath*/, bool& /*hasUnsavedChanges*/) {
    // Currently no per frame updates needed
    // Update logic happens during Render() to respond to ImGui interactions
}

void ContentEditorUI::Render(GP2Engine::Registry& registry, const std::string& /*currentScenePath*/, bool& hasUnsavedChanges, bool isPlaying) {
    // Render hierarchy panel (entity list with create/copy/delete)
    if (m_showHierarchy) {
        DrawHierarchy(registry, hasUnsavedChanges, isPlaying);
    }

    // Render properties panel (component editor for selected entity)
    if (m_showProperties) {
        DrawProperties(registry, hasUnsavedChanges, isPlaying);
    }

    // Render asset browser panel
    if (m_showAssetBrowser) {
        DrawAssetBrowser(registry);
    }
}


// ==================== HIERARCHY PANEL ====================

void ContentEditorUI::DrawHierarchy(GP2Engine::Registry& registry, bool& hasUnsavedChanges, bool isPlaying) {
    ImGui::Begin("Hierarchy", &m_showHierarchy);

    ImGui::Text("Scene Hierarchy");
    ImGui::Separator();

    // Disable entity creation/deletion during PLAY mode
    ImGui::BeginDisabled(isPlaying);

    // Entity management buttons
    if (ImGui::Button("Create Entity")) {
        // Create entity with default components
        GP2Engine::EntityID newEntity = registry.CreateEntity();

        // Add default Tag component
        GP2Engine::Tag tag;
        tag.name = "New Entity";
        tag.group = "default";
        registry.AddComponent<GP2Engine::Tag>(newEntity, tag);

        // Add default Transform2D
        GP2Engine::Transform2D transform;
        transform.position = GP2Engine::Vector2D(DEFAULT_ENTITY_POS_X, DEFAULT_ENTITY_POS_Y);
        transform.rotation = 0.0f;
        transform.scale = GP2Engine::Vector2D(1.0f, 1.0f);
        registry.AddComponent<GP2Engine::Transform2D>(newEntity, transform);

        // Select the newly created entity
        m_selectedEntity = newEntity;
        hasUnsavedChanges = true;

        LOG_INFO("Created new entity: " + std::to_string(newEntity));
    }
    ImGui::SameLine();

    if (ImGui::Button("Copy Selected") && m_selectedEntity != GP2Engine::INVALID_ENTITY) {
        // Clone entity using engine's CloneEntity
        GP2Engine::EntityID newEntity = registry.CloneEntity(m_selectedEntity);
        if (newEntity != GP2Engine::INVALID_ENTITY) {
            // Append " (Copy)" to tag name to distinguish duplicate
            if (GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(newEntity)) {
                tag->name += " (Copy)";
            }
            m_selectedEntity = newEntity;
            hasUnsavedChanges = true;
            LOG_INFO("Cloned entity " + std::to_string(m_selectedEntity) + " to " + std::to_string(newEntity));
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Delete Selected") && m_selectedEntity != GP2Engine::INVALID_ENTITY) {
        // Delete entity and deselect
        if (registry.IsEntityAlive(m_selectedEntity)) {
            registry.DestroyEntity(m_selectedEntity);
            m_selectedEntity = GP2Engine::INVALID_ENTITY;
            hasUnsavedChanges = true;
            LOG_INFO("Deleted entity");
        }
    }

    ImGui::EndDisabled();

    ImGui::Separator();

    // Entity list with selection
    ImGui::Text("Entities (%u):", registry.GetEntityCount());
    DrawEntityList(registry);

    ImGui::End();
}

void ContentEditorUI::DrawEntityList(GP2Engine::Registry& registry) {
    ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

    // Iterate through all active entities and display them as selectable items
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);

        // Generate entity label: "TagName (ID)" or "Entity ID"
        std::string label;
        if (tag && !tag->name.empty()) {
            label = tag->name + " (" + std::to_string(entity) + ")";
        } else {
            label = "Entity " + std::to_string(entity);
        }

        // Render selectable entity item
        bool isSelected = (m_selectedEntity == entity);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            m_selectedEntity = entity;
        }
    }

    ImGui::EndChild();
}

// ==================== PROPERTIES PANEL ====================

void ContentEditorUI::DrawProperties(GP2Engine::Registry& registry, bool& hasUnsavedChanges, bool isPlaying) {
    ImGui::Begin("Properties", &m_showProperties);

    // Check if an entity is selected
    if (m_selectedEntity == GP2Engine::INVALID_ENTITY) {
        ImGui::TextDisabled("No entity selected");
        ImGui::Text("Select an entity from the Hierarchy to edit its properties");
    }
    // Validate selected entity still exists
    else if (!registry.IsEntityAlive(m_selectedEntity)) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Selected entity no longer exists!");
        m_selectedEntity = GP2Engine::INVALID_ENTITY;
    }
    // Render component editor for valid selected entity
    else {
        ImGui::Text("Entity ID: %u", m_selectedEntity);
        ImGui::Separator();

        // Disable property editing during PLAY mode
        ImGui::BeginDisabled(isPlaying);
        DrawComponentEditor(registry, m_selectedEntity, hasUnsavedChanges);
        ImGui::EndDisabled();
    }

    ImGui::End();
}

void ContentEditorUI::DrawComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Component addition buttons - allow adding new components to entity
    ImGui::Text("Add Components:");

    if (ImGui::Button("Add Transform2D")) {
        AddTransform2D(registry, entity, hasUnsavedChanges);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Sprite")) {
        AddSpriteComponent(registry, entity, hasUnsavedChanges);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Tag")) {
        AddTag(registry, entity, hasUnsavedChanges);
    }

    if (ImGui::Button("Add Text")) {
        AddTextComponent(registry, entity, hasUnsavedChanges);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add TileMap")) {
        AddTileMapComponent(registry, entity, hasUnsavedChanges);
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Audio")) {
        AddAudioComponent(registry, entity, hasUnsavedChanges);
    }

    ImGui::Separator();
    ImGui::Text("Components:");

    // Render component editors for all components on this entity
    DrawTransform2DEditor(registry, entity, hasUnsavedChanges);
    DrawSpriteComponentEditor(registry, entity, hasUnsavedChanges);
    DrawTagEditor(registry, entity, hasUnsavedChanges);
    DrawTextComponentEditor(registry, entity, hasUnsavedChanges);
    DrawTileMapComponentEditor(registry, entity, hasUnsavedChanges);
    DrawAudioComponentEditor(registry, entity, hasUnsavedChanges);
}

// ==================== COMPONENT EDITORS ====================

void ContentEditorUI::DrawTransform2DEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
    if (transform) {
        if (ImGui::CollapsingHeader("Transform2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Position editing
            if (ImGui::DragFloat("Position X", &transform->position.x, 1.0f, 0.0f, 0.0f, "%.1f")) {
                hasUnsavedChanges = true;
            }
            if (ImGui::DragFloat("Position Y", &transform->position.y, 1.0f, 0.0f, 0.0f, "%.1f")) {
                hasUnsavedChanges = true;
            }

            // Rotation editing (degrees)
            if (ImGui::SliderFloat("Rotation", &transform->rotation, 0.0f, 360.0f, "%.1f deg")) {
                hasUnsavedChanges = true;
            }

            // Scale editing
            if (ImGui::SliderFloat("Scale X", &transform->scale.x, 0.1f, 5.0f)) {
                hasUnsavedChanges = true;
            }
            if (ImGui::SliderFloat("Scale Y", &transform->scale.y, 0.1f, 5.0f)) {
                hasUnsavedChanges = true;
            }

            // Remove component button
            if (ImGui::Button("Remove Transform2D")) {
                registry.RemoveComponent<GP2Engine::Transform2D>(entity);
                hasUnsavedChanges = true;
            }
        }
    }
}

void ContentEditorUI::DrawSpriteComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    GP2Engine::SpriteComponent* sprite = registry.GetComponent<GP2Engine::SpriteComponent>(entity);
    if (sprite) {
        if (ImGui::CollapsingHeader("SpriteComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Visibility toggle
            if (ImGui::Checkbox("Visible", &sprite->visible)) {
                hasUnsavedChanges = true;
            }

            // Render layer controls draw order (higher = drawn on top)
            if (ImGui::SliderInt("Render Layer", &sprite->renderLayer, -10, 10)) {
                hasUnsavedChanges = true;
            }

            // Color tint (multiplied with texture if present)
            if (ImGui::ColorEdit4("Color", &sprite->color.r)) {
                hasUnsavedChanges = true;
            }

            // Size controls
            if (ImGui::SliderFloat("Size X", &sprite->size.x, MIN_SPRITE_SIZE, MAX_SPRITE_SIZE)) {
                hasUnsavedChanges = true;
            }
            if (ImGui::SliderFloat("Size Y", &sprite->size.y, MIN_SPRITE_SIZE, MAX_SPRITE_SIZE)) {
                hasUnsavedChanges = true;
            }

            // Texture assignment section
            ImGui::Separator();
            ImGui::Text("Texture:");

            // Drag-drop target for textures from AssetBrowser or EditorViewport
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                    const char* path = (const char*)payload->Data;
                    std::string texturePath(path);

                    // Load texture through ResourceManager
                    auto& resMgr = GP2Engine::ResourceManager::GetInstance();
                    auto texture = resMgr.LoadTexture(texturePath);

                    // Apply texture to sprite
                    if (texture && texture->IsValid()) {
                        sprite->sprite = std::make_shared<GP2Engine::Sprite>(texture);
                        sprite->sprite->SetSize(sprite->size);
                        sprite->sprite->SetColor(sprite->color);
                        hasUnsavedChanges = true;
                        LOG_INFO("Applied texture via drag-drop: " + texturePath);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Display different UI based on whether sprite has texture
            if (sprite->IsTextured()) {
                ImGui::Text("Type: Textured Sprite");

                // Show current texture path
                std::string texturePath = sprite->sprite->GetTexture()->GetFilePath();
                ImGui::TextWrapped("Current: %s", texturePath.c_str());

                // Button to remove texture (convert to colored quad)
                if (ImGui::Button("Clear Texture (Convert to Quad)")) {
                    sprite->sprite = nullptr;
                    hasUnsavedChanges = true;
                }

                // UV coordinate controls for texture spritesheets
                ImGui::Separator();
                ImGui::Text("UV Coordinates:");
                if (ImGui::SliderFloat2("UV Offset", &sprite->uvOffset.x, 0.0f, 1.0f)) {
                    hasUnsavedChanges = true;
                }
                if (ImGui::SliderFloat2("UV Size", &sprite->uvSize.x, 0.0f, 1.0f)) {
                    hasUnsavedChanges = true;
                }
            } else {
                // Sprite is a colored quad - show texture picker
                ImGui::Text("Type: Colored Quad");

                // Texture picker widget to assign texture
                std::string selectedTexture;
                if (m_texturePicker.Render(selectedTexture)) {
                    auto& resMgr = GP2Engine::ResourceManager::GetInstance();
                    auto texture = resMgr.LoadTexture(selectedTexture);

                    if (texture && texture->IsValid()) {
                        sprite->sprite = std::make_shared<GP2Engine::Sprite>(texture);
                        sprite->sprite->SetSize(sprite->size);
                        sprite->sprite->SetColor(sprite->color);
                        hasUnsavedChanges = true;
                        LOG_INFO("Applied texture: " + selectedTexture);
                    } else {
                        LOG_ERROR("Failed to load texture: " + selectedTexture);
                    }
                }
            }

            // Remove component button
            if (ImGui::Button("Remove SpriteComponent")) {
                registry.RemoveComponent<GP2Engine::SpriteComponent>(entity);
                hasUnsavedChanges = true;
            }
        }
    }
}

void ContentEditorUI::DrawTagEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    GP2Engine::Tag* tag = registry.GetComponent<GP2Engine::Tag>(entity);
    if (tag) {
        if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Initialize buffers when entity selection changes OR when tag content changes
            // This handles entity recycling (when an entity ID is reused after deletion)
            if (m_lastTagEntity != entity || m_lastTagName != tag->name) {
                m_tagNameBuffer = tag->name;
                m_tagGroupBuffer = tag->group;
                m_lastTagEntity = entity;
                m_lastTagName = tag->name;
            }

            // Tag name input (used for entity identification in Hierarchy)
            m_tagNameBuffer.resize(256);
            if (ImGui::InputText("Name", m_tagNameBuffer.data(), m_tagNameBuffer.capacity())) {
                tag->name = m_tagNameBuffer.c_str();  // Update tag, trim null chars
                hasUnsavedChanges = true;
            }

            // Tag group input (used for entity grouping/filtering)
            m_tagGroupBuffer.resize(256);
            if (ImGui::InputText("Group", m_tagGroupBuffer.data(), m_tagGroupBuffer.capacity())) {
                tag->group = m_tagGroupBuffer.c_str();  // Update group
                hasUnsavedChanges = true;
            }

            // Remove component button
            if (ImGui::Button("Remove Tag")) {
                registry.RemoveComponent<GP2Engine::Tag>(entity);
                hasUnsavedChanges = true;
            }
        }
    }
}

void ContentEditorUI::DrawTextComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    GP2Engine::TextComponent* text = registry.GetComponent<GP2Engine::TextComponent>(entity);
    if (text) {
        if (ImGui::CollapsingHeader("TextComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Text content editing
            strncpy_s(m_textBuffer, text->text.c_str(), sizeof(m_textBuffer) - 1);
            if (ImGui::InputTextMultiline("Text", m_textBuffer, sizeof(m_textBuffer))) {
                text->text = m_textBuffer;
                hasUnsavedChanges = true;
            }

            // Color tint
            if (ImGui::ColorEdit4("Color", &text->color.r)) {
                hasUnsavedChanges = true;
            }

            // Scale multiplier
            if (ImGui::SliderFloat("Scale", &text->scale, 0.1f, 3.0f)) {
                hasUnsavedChanges = true;
            }

            // Visibility toggle
            if (ImGui::Checkbox("Visible", &text->visible)) {
                hasUnsavedChanges = true;
            }

            // Render layer controls draw order
            if (ImGui::SliderInt("Render Layer", &text->renderLayer, -10, 10)) {
                hasUnsavedChanges = true;
            }

            // Font selection section
            ImGui::Separator();
            ImGui::Text("Font:");

            // Show current font info
            if (text->font && text->font->IsValid()) {
                std::string fontPath = text->font->GetFontPath();
                unsigned int fontSize = text->font->GetFontSize();
                ImGui::TextWrapped("Current: %s (Size: %u)", fontPath.c_str(), fontSize);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No font loaded!");
            }

            // Font picker widget to change font
            std::string selectedFont;
            int selectedFontSize;
            if (m_fontPicker.Render(selectedFont, selectedFontSize)) {
                auto& resMgr = GP2Engine::ResourceManager::GetInstance();
                auto newFont = resMgr.LoadFont(selectedFont, static_cast<unsigned int>(selectedFontSize));

                if (newFont && newFont->IsValid()) {
                    text->font = newFont;
                    hasUnsavedChanges = true;
                    LOG_INFO("Applied font: " + selectedFont + " (Size: " + std::to_string(selectedFontSize) + ")");
                } else {
                    LOG_ERROR("Failed to load font: " + selectedFont);
                }
            }

            // Offset from entity position
            ImGui::Separator();
            ImGui::Text("Offset");
            if (ImGui::SliderFloat("##OffsetX", &text->offset.x, -200.0f, 200.0f, "X: %.1f")) {
                hasUnsavedChanges = true;
            }
            if (ImGui::SliderFloat("##OffsetY", &text->offset.y, -200.0f, 200.0f, "Y: %.1f")) {
                hasUnsavedChanges = true;
            }

            // Remove component button
            if (ImGui::Button("Remove TextComponent")) {
                registry.RemoveComponent<GP2Engine::TextComponent>(entity);
                hasUnsavedChanges = true;
            }
        }
    }
}

void ContentEditorUI::DrawTileMapComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    GP2Engine::TileMapComponent* tileMapComp = registry.GetComponent<GP2Engine::TileMapComponent>(entity);
    if (tileMapComp) {
        if (ImGui::CollapsingHeader("TileMapComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Visibility toggle
            if (ImGui::Checkbox("Visible", &tileMapComp->visible)) {
                hasUnsavedChanges = true;
            }

            // Render layer 
            if (ImGui::SliderInt("Render Layer", &tileMapComp->renderLayer, -10, 10)) {
                hasUnsavedChanges = true;
            }

            // Display tile map info (read-only)
            ImGui::Separator();
            ImGui::Text("TileMap Info:");
            if (tileMapComp->tileMap) {
                ImGui::Text("Grid Size: %dx%d", tileMapComp->tileMap->GetGridCols(), tileMapComp->tileMap->GetGridRows());
                ImGui::Text("Map Size: %d tiles", tileMapComp->tileMap->GetMapSize());
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "TileMap pointer is NULL!");
            }

            // Remove component button
            if (ImGui::Button("Remove TileMapComponent")) {
                registry.RemoveComponent<GP2Engine::TileMapComponent>(entity);
                hasUnsavedChanges = true;
            }

            // Show Level Editor panels when tilemap entity is selected
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Tile Editor Active");
            ImGui::Text("Use the Level Editor panels below to edit tiles:");
        }

        // Render LevelEditor ImGui panels (Property Editor, Hierarchy, Level Manager, Camera)
        if (m_levelEditor) {
            m_levelEditor->RenderImGui();
        }
    }
}

// ==================== ASSET BROWSER PANEL ====================

void ContentEditorUI::DrawAssetBrowser(GP2Engine::Registry& /*registry*/) {
    ImGui::Begin("Asset Browser", &m_showAssetBrowser);

    ImGui::Text("Asset Browser");
    ImGui::TextDisabled("(Coming soon)");
    ImGui::Separator();

    ImGui::Text("Future features:");
    ImGui::BulletText("Browse textures, fonts, audio");
    ImGui::BulletText("Drag & drop to assign to entities");
    ImGui::BulletText("Texture preview thumbnails");

    ImGui::End();
}

// ==================== COMPONENT OPERATIONS ====================

void ContentEditorUI::AddTransform2D(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Check if entity already has this component
    if (registry.HasComponent<GP2Engine::Transform2D>(entity)) {
        LOG_INFO("Entity already has Transform2D");
        return;
    }

    // Create default Transform2D at scene center
    GP2Engine::Transform2D transform;
    transform.position = GP2Engine::Vector2D(DEFAULT_ENTITY_POS_X, DEFAULT_ENTITY_POS_Y);
    transform.rotation = 0.0f;
    transform.scale = GP2Engine::Vector2D(1.0f, 1.0f);

    registry.AddComponent<GP2Engine::Transform2D>(entity, transform);
    hasUnsavedChanges = true;
    LOG_INFO("Added Transform2D to entity " + std::to_string(entity));
}

void ContentEditorUI::AddSpriteComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Check if entity already has this component
    if (registry.HasComponent<GP2Engine::SpriteComponent>(entity)) {
        LOG_INFO("Entity already has SpriteComponent");
        return;
    }

    // Create default SpriteComponent (white colored quad)
    GP2Engine::SpriteComponent spriteComp;
    spriteComp.visible = true;
    spriteComp.renderLayer = 0;
    spriteComp.size = GP2Engine::Vector2D(DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE);
    spriteComp.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White

    registry.AddComponent<GP2Engine::SpriteComponent>(entity, spriteComp);
    hasUnsavedChanges = true;
    LOG_INFO("Added SpriteComponent to entity " + std::to_string(entity));
}

void ContentEditorUI::AddTag(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Check if entity already has this component
    if (registry.HasComponent<GP2Engine::Tag>(entity)) {
        LOG_INFO("Entity already has Tag");
        return;
    }

    // Create default Tag with entity ID as name
    GP2Engine::Tag tag;
    tag.name = "Entity " + std::to_string(entity);
    tag.group = "default";

    registry.AddComponent<GP2Engine::Tag>(entity, tag);
    hasUnsavedChanges = true;
    LOG_INFO("Added Tag to entity " + std::to_string(entity));
}

void ContentEditorUI::AddTextComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Check if entity already has this component
    if (registry.HasComponent<GP2Engine::TextComponent>(entity)) {
        LOG_INFO("Entity already has TextComponent");
        return;
    }

    // Load default font (REQUIRED for text rendering)
    auto& resourceMgr = GP2Engine::ResourceManager::GetInstance();
    auto defaultFont = resourceMgr.LoadFont("../../Sandbox/assets/fonts/test1.ttf", 48);

    if (!defaultFont) {
        LOG_ERROR("Failed to load default font for TextComponent!");
        return;
    }

    // Create default TextComponent with sample text
    GP2Engine::TextComponent textComp;
    textComp.font = defaultFont;  // CRITICAL: Set font or text won't render
    textComp.text = "Sample Text";
    textComp.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White
    textComp.scale = 1.0f;
    textComp.visible = true;
    textComp.renderLayer = 1;  // Above sprites by default
    textComp.offset = GP2Engine::Vector2D(0.0f, 0.0f);

    registry.AddComponent<GP2Engine::TextComponent>(entity, textComp);
    hasUnsavedChanges = true;
    LOG_INFO("Added TextComponent to entity " + std::to_string(entity));
}

void ContentEditorUI::AddTileMapComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    // Check if entity already has this component
    if (registry.HasComponent<GP2Engine::TileMapComponent>(entity)) {
        LOG_INFO("Entity already has TileMapComponent");
        return;
    }

    // Verify TileMap system is initialized
    if (!m_levelEditor || !m_tileMap || !m_tileRenderer) {
        LOG_ERROR("Cannot add TileMapComponent: TileMap system not initialized!");
        return;
    }

    // Create TileMapComponent with references to the main TileMap/TileRenderer
    // Only one entity should have a TileMapComponent per scene
    GP2Engine::TileMapComponent tileMapComp;
    tileMapComp.tileMap = m_tileMap;
    tileMapComp.tileRenderer = m_tileRenderer;
    tileMapComp.visible = true;
    tileMapComp.renderLayer = -1;  // Background layer (drawn first)

    registry.AddComponent<GP2Engine::TileMapComponent>(entity, tileMapComp);
    hasUnsavedChanges = true;
    LOG_INFO("Added TileMapComponent to entity " + std::to_string(entity));
    LOG_INFO("Note: Use the Tile Editor panels below to edit the tilemap");
}


void ContentEditorUI::AddAudioComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    if (registry.HasComponent<AudioComponent>(entity)) {
        LOG_INFO("Entity already has AudioComponent");
        return;
    }

    AudioComponent audioComp;
    registry.AddComponent<AudioComponent>(entity, audioComp);
    hasUnsavedChanges = true;
    LOG_INFO("Added AudioComponent to entity " + std::to_string(entity));
}

void ContentEditorUI::DrawAudioComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges) {
    AudioComponent* audioComp = registry.GetComponent<AudioComponent>(entity);

    if (!audioComp) return;

    if (ImGui::CollapsingHeader("AudioComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
        AudioComponentData& data = audioComp->GetData();

        // Audio File Drop Zone
        ImGui::Text("Audio File:");

        std::string displayName = data.soundPath.empty() ? "[Drag audio here]" : data.soundPath;
        if (!data.soundPath.empty()) {
            size_t lastSlash = data.soundPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                displayName = data.soundPath.substr(lastSlash + 1);
            }
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::Button(displayName.c_str(), ImVec2(250, 40));
        ImGui::PopStyleColor();

        // Drag-Drop Target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                const char* path = static_cast<const char*>(payload->Data);
                std::string droppedFile(path);

                // Validate audio format
                std::string ext = droppedFile.substr(droppedFile.find_last_of("."));
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
                    data.soundPath = droppedFile;
                    audioComp->Initialize(data);
                    hasUnsavedChanges = true;
                    LOG_INFO("Audio changed to: " + droppedFile);
                }
                else {
                    LOG_ERROR("Not an audio file: " + droppedFile);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Separator();

        // Playback Controls
        if (ImGui::Button("Play")) {
            audioComp->Play();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            audioComp->Stop();
        }
        ImGui::SameLine();
        if (audioComp->IsPlaying()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Playing");
        }
        else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Stopped");
        }

        ImGui::Separator();

        // Properties
        if (ImGui::SliderFloat("Volume (dB)", &data.volumeDB, -60.0f, 6.0f)) {
            audioComp->SetVolume(data.volumeDB);
            hasUnsavedChanges = true;
        }

        if (ImGui::Checkbox("3D Sound", &data.is3D)) {
            hasUnsavedChanges = true;
        }

        if (ImGui::Checkbox("Loop", &data.isLooping)) {
            hasUnsavedChanges = true;
        }

        if (ImGui::Checkbox("Play on Awake", &data.playOnAwake)) {
            hasUnsavedChanges = true;
        }

        // 3D Settings
        if (data.is3D) {
            ImGui::Separator();
            ImGui::Text("3D Settings:");

            if (ImGui::SliderFloat("Min Distance", &data.minDistance, 0.1f, 100.0f)) {
                hasUnsavedChanges = true;
            }

            if (ImGui::SliderFloat("Max Distance", &data.maxDistance, 1.0f, 1000.0f)) {
                hasUnsavedChanges = true;
            }

            float pos[2] = { data.positionX, data.positionY };
            if (ImGui::DragFloat2("Position", pos, 1.0f)) {
                data.positionX = pos[0];
                data.positionY = pos[1];
                audioComp->SetPosition(pos[0], pos[1]);
                hasUnsavedChanges = true;
            }
        }

        ImGui::Separator();

        // Remove Button
        if (ImGui::Button("Remove AudioComponent")) {
            registry.RemoveComponent<AudioComponent>(entity);
            hasUnsavedChanges = true;
            LOG_INFO("Removed AudioComponent");
        }
    }
}