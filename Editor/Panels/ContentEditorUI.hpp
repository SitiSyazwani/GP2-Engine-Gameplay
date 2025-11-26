/**
 * @file ContentEditorUI.hpp
 * @author Adi (100%)
 * @brief Main UI controller for the Content Editor
 *
 * Manages all editor panels including entity hierarchy, property editor,
 * and asset browser. Provides component editing, entity management, and
 * visual property manipulation.
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>
#include "TexturePicker.hpp"
#include "FontPicker.hpp"
#include "AssetBrowser.hpp"
#include "../Engine/Audio/AudioComponent.hpp"

class EditorViewport;

class ContentEditorUI {
public:
    ContentEditorUI() = default;
    ~ContentEditorUI() = default;

    /**
     * @brief Initialize the editor UI system
     */
    void Initialize();

    /**
     * @brief Set the level editor pointer for tilemap editing
     * @param levelEditor Pointer to the LevelEditor instance
     * @param tileMap Pointer to the TileMap instance
     * @param tileRenderer Pointer to the TileRenderer instance
     */
    void SetLevelEditor(GP2Engine::LevelEditor* levelEditor, GP2Engine::TileMap* tileMap, GP2Engine::TileRenderer* tileRenderer) {
        m_levelEditor = levelEditor;
        m_tileMap = tileMap;
        m_tileRenderer = tileRenderer;
    }

    /**
     * @brief Update editor UI state
     * @param deltaTime Time since last frame
     * @param registry ECS registry reference
     * @param currentScenePath Path to currently loaded scene
     * @param hasUnsavedChanges Whether scene has unsaved changes
     */
    void Update(float deltaTime, GP2Engine::Registry& registry, const std::string& currentScenePath, bool& hasUnsavedChanges);

    /**
     * @brief Render all editor UI panels
     * @param registry ECS registry reference
     * @param currentScenePath Path to currently loaded scene
     * @param hasUnsavedChanges Whether scene has unsaved changes
     * @param isPlaying Whether editor is in PLAY mode (disables editing)
     */
    void Render(GP2Engine::Registry& registry, const std::string& currentScenePath, bool& hasUnsavedChanges, bool isPlaying);

    // === PANEL VISIBILITY GETTERS/SETTERS ===
    bool GetShowHierarchy() const { return m_showHierarchy; }
    void SetShowHierarchy(bool show) { m_showHierarchy = show; }

    bool GetShowProperties() const { return m_showProperties; }
    void SetShowProperties(bool show) { m_showProperties = show; }

    bool GetShowAssetBrowser() const { return m_showAssetBrowser; }
    void SetShowAssetBrowser(bool show) { m_showAssetBrowser = show; }

    // === ENTITY SELECTION API ===
    GP2Engine::EntityID GetSelectedEntity() const { return m_selectedEntity; }
    void SetSelectedEntity(GP2Engine::EntityID entity) { m_selectedEntity = entity; }
    GP2Engine::EntityID* GetSelectedEntityPtr() { return &m_selectedEntity; }

private:

    void DrawAudioComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);
    void AddAudioComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    // === EDITOR CONSTANTS ===
    static constexpr float DEFAULT_ENTITY_POS_X = 512.0f;  
    static constexpr float DEFAULT_ENTITY_POS_Y = 384.0f;  
    static constexpr float DEFAULT_SPRITE_SIZE = 64.0f;
    static constexpr float MIN_SPRITE_SIZE = 1.0f;
    static constexpr float MAX_SPRITE_SIZE = 512.0f;

    // === PANEL VISIBILITY ===
    bool m_showHierarchy = true;
    bool m_showProperties = true;
    bool m_showAssetBrowser = false;  

    // === ENTITY SELECTION ===
    GP2Engine::EntityID m_selectedEntity = GP2Engine::INVALID_ENTITY;

    // === INPUT BUFFERS ===
    std::string m_tagNameBuffer;
    std::string m_tagGroupBuffer;
    char m_textBuffer[512] = {0};  // For TextComponent editing

    // === TAG EDITOR STATE ===
    GP2Engine::EntityID m_lastTagEntity = GP2Engine::INVALID_ENTITY;
    std::string m_lastTagName = "";

    // === PICKER WIDGETS ===
    TexturePicker m_texturePicker;
    FontPicker m_fontPicker;

    // === LEVEL EDITOR ===
    GP2Engine::LevelEditor* m_levelEditor = nullptr;
    GP2Engine::TileMap* m_tileMap = nullptr;
    GP2Engine::TileRenderer* m_tileRenderer = nullptr;


    // === PANEL RENDERING ===
    /**
     * @brief Render entity hierarchy panel (lists all entities)
     * @param registry ECS registry reference
     * @param hasUnsavedChanges Reference to unsaved changes flag
     * @param isPlaying Whether editor is in PLAY mode (disables entity creation/deletion)
     */
    void DrawHierarchy(GP2Engine::Registry& registry, bool& hasUnsavedChanges, bool isPlaying);

    /**
     * @brief Render entity list in hierarchy panel
     */
    void DrawEntityList(GP2Engine::Registry& registry);

    /**
     * @brief Render properties panel (edit selected entity components)
     * @param registry ECS registry reference
     * @param hasUnsavedChanges Reference to unsaved changes flag
     * @param isPlaying Whether editor is in PLAY mode (disables property editing)
     */
    void DrawProperties(GP2Engine::Registry& registry, bool& hasUnsavedChanges, bool isPlaying);

    /**
     * @brief Render component editor for selected entity
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render Transform2D component editor
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawTransform2DEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render SpriteComponent editor
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawSpriteComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render Tag component editor
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawTagEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render TextComponent editor
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawTextComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render TileMapComponent editor
     * @param registry ECS registry reference
     * @param entity Entity to edit
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void DrawTileMapComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Render asset browser panel (textures, fonts, audio)
     */
    void DrawAssetBrowser(GP2Engine::Registry& registry);

    // === COMPONENT OPERATIONS ===
    /**
     * @brief Add Transform2D component to entity
     * @param registry ECS registry reference
     * @param entity Entity to add component to
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void AddTransform2D(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Add SpriteComponent to entity
     * @param registry ECS registry reference
     * @param entity Entity to add component to
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void AddSpriteComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Add Tag component to entity
     * @param registry ECS registry reference
     * @param entity Entity to add component to
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void AddTag(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Add TextComponent to entity
     * @param registry ECS registry reference
     * @param entity Entity to add component to
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void AddTextComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);

    /**
     * @brief Add TileMapComponent to entity
     * @param registry ECS registry reference
     * @param entity Entity to add component to
     * @param hasUnsavedChanges Reference to unsaved changes flag
     */
    void AddTileMapComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity, bool& hasUnsavedChanges);
};
