/**
 * @file main.cpp
 * @author Adi (100%)
 * @brief Main entry point for the Hollows game
 *
 * Simplified application class that uses the Layer system.
 * All game logic is now contained in GameLayer.
 */

#include <Engine.hpp>
#include "GameLayer.hpp"
#include <iostream>

class HollowsGame : public GP2Engine::Application {
public:
    void OnStart() override {
        // Push the game layer - all logic is in the layer
        PushLayer(new GameLayer(GetWindow()));
    }
};

// Global instance for scroll callback
static HollowsGame* g_GameInstance = nullptr;

void ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    // TODO: Forward scroll events to layers if needed
    (void)yoffset;
}

int main() {
    // Enable run-time memory check for debug builds
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize logging system
    GP2Engine::Logger& logger = GP2Engine::Logger::GetInstance();
    logger.Initialize("Hollows_Log.txt");

    std::cout << "=== Hollows Game (GP2Engine) ===" << std::endl;
    std::cout << "Starting application..." << std::endl;

    // Create and initialize application
    HollowsGame game;
    g_GameInstance = &game;

    if (!game.Init(1024, 768, "Hollows - GP2Engine")) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }

    // Set target FPS to 200
    GP2Engine::Time::SetTargetFPS(200);

    // Set scroll callback
    glfwSetScrollCallback(game.GetWindow(), ScrollCallback);

    // Run the game
    game.Run();

    std::cout << "Application shutdown complete." << std::endl;
    return 0;
}
