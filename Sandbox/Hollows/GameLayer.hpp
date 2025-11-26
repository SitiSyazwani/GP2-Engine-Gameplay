/**
 * @file GameLayer.hpp
 * @author Adi (100%)
 * @brief Game logic layer for Hollows - decoupled from Application
 *
 * This layer contains all game-specific logic, systems, and state management.
 * It can be used by both the standalone game and the editor's play mode,
 * providing a single source of truth for game behavior.
 */

#pragma once

#include <Engine.hpp>
#include <memory>
#include "DebugUI.h"
#include "PlayerController.hpp"
#include "DebugLogic.hpp"

enum class GameState {
    MainMenu,
    Playing,
    Settings
};

/**
 * @brief Main game logic layer
 *
 * Manages game state, systems, cameras, and all gameplay logic.
 * Separate from Application infrastructure for better modularity.
 */
class GameLayer : public GP2Engine::Layer {
private:
    // === GAME STATE ===
    GameState m_gameState = GameState::MainMenu;
    std::unique_ptr<GP2Engine::MainMenu> m_mainMenu;
    bool m_gameInitialized = false;
    GLFWwindow* m_window = nullptr;  // Store window reference

    // === DEBUG UI STATE ===
    bool m_showDebugUI = true;
    bool m_showCollisionBoxes = false;
    bool m_showVelocityVectors = false;
    bool m_showGridToggle = false;

    // === TILEMAP STATE ===
    std::unique_ptr<GP2Engine::TileRenderer> m_tileRenderer;
    std::unique_ptr<GP2Engine::TileMap> m_tileMap;
    GP2Engine::EntityID m_tileMapEntity;
    GP2Engine::Vector2D m_hoveredTileCoords = GP2Engine::Vector2D(-1.0f, -1.0f);
    int m_hoveredTileValue = -1;

    // === CAMERAS ===
    GP2Engine::Camera m_gameCamera;
    GP2Engine::Camera* m_currentCamera = &m_gameCamera;
    float m_playerSpeed = 300.0f;
    float m_cameraFollowSpeed = 5.0f;
    GP2Engine::Vector2D m_cameraTargetPos{512.0f, 384.0f};

    // === PERFORMANCE METRICS ===
    float m_frameTimeHistory[120] = { 0.0f };
    int m_frameIndex = 0;

    // === DEBUG UI ===
    Hollows::DebugUI m_debugUI;

    // === SYSTEMS ===
    GP2Engine::RenderSystem m_renderSystem;
    GP2Engine::ButtonSystem m_buttonSystem;
    GP2Engine::AISystem m_aiSystem;
    Hollows::PlayerController m_playerController;
    Hollows::DebugLogic m_debugLogic;
    int m_backgroundMusicChannel = -1;

    // === LEVEL EDITOR ===
    std::unique_ptr<GP2Engine::LevelEditor> m_levelEditor;

    // === MOUSE STATE ===
    GP2Engine::Vector2D m_lastMousePos{ 0.0f, 0.0f };
    bool m_firstMouse = true;
    bool m_cameraTogglePressed = false;

    // === EVENT SYSTEM LISTENER IDs ===
    GP2Engine::EventSystem::ListenerID m_startGameListenerId = 0;
    GP2Engine::EventSystem::ListenerID m_settingsListenerId = 0;
    GP2Engine::EventSystem::ListenerID m_quitListenerId = 0;
    GP2Engine::EventSystem::ListenerID m_hoverListenerId = 0;

public:
    /**
     * @brief Constructor - takes window reference
     */
    GameLayer(GLFWwindow* window);
    virtual ~GameLayer() = default;

    // === LAYER LIFECYCLE ===
    void OnStart(GP2Engine::Registry& registry) override;
    void Update(GP2Engine::Registry& registry, float deltaTime) override;
    void Render(GP2Engine::Registry& registry) override;
    void OnShutdown(GP2Engine::Registry& registry) override;

private:
    // === INITIALIZATION HELPERS ===
    void InitializeMainMenu(GP2Engine::Registry& registry);
    void InitializeGame(GP2Engine::Registry& registry);

    // === UPDATE HELPERS ===
    void UpdateMainMenu(GP2Engine::Registry& registry, float deltaTime);
    void UpdateGame(GP2Engine::Registry& registry, float deltaTime);
    void UpdatePerformanceMetrics(float deltaTime);

    // === INPUT HANDLERS ===
    void HandleContinuousCameraInput(float deltaTime);
    void HandleEditorKeyboardInput();

    // === RENDERING HELPERS ===
    void RenderMainMenu(GP2Engine::Registry& registry);
    void RenderGame(GP2Engine::Registry& registry);
};
