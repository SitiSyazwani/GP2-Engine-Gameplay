/**
 * @file DebugUI.h
 * @author Syazwani (100%)
 * @brief Defines the structure and interface for the Engine's Developer Debug Console (ImGui).
 *
 * This class provides a centralized set of runtime tools for debugging, performance profiling,
 * entity inspection, and stress testing. It orchestrates multiple panels (Performance, ECS
 * Inspector, Stress Test) to assist with rapid iteration and verification of engine systems.
 */

#pragma once

#include <Engine.hpp>

namespace Hollows {

    class DebugUI {
    public:
        DebugUI();
        ~DebugUI() = default;

        // Main render function
        void Render(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity, float* frameTimeHistory, int frameIndex, float& playerSpeed,
            bool& showCollisionBoxes, bool& showVelocityVectors, bool& showTileMapEditor, GP2Engine::Vector2D& m_hoveredTileCoords, int hoveredTileValue,
            GP2Engine::TileRenderer& tileRenderer, GP2Engine::TileMap& tilemap);

        // Public method for updating stress test objects
        void UpdateStressTestObjects(GP2Engine::Registry& registry, float deltaTime);

        //TileMap public operation
        int GetSelectedTileType() const { return m_selectedTileType; }
        void SetTileTextureIDs(unsigned int floorID, unsigned int wallID);
        void UpdateTileEditorLogic(GP2Engine::TileRenderer& tileRenderer, GP2Engine::TileMap& tileMap, GP2Engine::Vector2D& hoveredTileCoords,
                                    int& hoveredTileValue, bool showGridToggle);

        void SetLevelEditor(GP2Engine::LevelEditor* levelEditor) {
            m_LevelEditor = levelEditor;
        }
        // End of public methods


    private:
        // Panel visibility states (closed by default)
        bool m_showPerformance = false;
        bool m_showEntityInspector = false;
        bool m_showPlayerPanel = false;
        bool m_showControlsPanel = false;
        bool m_showDebugVisualization = false;
        bool m_showStressTest = false;
        bool m_showTileMapEditorPanel = false;
        bool m_showLevelEditor = false;
        GP2Engine::LevelEditor* m_LevelEditor = nullptr;

        // Stress test state
        bool m_stressTestActive = false;
        int m_stressTestObjectCount = 1000; // Optimized for 60+ FPS
        float m_stressTestAnimationSpeed = 1.0f; // Animation speed multiplier
        std::vector<GP2Engine::EntityID> m_stressTestEntities;
        std::unordered_map<GP2Engine::EntityID, GP2Engine::Vector2D> m_stressTestVelocities; // Physics velocities

        // Shared sprite instances for stress test (major optimization!)
        std::shared_ptr<GP2Engine::Sprite> m_stressTestPlayerSprite;
        std::shared_ptr<GP2Engine::Sprite> m_stressTestMonsterSprite;

        // Performance panel update timing
        float m_performanceUpdateTimer = 0.0f;
        const float m_performanceUpdateInterval = 0.5f; // Update every 500ms

        // Cached performance data (updated at intervals)
        float m_cachedInputTime = 0.0f;
        float m_cachedPhysicsTime = 0.0f;
        float m_cachedGraphicsTime = 0.0f;
        float m_cachedUITime = 0.0f;
        float m_cachedInputPercent = 0.0f;
        float m_cachedPhysicsPercent = 0.0f;
        float m_cachedGraphicsPercent = 0.0f;
        float m_cachedUIPercent = 0.0f;

        // Entity Inspector state
        GP2Engine::EntityID m_selectedEntity = GP2Engine::INVALID_ENTITY;
        std::string m_tagNameBuffer;
        std::string m_tagGroupBuffer;

        // Tilemap Editor State
        int m_selectedTileType = 1;
        unsigned int m_floorTextureID = 0;
        unsigned int m_wallTextureID = 0;

        // Performance metrics
        float m_cachedAvgFrameTime = 0.0f;

        // Constants
        static constexpr float SCREEN_WIDTH = 1024.0f;
        static constexpr float SCREEN_HEIGHT = 768.0f;
        static constexpr int STRESS_TEST_GRID_SIZE = 50;
        static constexpr int FRAME_HISTORY_SIZE = 120;

        // Panel drawing functions
        void DrawPerformancePanel(float* frameTimeHistory, int frameIndex);
        void DrawEntityInspector(GP2Engine::Registry& registry);
        void DrawPlayerPanel(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity, float& playerSpeed);
        void DrawControlsPanel(GP2Engine::Registry& registry, GP2Engine::EntityID playerEntity);
        void DrawDebugVisualizationPanel(bool& showCollisionBoxes, bool& showVelocityVectors);
        void DrawTileEditorPanel(const GP2Engine::Vector2D& hoveredTileCoords, int hoveredTileValue, GP2Engine::TileRenderer& tileRenderer, GP2Engine::TileMap& tilemap);

        // Entity Inspector helper functions
        void DrawEntityManagement(GP2Engine::Registry& registry);
        void DrawEntityList(GP2Engine::Registry& registry);
        void DrawSelectedEntityDetails(GP2Engine::Registry& registry);
        void DrawComponentEditor(GP2Engine::Registry& registry, GP2Engine::EntityID entity);

        // Entity operations
        void CreateNewEntity(GP2Engine::Registry& registry);
        void CopyEntity(GP2Engine::Registry& registry, GP2Engine::EntityID sourceEntity);
        void DeleteEntity(GP2Engine::Registry& registry, GP2Engine::EntityID entity);

        // Component operations
        void AddTransform2D(GP2Engine::Registry& registry, GP2Engine::EntityID entity);
        void AddSpriteComponent(GP2Engine::Registry& registry, GP2Engine::EntityID entity);
        void AddTag(GP2Engine::Registry& registry, GP2Engine::EntityID entity);

        // Stress test operations
        void DrawStressTestPanel(GP2Engine::Registry& registry);
        void StartStressTest(GP2Engine::Registry& registry);
        void StopStressTest(GP2Engine::Registry& registry);

};

} // namespace Hollows