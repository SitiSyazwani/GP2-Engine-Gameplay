/**
 * @file Application.cpp
 * @author Adi (100%)
 * @brief Implementation of the Application class - the entry point for GP2Engine sandbox games
 *
 * This file implements the complete application lifecycle management including
 * window creation, system initialization, the main game loop, and cleanup.
 *
 * Implementation details:
 * - GLFW is used for window and input management
 * - SystemManager handles all engine subsystem initialization/shutdown
 * - Time system owns all timing calculations
 * - Logger is used for error reporting 
 * - FPS display on window title
 *
 * Design pattern: Template Method
 * - Application defines the skeleton of the game loop
 * - Derived classes fill in game-specific logic through virtual methods
 */

#include "Application.hpp"
#include "Time.hpp"
#include "Logger.hpp"
#include "../Graphics/Font.hpp"

namespace GP2Engine {

    Application::Application() {
        // Application starts in default state
        // Systems are initialized in Init()
    }

    Application::~Application() {
        Shutdown();
    }

    bool Application::Init(int width, int height, const char* title) {
        // Initialize GLFW
        if (!glfwInit()) {
            LOG_ERROR("Failed to initialize GLFW");
            return false;
        }

        // Create window
        m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!m_window) {
            LOG_ERROR("Failed to create GLFW window");
            glfwTerminate();
            return false;
        }

        // Set up OpenGL context
        glfwMakeContextCurrent(m_window);

        // Initialize all engine systems (Input, Renderer, Audio, ECS)
        if (!m_systemManager.Initialize(m_window)) {
            LOG_ERROR("Failed to initialize engine systems");
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return false;
        }

        // Initialization complete
        m_running = true;
        m_baseTitle = title;

        LOG_INFO("Application initialized successfully");
        return true;
    }

    void Application::Run() {
        // Call sandbox game initialization first (backward compatibility)
        // This allows the application to push layers in OnStart()
        OnStart();

        // Initialize all layers (call OnStart for each)
        std::vector<Layer*> allLayers = m_layerStack.GetAllLayers();
        for (Layer* layer : allLayers) {
            layer->OnStart(GetRegistry());
        }

        // Main game loop - runs until window is closed
        while (m_running && !glfwWindowShouldClose(m_window)) {
            // Begin frame: Time system calculates deltaTime, poll input events
            m_systemManager.BeginFrame();

            // Update window title with FPS
            UpdateWindowTitle();

            // Update all layers
            allLayers = m_layerStack.GetAllLayers();
            for (Layer* layer : allLayers) {
                layer->Update(GetRegistry(), Time::DeltaTime());
            }

            // Call game logic update (backward compatibility)
            Update(Time::DeltaTime());

            // Render all layers
            for (Layer* layer : allLayers) {
                layer->Render(GetRegistry());
            }

            // Call game rendering (backward compatibility)
            Render();

            // End frame: swap buffers, limit FPS, reset input states
            m_systemManager.EndFrame(m_window);
        }

        // Shutdown all layers (call OnShutdown in reverse order)
        std::vector<Layer*> reversedLayers = m_layerStack.GetAllLayersReversed();
        for (Layer* layer : reversedLayers) {
            layer->OnShutdown(GetRegistry());
        }
    }

    void Application::Close() {
        if (m_window) {
            glfwSetWindowShouldClose(m_window, true);
        }
    }

    void Application::Shutdown() {
        // Guard against double-shutdown
        if (!m_running) return;

        // Shutdown systems in reverse order of initialization
        m_systemManager.Shutdown();

        // Cleanup static graphics resources
        Font::ShutdownFreeTypeLibrary();

        // Destroy window
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        // Terminate GLFW
        glfwTerminate();

        m_running = false;
    }

    void Application::UpdateWindowTitle() {
        float deltaTime = Time::DeltaTime();
        m_titleUpdateTimer += deltaTime;

        if (m_titleUpdateTimer >= 0.5f) {
            // Format: "Game Title - FPS: 60 | Frame Time: 16.67ms"
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%s - FPS: %d | Frame Time: %.2fms",
                     m_baseTitle.c_str(), Time::GetFPS(), deltaTime * 1000.0f);

            glfwSetWindowTitle(m_window, buffer);
            m_titleUpdateTimer = 0.0f;
        }
    }

    void Application::PushLayer(Layer* layer) {
        m_layerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* overlay) {
        m_layerStack.PushOverlay(overlay);
    }

} // namespace GP2Engine
