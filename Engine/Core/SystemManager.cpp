/**
 * @file SystemManager.cpp
 * @author Adi (100%)
 * @brief Implementation of SystemManager for GP2Engine
 *
 * SystemManager initializes and manages all engine subsystems in the correct order.
 * Initialization order: Input → Renderer → ECS → Audio
 * Shutdown order: Audio → ECS → Renderer (reverse)
 */

#include "SystemManager.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "Logger.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Audio/AudioEngine.hpp"

namespace GP2Engine {

    SystemManager::SystemManager() = default;

    SystemManager::~SystemManager() {
        Shutdown();
    }

    bool SystemManager::Initialize(GLFWwindow* window) {
        if (m_initialized) {
            LOG_WARNING("SystemManager already initialized");
            return false;
        }

        LOG_INFO("Initializing engine systems...");

        // Initialize Input system
        Input::Initialize(window);
        LOG_INFO("Input system initialized");

        // Initialize Renderer
        if (!Renderer::Initialize(window)) {
            LOG_ERROR("Failed to initialize Renderer");
            return false;
        }
        LOG_INFO("Renderer initialized");

        // Initialize ECS (Registry is already constructed as value member)
        LOG_INFO("ECS initialized");

        // Initialize Audio
        InitializeAudio();

        m_initialized = true;
        LOG_INFO("Engine systems initialized successfully");
        return true;
    }

    void SystemManager::Shutdown() {
        if (!m_initialized) return;

        LOG_INFO("Shutting down engine systems...");

        // Shutdown in reverse order of initialization

        // Shutdown Audio
        if (m_audioInitialized) {
            try {
                DKAudioEngine::Shutdown();
                LOG_INFO("Audio shutdown complete");
            } catch (const std::exception& e) {
                LOG_ERROR(std::string("Audio shutdown error: ") + e.what());
            }
            m_audioInitialized = false;
        }

        // ECS registry cleanup is thru destructor
        LOG_INFO("ECS cleanup complete");

        // Shutdown Renderer
        Renderer::Shutdown();
        LOG_INFO("Renderer shutdown complete");

        m_initialized = false;
        LOG_INFO("Engine systems shutdown complete");
    }

    void SystemManager::BeginFrame() {
        // Update Time system
        Time::Update();

        // Poll for input and window events
        glfwPollEvents();
    }

    void SystemManager::EndFrame(GLFWwindow* window) {
        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Reset input frame states (pressed/released)
        Input::ResetFrameStates();

        // Apply frame rate limiting
        Time::LimitFrameRate();
    }

    void SystemManager::InitializeAudio() {
        LOG_INFO("Initializing Audio...");

        try {
            DKAudioEngine::Init();
            m_audioInitialized = true;
            LOG_INFO("Audio initialized successfully");
        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Audio initialization failed: ") + e.what());
            LOG_INFO("Continuing without audio");
            m_audioInitialized = false;
        }
    }

} // namespace GP2Engine
