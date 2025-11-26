/**
 * @file Application.hpp
 * @author Adi (100%)
 * @brief Core application framework providing the foundation for all GP2Engine games
 *
 * The Application class is the entry point for creating games with GP2Engine.
 * It manages the complete application lifecycle from initialization through
 * the main game loop to shutdown, handling all low-level systems automatically.
 *
 * Architecture:
 * - Uses the Template Method pattern - derive from this class and override
 *   OnStart(), Update(), and Render() to implement your game
 * - Integrates with SystemManager to handle all engine subsystems
 *
 * Responsibilities:
 * - GLFW window creation and OpenGL context management
 * - Main game loop execution
 * - System initialization and shutdown in correct order
 * - Window title updates with FPS display
 * - Event polling and frame synchronization
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include "SystemManager.hpp"
#include "LayerStack.hpp"

namespace GP2Engine {

    /**
     * @brief Base class for all GP2Engine applications
     *
     * Application manages the complete lifecycle of a game, from window creation
     * to the main game loop. Derive from this class and override the virtual
     * methods to implement game specific logic.
     *
     * Lifecycle:
     * 1. Constructor - Create Application instance
     * 2. Init() - Initialize GLFW, create window, start engine systems
     * 3. OnStart() - Called once before main loop (setup game here)
     * 4. Run() - Main game loop (Update/Render called every frame)
     * 5. Shutdown() - Cleanup resources, destroy window
     * 6. Destructor - Final cleanup
     *
     * Virtual methods to override:
     * - OnStart() - One time game initialization
     * - Update(float deltaTime) - Per-frame game logic
     * - Render() - Per-frame rendering
     * - OnWindowResize(int, int) - Window resize handling
     *
     * Key features:
     * - Automatic system management via SystemManager
     * - FPS display in window title
     * - Access to ECS Registry
     * - Safe shutdown via RAII
     */
    class Application {
    public:
        // === CONSTRUCTORS ===

        /**
         * @brief Default constructor
         *
         * Creates an Application instance with default state.
         * No engine systems are initialized until Init() is called.
         */
        Application();

        /**
         * @brief Virtual destructor for proper polymorphic cleanup
         *
         * Ensures all resources are released by calling Shutdown() if the
         * application is still running.
         */
        virtual ~Application();

        // === APPLICATION LIFECYCLE ===

        /**
         * @brief Initialize the application and all engine systems
         *
         * This must be called before Run(). It performs the following:
         * 1. Initializes GLFW
         * 2. Creates the application window
         * 3. Sets up OpenGL context
         * 4. Initializes SystemManager (Input, Renderer, Audio, ECS)
         *
         * @param width Window width in pixels (default: 1024)
         * @param height Window height in pixels (default: 768)
         * @param title Window title string (default: "GP2Engine")
         * @return true if all systems initialized successfully, false on any failure
         */
        bool Init(int width = 1024, int height = 768, const char* title = "GP2Engine");

        /**
         * @brief Execute the main game loop
         *
         * Runs the game loop until the window is closed. Each frame:
         * 1. Updates timing (Time system calculates deltaTime internally)
         * 2. Polls input events
         * 3. Updates window title with FPS
         * 4. Calls Update() with deltaTime
         * 5. Calls Render()
         * 6. Swaps buffers and limits frame rate
         *
         * This method blocks until the application exits.
         */
        void Run();

        /**
         * @brief Request application to close 
         *
         * Signals the main loop to exit, which triggers proper shutdown sequence.
         */
        void Close();

        /**
         * @brief Clean up all resources and shutdown engine systems
         *
         * Shuts down all systems in reverse order of initialization,
         * destroys the window, and terminates GLFW. Called automatically by destructor.
         */
        void Shutdown();

        // === VIRTUAL METHODS (OVERRIDE IN DERIVED CLASSES) ===

        /**
         * @brief One time initialization hook called before the main loop
         *
         * Override this to perform game-specific setup such as:
         * - Registering ECS components
         * - Loading assets (textures, audio, scenes)
         * - Creating initial entities
         * - Setting up cameras
         *
         * Called once after Init() succeeds, before the first frame.
         */
        virtual void OnStart() {}

        /**
         * @brief Per-frame game logic update
         *
         * Override this to implement your game logic. Common tasks:
         * - Process input and update player state
         * - Update AI and game systems
         * - Handle physics and collision
         * - Update animations
         *
         * @param deltaTime Time elapsed since last frame in seconds
         */
        virtual void Update(float deltaTime) { (void)deltaTime; }

        /**
         * @brief Per-frame rendering
         *
         * Override this to draw your game. Common tasks:
         * - Clear the screen
         * - Set camera for the frame
         * - Render entities using the Renderer
         * - Draw debug visualization
         * - Render UI
         */
        virtual void Render() {}

        /**
         * @brief Window resize event handler
         *
         * Override this to respond to window size changes. Common tasks:
         * - Update camera projection/viewport
         * - Reposition UI elements
         * - Adjust aspect ratio
         *
         * @param width New window width in pixels
         * @param height New window height in pixels
         */
        virtual void OnWindowResize(int width, int height) { (void)width; (void)height; }

        // === GETTERS ===

        /**
         * @brief Check if the application is currently running
         *
         * @return true if Init() succeeded and Shutdown() hasn't been called yet
         */
        bool IsRunning() const { return m_running; }

        /**
         * @brief Get the GLFW window handle
         *
         * Provides direct access to the window for advanced use cases like:
         * - Setting custom window callbacks
         * - Querying window properties
         * - Integrating third-party libraries (ImGui, etc.)
         *
         * @return Pointer to GLFW window, or nullptr if not initialized
         */
        GLFWwindow* GetWindow() const { return m_window; }

        /**
         * @brief Get the ECS registry for entity and component management
         *
         * Use this to:
         * - Create and destroy entities
         * - Add/remove components
         * - Query entities by component types
         * - Iterate over active entities
         *
         * @return Reference to the central ECS Registry
         */
        Registry& GetRegistry() { return m_systemManager.GetRegistry(); }

        // === LAYER MANAGEMENT ===

        /**
         * @brief Push a layer onto the layer stack
         *
         * Layers are processed in the order they are pushed. Regular layers
         * are inserted before overlays and are typically used for game logic,
         * menus, and gameplay systems.
         *
         * @param layer Pointer to layer (Application takes ownership)
         */
        void PushLayer(Layer* layer);

        /**
         * @brief Push an overlay onto the layer stack
         *
         * Overlays are always rendered on top of regular layers. They are
         * typically used for debug UI, editor panels, and HUD elements.
         *
         * @param overlay Pointer to overlay (Application takes ownership)
         */
        void PushOverlay(Layer* overlay);

    private:
        // === MEMBER VARIABLES ===

        GLFWwindow* m_window = nullptr;        // GLFW window handle
        bool m_running = false;                // Application state flag
        std::string m_baseTitle;               // Base window title
        float m_titleUpdateTimer = 0.0f;       // Timer for throttling title updates

        SystemManager m_systemManager;         // Manages all engine subsystems
        LayerStack m_layerStack;               // Manages application layers

        // === INTERNAL METHODS ===

        /**
         * @brief Update window title with FPS and frame time
         *
         * Appends FPS and frame time to the base window title.
         */
        void UpdateWindowTitle();
    };

} // namespace GP2Engine
