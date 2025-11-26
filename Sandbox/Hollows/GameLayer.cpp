/**
 * @file GameLayer.cpp
 * @author Adi (100%)
 * @brief Implementation of the game logic layer
 */

#include "GameLayer.hpp"
#include "GameInitializer.hpp"
#include <Core/Events.hpp>
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

GameLayer::GameLayer(GLFWwindow* window)
    : m_window(window) {
}

void GameLayer::OnStart(GP2Engine::Registry& registry) {
    std::cout << "GameLayer::OnStart - Initializing game layer" << std::endl;

    // Initialize main menu
    InitializeMainMenu(registry);
}

void GameLayer::InitializeMainMenu(GP2Engine::Registry& registry) {
    std::cout << "\n[EventSystem] Subscribing to menu events..." << std::endl;

    // Subscribe to Start Game event
    m_startGameListenerId = GP2Engine::EventSystem::Subscribe<GP2Engine::MenuStartGameEvent>(
        [this](const GP2Engine::MenuStartGameEvent&) {
            std::cout << "[EventSystem] Received MenuStartGameEvent" << std::endl;
        });

    // Subscribe to Settings event
    m_settingsListenerId = GP2Engine::EventSystem::Subscribe<GP2Engine::MenuOpenSettingsEvent>(
        [this](const GP2Engine::MenuOpenSettingsEvent&) {
            std::cout << "[EventSystem] Received MenuOpenSettingsEvent" << std::endl;
        });

    // Subscribe to Quit event
    m_quitListenerId = GP2Engine::EventSystem::Subscribe<GP2Engine::MenuQuitGameEvent>(
        [this](const GP2Engine::MenuQuitGameEvent&) {
            std::cout << "[EventSystem] Received MenuQuitGameEvent" << std::endl;
        });

    // Subscribe to Button Hover events
    m_hoverListenerId = GP2Engine::EventSystem::Subscribe<GP2Engine::MenuButtonHoverEvent>(
        [this](const GP2Engine::MenuButtonHoverEvent& e) {
            std::cout << "[EventSystem] Button hovered: " << e.buttonName << std::endl;
        });

    std::cout << "[EventSystem] Subscribed to menu events\n" << std::endl;

    // Load main menu scene
    if (!GP2Engine::JsonSerializer::LoadScene(registry, "assets/scenes/main_menu.json")) {
        std::cerr << "Failed to load main menu scene!" << std::endl;
    }

    // Initialize main menu camera
    m_mainMenu = std::make_unique<GP2Engine::MainMenu>();
    auto& renderer = GP2Engine::Renderer::GetInstance();
    int windowWidth = renderer.GetWidth();
    int windowHeight = renderer.GetHeight();
    if (!m_mainMenu->Initialize(windowWidth, windowHeight)) {
        std::cerr << "Failed to initialize MainMenu camera!" << std::endl;
    }

    // Attach ButtonComponent to menu entities
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        auto* tag = registry.GetComponent<GP2Engine::Tag>(entity);
        if (!tag) continue;

        if (tag->name == "StartButton") {
            registry.AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::StartGame));
        } else if (tag->name == "QuitButton") {
            registry.AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::QuitGame));
        } else if (tag->name == "SettingsButton") {
            registry.AddComponent(entity, GP2Engine::ButtonComponent(GP2Engine::ButtonComponent::Action::OpenSettings));
        }
    }

    std::cout << "ButtonComponents attached to menu entities" << std::endl;
}

void GameLayer::InitializeGame(GP2Engine::Registry& registry) {
    if (m_gameInitialized) return;

    std::cout << "Initializing game systems..." << std::endl;

    // Initialize game systems
    Hollows::GameInitializer::Initialize(registry, m_window, m_playerController, m_gameCamera, m_playerSpeed, m_aiSystem);

    m_currentCamera = &m_gameCamera;

    // Load sound effects
    DKAudioEngine::LoadSound("assets/audio_files/keyboard_input.wav", false, false, false);
    DKAudioEngine::LoadSound("assets/audio_files/mouse_click.wav", false, false, false);
    DKAudioEngine::LoadSound("assets/audio_files/collision.wav", false, false, false);

    // Load and play background music
    DKAudioEngine::LoadSound("assets/audio_files/background_music.wav", false, true, true);
    m_backgroundMusicChannel = DKAudioEngine::PlaySounds(
        "assets/audio_files/background_music.wav",
        Vector3{ 0.0f, 0.0f, 0.0f },
        -6.0f
    );

    std::cout << "Background music started on channel: " << m_backgroundMusicChannel << std::endl;

    // Initialize tilemap
    m_tileMap = std::make_unique<GP2Engine::TileMap>();
    m_tileRenderer = std::make_unique<GP2Engine::TileRenderer>();
    m_tileMap->LoadTileDefinitionsFromJSON("assets/scenes/test1.json");
    m_tileRenderer->m_tileMap = m_tileMap.get();
    m_tileRenderer->InitializeDimensions();

    // Create tilemap entity
    m_tileMapEntity = registry.CreateEntity();
    registry.AddComponent(m_tileMapEntity, GP2Engine::Transform2D(GP2Engine::Vector2D(0.0f, 0.0f)));
    registry.AddComponent(m_tileMapEntity, GP2Engine::TileMapComponent(m_tileMap.get(), m_tileRenderer.get(), -1));
    registry.AddComponent(m_tileMapEntity, GP2Engine::Tag("TileMap"));

    // Initialize level editor
    m_levelEditor = std::make_unique<GP2Engine::LevelEditor>();
    m_levelEditor->Initialize(m_tileMap.get(), m_tileRenderer.get());
    m_debugUI.SetLevelEditor(m_levelEditor.get());
    m_showGridToggle = m_levelEditor->IsGridVisible();

    auto& editorCamera = m_levelEditor->GetEditorCamera();
    int gridCols = m_tileMap->GetGridCols();
    int gridRows = m_tileMap->GetGridRows();
    float levelWidth = gridCols * 64.0f;
    float levelHeight = gridRows * 64.0f;

    editorCamera.SetOrthographic(-512.0f, 512.0f, 384.0f, -384.0f);
    editorCamera.SetPosition(glm::vec3(levelWidth / 2, levelHeight / 2, 0.0f));
    editorCamera.SetZoom(1.0f);

    // Initialize mouse position
    double startX, startY;
    GP2Engine::Input::GetMousePosition(startX, startY);
    m_lastMousePos = GP2Engine::Vector2D((float)startX, (float)startY);

    m_gameInitialized = true;
    std::cout << "Game systems initialized successfully!" << std::endl;
}

void GameLayer::Update(GP2Engine::Registry& registry, float deltaTime) {
    if (m_gameState == GameState::MainMenu) {
        UpdateMainMenu(registry, deltaTime);
    } else if (m_gameState == GameState::Playing) {
        UpdateGame(registry, deltaTime);
    }
}

void GameLayer::UpdateMainMenu(GP2Engine::Registry& registry, float deltaTime) {
    // Update ButtonSystem
    m_buttonSystem.Update(registry, deltaTime);

    // Check for button clicks
    GP2Engine::ButtonComponent::Action clickedAction;
    GP2Engine::EntityID clickedButton = m_buttonSystem.GetClickedButton(registry, clickedAction);

    if (clickedButton != GP2Engine::INVALID_ENTITY) {
        switch (clickedAction) {
            case GP2Engine::ButtonComponent::Action::StartGame:
                std::cout << "Start button clicked" << std::endl;
                InitializeGame(registry);
                m_gameState = GameState::Playing;
                break;

            case GP2Engine::ButtonComponent::Action::QuitGame:
                std::cout << "Quit button clicked" << std::endl;
                glfwSetWindowShouldClose(m_window, true);
                break;

            case GP2Engine::ButtonComponent::Action::OpenSettings:
                std::cout << "Settings button clicked (not implemented)" << std::endl;
                break;

            default:
                break;
        }
    }
}

void GameLayer::UpdateGame(GP2Engine::Registry& registry, float deltaTime) {
    // Update audio
    DKAudioEngine::Update();

    // Update performance metrics
    UpdatePerformanceMetrics(deltaTime);

    // Toggle debug UI with F1
    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::F1)) {
        m_showDebugUI = !m_showDebugUI;
    }

    // Camera toggle with C key
    bool cPressed = GP2Engine::Input::IsKeyPressed(GP2Engine::Key::C);
    if (cPressed && !m_cameraTogglePressed) {
        bool useEditorCam = !m_levelEditor->IsUsingEditorCamera();
        m_levelEditor->SetUseEditorCamera(useEditorCam);

        if (useEditorCam) {
            m_currentCamera = &m_levelEditor->GetEditorCamera();
            std::cout << "Switched to EDITOR camera" << std::endl;
        } else {
            m_currentCamera = &m_gameCamera;
            std::cout << "Switched to GAME camera" << std::endl;
        }
        m_cameraTogglePressed = true;
    } else if (!cPressed) {
        m_cameraTogglePressed = false;
    }

    // Editor camera mouse movement
    if (m_levelEditor->IsUsingEditorCamera()) {
        double currentX, currentY;
        GP2Engine::Input::GetMousePosition(currentX, currentY);
        GP2Engine::Vector2D currentMousePos((float)currentX, (float)currentY);

        if (!m_firstMouse) {
            GP2Engine::Vector2D mouseDelta = currentMousePos - m_lastMousePos;
            bool rightPressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Right);
            m_levelEditor->HandleMouseMove(mouseDelta, rightPressed);
        } else {
            m_firstMouse = false;
        }

        m_lastMousePos = currentMousePos;
    } else {
        m_firstMouse = true;
    }

    // Continuous camera input
    if (m_levelEditor->IsUsingEditorCamera()) {
        HandleContinuousCameraInput(deltaTime);
    }

    // Editor input
    if (m_levelEditor && m_levelEditor->IsUsingEditorCamera()) {
        double mouseX, mouseY;
        GP2Engine::Input::GetMousePosition(mouseX, mouseY);
        GP2Engine::Vector2D mousePos((float)mouseX, (float)mouseY);

        bool leftPressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);
        bool rightPressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Right);

        m_levelEditor->HandleMouseInput(mousePos, leftPressed, rightPressed);
        HandleEditorKeyboardInput();
    }

    // Update game systems
    if (!m_levelEditor->IsUsingEditorCamera()) {
        m_playerController.SetSpeed(m_playerSpeed);
        m_playerController.Update(registry, deltaTime);
        m_aiSystem.Update(registry, deltaTime);

        // Update camera follow
        GP2Engine::EntityID playerEntity = m_playerController.GetPlayerEntity();
        auto* playerTransform = registry.GetComponent<GP2Engine::Transform2D>(playerEntity);
        if (playerTransform) {
            m_cameraTargetPos = playerTransform->position;

            int gridCols = m_tileMap->GetGridCols();
            int gridRows = m_tileMap->GetGridRows();
            float levelWidth = gridCols * 64.0f;
            float levelHeight = gridRows * 64.0f;

            float zoomLevel = 1.5f;
            float baseWidth = 1024.0f;
            float baseHeight = 768.0f;
            float viewportWidth = baseWidth / zoomLevel;
            float viewportHeight = baseHeight / zoomLevel;

            float halfViewWidth = viewportWidth / 2.0f;
            float halfViewHeight = viewportHeight / 2.0f;

            m_cameraTargetPos.x = std::max(halfViewWidth, std::min(levelWidth - halfViewWidth, m_cameraTargetPos.x));
            m_cameraTargetPos.y = std::max(halfViewHeight, std::min(levelHeight - halfViewHeight, m_cameraTargetPos.y));

            glm::vec3 currentPos = m_gameCamera.GetPosition();
            GP2Engine::Vector2D currentPos2D(currentPos.x, currentPos.y);

            float lerpFactor = 1.0f - exp(-m_cameraFollowSpeed * deltaTime);
            GP2Engine::Vector2D newPos;
            newPos.x = currentPos2D.x + (m_cameraTargetPos.x - currentPos2D.x) * lerpFactor;
            newPos.y = currentPos2D.y + (m_cameraTargetPos.y - currentPos2D.y) * lerpFactor;

            m_gameCamera.SetPosition(newPos);

            float halfWidth = (baseWidth / zoomLevel) / 2.0f;
            float halfHeight = (baseHeight / zoomLevel) / 2.0f;
            m_gameCamera.SetOrthographic(-halfWidth, halfWidth, halfHeight, -halfHeight);
        }
    }

    // Update stress test objects
    m_debugUI.UpdateStressTestObjects(registry, deltaTime);

    // Update level editor
    if (m_levelEditor) {
        m_levelEditor->Update(deltaTime);
    }

    // Update tile editor logic
    m_debugUI.UpdateTileEditorLogic(
        *m_tileRenderer,
        *m_tileMap,
        m_hoveredTileCoords,
        m_hoveredTileValue,
        m_showGridToggle);
}

void GameLayer::Render(GP2Engine::Registry& registry) {
    auto& renderer = GP2Engine::Renderer::GetInstance();
    renderer.Clear();

    // Get window size and set viewport
    int width = renderer.GetWidth();
    int height = renderer.GetHeight();
    glViewport(0, 0, width, height);

    if (m_gameState == GameState::MainMenu) {
        RenderMainMenu(registry);
    } else if (m_gameState == GameState::Playing) {
        RenderGame(registry);
    }
}

void GameLayer::RenderMainMenu(GP2Engine::Registry& registry) {
    auto& renderer = GP2Engine::Renderer::GetInstance();
    int width = renderer.GetWidth();
    int height = renderer.GetHeight();

    // Use menu camera
    if (m_mainMenu) {
        GP2Engine::Camera& menuCamera = m_mainMenu->GetCamera();
        menuCamera.SetOrthographic(0.0f, static_cast<float>(width),
                                  static_cast<float>(height), 0.0f);
        m_renderSystem.Render(registry, menuCamera);
    } else {
        m_renderSystem.Render(registry, *m_currentCamera);
    }
}

void GameLayer::RenderGame(GP2Engine::Registry& registry) {
    auto& renderer = GP2Engine::Renderer::GetInstance();

    // Render game scene
    m_renderSystem.Render(registry, *m_currentCamera);

    auto& debugRenderer = renderer.GetDebugRenderer();
    debugRenderer.Begin();

    // Level editor visuals
    if (m_levelEditor) {
        m_levelEditor->Render();
        m_showGridToggle = m_levelEditor->IsGridVisible();

        if (m_showGridToggle) {
            m_tileRenderer->RenderDebugGrid(renderer);
        }
    }

    // Debug physics
    if (!m_levelEditor->IsUsingEditorCamera()) {
        m_debugLogic.RenderDebugPhysics(registry, m_playerController.GetPlayerEntity(),
            m_playerController.GetVelocity(), m_showCollisionBoxes, m_showVelocityVectors);
    }

    debugRenderer.Flush(renderer);

    // Render debug UI
    if (m_showDebugUI) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_debugUI.Render(registry, m_playerController.GetPlayerEntity(), m_frameTimeHistory,
            m_frameIndex, m_playerSpeed, m_showCollisionBoxes, m_showVelocityVectors,
            m_showGridToggle, m_hoveredTileCoords, m_hoveredTileValue, *m_tileRenderer, *m_tileMap);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void GameLayer::OnShutdown(GP2Engine::Registry& registry) {
    (void)registry;  // Unused parameter

    std::cout << "GameLayer::OnShutdown - Cleaning up" << std::endl;

    // Unsubscribe from events
    if (m_startGameListenerId != 0) {
        GP2Engine::EventSystem::Unsubscribe<GP2Engine::MenuStartGameEvent>(m_startGameListenerId);
    }
    if (m_settingsListenerId != 0) {
        GP2Engine::EventSystem::Unsubscribe<GP2Engine::MenuOpenSettingsEvent>(m_settingsListenerId);
    }
    if (m_quitListenerId != 0) {
        GP2Engine::EventSystem::Unsubscribe<GP2Engine::MenuQuitGameEvent>(m_quitListenerId);
    }
    if (m_hoverListenerId != 0) {
        GP2Engine::EventSystem::Unsubscribe<GP2Engine::MenuButtonHoverEvent>(m_hoverListenerId);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GameLayer::UpdatePerformanceMetrics(float deltaTime) {
    m_frameTimeHistory[m_frameIndex] = deltaTime;
    m_frameIndex = (m_frameIndex + 1) % 120;
}

void GameLayer::HandleContinuousCameraInput(float deltaTime) {
    float moveSpeed = 500.0f * deltaTime;
    glm::vec3 movement(0.0f);

    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::W)) movement.y -= moveSpeed;
    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::S)) movement.y += moveSpeed;
    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::A)) movement.x -= moveSpeed;
    if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::D)) movement.x += moveSpeed;

    if (glm::length(movement) > 0.0f) {
        m_levelEditor->GetEditorCamera().Move(movement);
    }
}

void GameLayer::HandleEditorKeyboardInput() {
    if (!m_levelEditor || !m_levelEditor->IsUsingEditorCamera()) return;

    auto& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;

    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::W) ||
        GP2Engine::Input::IsKeyHeld(GP2Engine::Key::W)) {
        m_levelEditor->HandleKeyboardInput(GLFW_KEY_W, GLFW_PRESS);
    }
    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::A) ||
        GP2Engine::Input::IsKeyHeld(GP2Engine::Key::A)) {
        m_levelEditor->HandleKeyboardInput(GLFW_KEY_A, GLFW_PRESS);
    }
    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::S) ||
        GP2Engine::Input::IsKeyHeld(GP2Engine::Key::S)) {
        m_levelEditor->HandleKeyboardInput(GLFW_KEY_S, GLFW_PRESS);
    }
    if (GP2Engine::Input::IsKeyPressed(GP2Engine::Key::D) ||
        GP2Engine::Input::IsKeyHeld(GP2Engine::Key::D)) {
        m_levelEditor->HandleKeyboardInput(GLFW_KEY_D, GLFW_PRESS);
    }
}
