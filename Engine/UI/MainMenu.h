/**
 * @file MainMenu.h
 * @brief Main menu system with clickable buttons
 *
 * Renders a menu background and interactive text buttons
 * for Start, Settings, and Quit options.
 * Supports JSON-based configuration for easy customization.
 */

#pragma once

#include <Engine.hpp>
#include <memory>
#include <functional>
#include <string>
#include <nlohmann/json.hpp>

namespace GP2Engine {

/**
 * @class MainMenu
 * @brief Manages the main menu rendering and interaction
 *
 * Features:
 * - Background image rendering
 * - Clickable text buttons
 * - Mouse hover detection
 * - Callback system for button actions
 * - JSON-based configuration
 */
class MainMenu {
public:
    enum class MenuAction {
        None,
        StartGame,
        OpenSettings,
        QuitGame
    };

    /**
     * @brief Configuration structure for JSON serialization
     */
    struct Config {
        std::string backgroundTexturePath;
        std::string fontPath;
        float fontSize = 48.0f;
        float buttonSpacing = 60.0f;

        struct ButtonConfig {
            std::string text;
            std::string action;  // "StartGame", "OpenSettings", "QuitGame"
            float posX = 0.0f;   // Normalized position (0-1) or absolute
            float posY = 0.0f;
            float width = 200.0f;
            float height = 60.0f;
            bool useNormalizedPosition = true;  // If true, pos is 0-1 range
        };

        std::vector<ButtonConfig> buttons;
    };

    MainMenu();
    ~MainMenu() = default;

    /**
     * @brief Initialize menu resources (textures, fonts)
     * @param windowWidth Width of the game window
     * @param windowHeight Height of the game window
     * @return true if initialization succeeded
     */
    bool Initialize(int windowWidth, int windowHeight);

    /**
     * @brief Initialize menu from JSON configuration file
     * @param configPath Path to JSON configuration file
     * @param windowWidth Width of the game window
     * @param windowHeight Height of the game window
     * @return true if initialization succeeded
     */
    bool InitializeFromJson(const std::string& configPath, int windowWidth, int windowHeight);

    /**
     * @brief Load configuration from JSON file
     * @param configPath Path to JSON configuration file
     * @return true if loading succeeded
     */
    bool LoadFromJson(const std::string& configPath);

    /**
     * @brief Save configuration to JSON file
     * @param configPath Path to JSON configuration file
     * @return true if saving succeeded
     */
    bool SaveToJson(const std::string& configPath) const;

    /**
     * @brief Get current configuration
     */
    const Config& GetConfig() const { return m_config; }

    /**
     * @brief Set configuration and reload menu
     */
    void SetConfig(const Config& config);

    /**
     * @brief Update menu logic (mouse hover, click detection)
     * @param deltaTime Time since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render the menu background and buttons
     */
    void Render();

    /**
     * @brief Check if a menu action was triggered
     * @return The menu action that occurred
     */
    MenuAction GetTriggeredAction() const { return m_triggeredAction; }

    /**
     * @brief Reset triggered action after handling
     */
    void ClearTriggeredAction() { m_triggeredAction = MenuAction::None; }

    /**
     * @brief Handle window resize
     */
    void OnWindowResize(int width, int height);

    /**
     * @brief Get reference to the menu camera
     * @return Reference to the menu camera
     */
    GP2Engine::Camera& GetCamera() { return m_menuCamera; }

    /**
     * @brief Create button hitboxes from scene entities
     * @param registry ECS registry containing loaded menu scene
     */
    void CreateButtonsFromScene(GP2Engine::Registry& registry);

private:
    // Button structure for menu items
    struct Button {
        std::string text;
        MenuAction action;
        glm::vec2 position;     // Top-left of hitbox
        glm::vec2 size;         // Size of hitbox
        glm::vec2 textPosition; // Exact position for text rendering
        bool isHovered = false;

        // Check if point is inside button (using TOP-LEFT coordinates)
        bool Contains(float x, float y) const {
            // Standard AABB collision test (top-left origin)
            return x >= position.x && x <= position.x + size.x &&
                   y >= position.y && y <= position.y + size.y;
        }
    };

    // Configuration
    Config m_config;

    // Resources
    std::shared_ptr<GP2Engine::Texture> m_backgroundTexture;
    std::shared_ptr<GP2Engine::Font> m_font;
    std::shared_ptr<GP2Engine::Font> m_secondaryFont;

    // Camera for UI rendering
    GP2Engine::Camera m_menuCamera;

    // Menu buttons
    std::vector<Button> m_buttons;

    // Window dimensions
    int m_windowWidth = 1024;
    int m_windowHeight = 768;

    // State
    MenuAction m_triggeredAction = MenuAction::None;
    GP2Engine::Vector2D m_mousePosition{ 0.0f, 0.0f };
    bool m_mouseWasPressed = false;

    /**
     * @brief Create and position menu buttons
     */
    void CreateButtons();

    /**
     * @brief Create buttons from configuration
     */
    void CreateButtonsFromConfig();

    /**
     * @brief Update button hover states
     */
    void UpdateButtonHover();

    /**
     * @brief Check for button clicks
     */
    void HandleButtonClick();

    /**
     * @brief Convert action string to MenuAction enum
     */
    static MenuAction StringToAction(const std::string& actionStr);

    /**
     * @brief Convert MenuAction enum to string
     */
    static std::string ActionToString(MenuAction action);
};

} // namespace GP2Engine
