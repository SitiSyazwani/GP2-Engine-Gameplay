/**
 * @file SystemManager.hpp
 * @author Adi (100%)
 * @brief System manager for GP2Engine subsystems
 *
 * SystemManager handles initialization and shutdown of all engine subsystems
 * in the correct order. It also manages per-frame operations like timing,
 * input polling, and buffer swapping.
 *
 * Responsibilities:
 * - Initialize systems: Input → Renderer → ECS → Audio
 * - Shutdown systems in reverse order
 * - Per-frame: Update Time, poll input, swap buffers, limit FPS
 * - Provide access to ECS Registry
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../ECS/Registry.hpp"

namespace GP2Engine {

    /**
     * @brief Manages all engine subsystems
     *
     * SystemManager owns the ECS Registry and coordinates initialization,
     * shutdown, and per-frame operations for all engine systems.
     */
    class SystemManager {
    public:
        SystemManager();
        ~SystemManager();

        // Non-copyable
        SystemManager(const SystemManager&) = delete;
        SystemManager& operator=(const SystemManager&) = delete;

        /**
         * @brief Initialize all engine systems
         *
         * Initialization order: Input → Renderer → ECS → Audio
         * Audio failure is non-fatal (continues without audio).
         *
         * @param window GLFW window handle
         * @return true if initialization successful, false on failure
         */
        bool Initialize(GLFWwindow* window);

        /**
         * @brief Shutdown all engine systems
         *
         * Shutdown order: Audio → ECS → Renderer (reverse of init).
         * Safe to call multiple times.
         */
        void Shutdown();

        /**
         * @brief Begin a new frame
         *
         * Updates Time system and polls input events.
         * Called by Application at start of each frame.
         */
        void BeginFrame();

        /**
         * @brief End the current frame
         *
         * Swaps buffers, resets input states, and limits frame rate.
         * Called by Application at end of each frame.
         *
         * @param window GLFW window handle
         */
        void EndFrame(GLFWwindow* window);

        /**
         * @brief Get the ECS Registry
         *
         * @return Reference to the ECS Registry
         */
        Registry& GetRegistry() { return m_registry; }

        /**
         * @brief Check if systems are initialized
         *
         * @return true if Initialize() succeeded, false otherwise
         */
        bool IsInitialized() const { return m_initialized; }

    private:
        Registry m_registry;
        bool m_initialized = false;
        bool m_audioInitialized = false;

        void InitializeAudio();
    };

} // namespace GP2Engine
