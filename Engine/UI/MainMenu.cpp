/**
 * @file MainMenu.cpp
 * @brief Implementation of the main menu system with Event System integration
 */

#include <UI/MainMenu.h>
#include <Core/Input.hpp>
#include <Core/EventSystem.hpp>
#include <Core/Events.hpp>
#include <Graphics/Renderer.hpp>
#include <Resources/ResourceManager.hpp>
#include <iostream>
#include <fstream>

namespace GP2Engine {

MainMenu::MainMenu() {
}

bool MainMenu::Initialize(int windowWidth, int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    // Set default config if not already set
    if (m_config.fontSize <= 0) m_config.fontSize = 48.0f;
    if (m_config.buttonSpacing <= 0) m_config.buttonSpacing = 60.0f;
    if (m_config.backgroundTexturePath.empty()) m_config.backgroundTexturePath = "textures/Menus.png";
    if (m_config.fontPath.empty()) m_config.fontPath = "fonts/test1.ttf";

    // Load background texture
    auto& resourceManager = GP2Engine::ResourceManager::GetInstance();
    m_backgroundTexture = resourceManager.LoadTexture(m_config.backgroundTexturePath);

    if (!m_backgroundTexture) {
        std::cerr << "Failed to load menu background texture!" << std::endl;
        return false;
    }

    // Load font
    unsigned int fontSize = static_cast<unsigned int>(m_config.fontSize);
    m_font = resourceManager.LoadFont(m_config.fontPath, fontSize);

    if (!m_font) {
        std::cerr << "Failed to load menu font!" << std::endl;
        return false;
    }

    // Load secondary font
    m_secondaryFont = resourceManager.LoadFont("fonts/test2.ttf", fontSize);
    if (!m_secondaryFont) {
        std::cerr << "Warning: Failed to load secondary font" << std::endl;
    }

    // Set up camera for UI rendering (orthographic projection)
    // Use TOP-LEFT coordinate system to match mouse input and JSON positions
    // Invert Y-axis: bottom = windowHeight, top = 0
    m_menuCamera.SetOrthographic(0.0f, static_cast<float>(m_windowWidth),
                                 static_cast<float>(m_windowHeight), 0.0f);
    m_menuCamera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    // Create buttons
    CreateButtons();

    return true;
}

void MainMenu::CreateButtons() {
    m_buttons.clear();

    if (!m_font) {
        return;
    }

    // Button data with positions matching your JSON file
    struct ButtonData {
        std::string text;
        MenuAction action;
        float x, y;
    };

    std::vector<ButtonData> buttonData = {
        {"START", MenuAction::StartGame, 250.0f, 230.0f},
        {"SETTINGS", MenuAction::OpenSettings, 250.0f, 160.0f},
        {"QUIT", MenuAction::QuitGame, 250.0f, 90.0f}
    };

    for (const auto& data : buttonData) {
        Button button;
        button.text = data.text;
        button.action = data.action;

        // Use the exact positions from JSON
        float textWidth = m_font->CalculateTextWidth(button.text, 1.2f); // 1.2f matches your scale
        float fontSize = m_config.fontSize > 0 ? m_config.fontSize : 48.0f;

        // Set the text position (from JSON)
        button.textPosition = glm::vec2(data.x, data.y);

        // Set hitbox size with some padding
        float paddingX = 20.0f;
        float paddingY = 10.0f;
        float textScale = 1.2f; // Match the scale used in Render()
        button.size.x = textWidth + paddingX * 2.0f;
        button.size.y = fontSize * textScale + paddingY * 2.0f;

        // Position the hitbox around the text (TOP-LEFT coordinate system)
        // Text position is at the baseline, so adjust upward for the hitbox top
        button.position.x = data.x - paddingX;
        button.position.y = data.y - fontSize * textScale + paddingY;
        
        m_buttons.push_back(button);
    }
}

void MainMenu::Update(float /*deltaTime*/) {
    // Get mouse position (already in TOP-LEFT coords: y=0 at top)
    double mouseX, mouseY;
    GP2Engine::Input::GetMousePosition(mouseX, mouseY);

    // Keep in TOP-LEFT coordinates to match camera and button positions
    m_mousePosition.x = static_cast<float>(mouseX);
    m_mousePosition.y = static_cast<float>(mouseY);

    // Update button hover states
    UpdateButtonHover();

    // Check for clicks
    HandleButtonClick();
}

void MainMenu::UpdateButtonHover() {
    for (auto& button : m_buttons) {
        bool wasHovered = button.isHovered;
        bool isHovering = button.Contains(m_mousePosition.x, m_mousePosition.y);

        button.isHovered = isHovering;

        if (button.isHovered && !wasHovered) {
            GP2Engine::MenuButtonHoverEvent hoverEvent;
            hoverEvent.buttonName = button.text;
            GP2Engine::EventSystem::Publish(hoverEvent);
        }
    }
}

void MainMenu::HandleButtonClick() {
    bool mousePressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);

    // Detect click (button press, not hold)
    if (mousePressed && !m_mouseWasPressed) {
        // Check which button was clicked
        for (const auto& button : m_buttons) {
            if (button.isHovered) {
                m_triggeredAction = button.action;
                // Fire appropriate event based on button action
                switch (button.action) {
                case MenuAction::StartGame:
                    GP2Engine::EventSystem::Publish(GP2Engine::MenuStartGameEvent{});
                    break;

                case MenuAction::OpenSettings:
                    GP2Engine::EventSystem::Publish(GP2Engine::MenuOpenSettingsEvent{});
                    break;

                case MenuAction::QuitGame:
                    GP2Engine::EventSystem::Publish(GP2Engine::MenuQuitGameEvent{});
                    break;

                default:
                    break;
                }

                break;
            }
        }
    }

    m_mouseWasPressed = mousePressed;
}

void MainMenu::Render() {
    auto& renderer = GP2Engine::Renderer::GetInstance();

    // Set up camera for UI rendering
    renderer.SetCamera(m_menuCamera);

    // Render background texture (fullscreen)
    if (m_backgroundTexture) {
        // Position is the CENTER of the quad (not top-left), so center it on screen
        glm::vec2 position(m_windowWidth / 2.0f, m_windowHeight / 2.0f);
        glm::vec2 size(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight));
        renderer.DrawTexturedQuad(position, size, m_backgroundTexture->GetTextureID());
    }

    // Render buttons
    for (const auto& button : m_buttons) {
        // Set text color based on hover state
        glm::vec4 textColor = button.isHovered ? 
            glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) :  // Yellow when hovered
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);   // White when not hovered

        // Render button text at the exact position from JSON
        renderer.DrawText(
            m_font.get(),
            button.text,
            button.textPosition,  // Use the exact position from JSON
            1.2f,                // Scale to match your original setup
            textColor
        );
    }
}

void MainMenu::OnWindowResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;

    // Update camera projection for new window size (TOP-LEFT coordinate system)
    // Invert Y-axis: bottom = windowHeight, top = 0
    m_menuCamera.SetOrthographic(0.0f, static_cast<float>(m_windowWidth),
                                 static_cast<float>(m_windowHeight), 0.0f);

    // Recreate buttons with new positions
    CreateButtons();
}

// ============================================================================
// JSON Configuration Methods
// ============================================================================

bool MainMenu::InitializeFromJson(const std::string& configPath, int windowWidth, int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    // Load configuration from JSON
    if (!LoadFromJson(configPath)) {
        std::cerr << "Failed to load menu configuration from: " << configPath << std::endl;
        return false;
    }

    // Setup camera (TOP-LEFT coordinate system)
    // Invert Y-axis: bottom = windowHeight, top = 0
    m_menuCamera.SetOrthographic(0.0f, static_cast<float>(m_windowWidth),
                                static_cast<float>(m_windowHeight), 0.0f);

    // Load resources from config
    auto& resMgr = ResourceManager::GetInstance();

    if (!m_config.backgroundTexturePath.empty()) {
        m_backgroundTexture = resMgr.LoadTexture(m_config.backgroundTexturePath);
        if (!m_backgroundTexture || !m_backgroundTexture->IsValid()) {
            std::cerr << "Failed to load background texture: " << m_config.backgroundTexturePath << std::endl;
            return false;
        }
    }

    if (!m_config.fontPath.empty()) {
        unsigned int fontSize = static_cast<unsigned int>(m_config.fontSize > 0 ? m_config.fontSize : 48.0f);
        m_font = resMgr.LoadFont(m_config.fontPath, fontSize);
        if (!m_font) {
            std::cerr << "Failed to load font: " << m_config.fontPath << std::endl;
            return false;
        }
    }

    // Create buttons from config
    CreateButtonsFromConfig();

    return true;
}

bool MainMenu::LoadFromJson(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << configPath << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        // Parse configuration
        m_config.backgroundTexturePath = j.value("backgroundTexture", "");
        m_config.fontPath = j.value("font", "");
        m_config.fontSize = j.value("fontSize", 48.0f);
        m_config.buttonSpacing = j.value("buttonSpacing", 60.0f);

        // Parse buttons
        m_config.buttons.clear();
        if (j.contains("buttons") && j["buttons"].is_array()) {
            for (const auto& btnJson : j["buttons"]) {
                Config::ButtonConfig btn;
                btn.text = btnJson.value("text", "");
                btn.action = btnJson.value("action", "None");
                btn.posX = btnJson.value("posX", 0.0f);
                btn.posY = btnJson.value("posY", 0.0f);
                btn.width = btnJson.value("width", 200.0f);
                btn.height = btnJson.value("height", 60.0f);
                btn.useNormalizedPosition = btnJson.value("useNormalizedPosition", true);

                m_config.buttons.push_back(btn);
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON config: " << e.what() << std::endl;
        return false;
    }
}

bool MainMenu::SaveToJson(const std::string& configPath) const {
    try {
        nlohmann::json j;

        // Serialize configuration
        j["backgroundTexture"] = m_config.backgroundTexturePath;
        j["font"] = m_config.fontPath;
        j["fontSize"] = m_config.fontSize;
        j["buttonSpacing"] = m_config.buttonSpacing;

        // Serialize buttons
        j["buttons"] = nlohmann::json::array();
        for (const auto& btn : m_config.buttons) {
            nlohmann::json btnJson;
            btnJson["text"] = btn.text;
            btnJson["action"] = btn.action;
            btnJson["posX"] = btn.posX;
            btnJson["posY"] = btn.posY;
            btnJson["width"] = btn.width;
            btnJson["height"] = btn.height;
            btnJson["useNormalizedPosition"] = btn.useNormalizedPosition;

            j["buttons"].push_back(btnJson);
        }

        // Write to file
        std::ofstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << configPath << std::endl;
            return false;
        }

        file << j.dump(4);  // Pretty print with 4 space indent
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving JSON config: " << e.what() << std::endl;
        return false;
    }
}

void MainMenu::SetConfig(const Config& config) {
    m_config = config;

    // Reload resources
    auto& resMgr = ResourceManager::GetInstance();

    if (!m_config.backgroundTexturePath.empty()) {
        m_backgroundTexture = resMgr.LoadTexture(m_config.backgroundTexturePath);
    }

    if (!m_config.fontPath.empty()) {
        unsigned int fontSize = static_cast<unsigned int>(m_config.fontSize > 0 ? m_config.fontSize : 48.0f);
        m_font = resMgr.LoadFont(m_config.fontPath, fontSize);
    }

    // Recreate buttons
    CreateButtonsFromConfig();
}

void MainMenu::CreateButtonsFromConfig() {
    m_buttons.clear();

    for (const auto& btnConfig : m_config.buttons) {
        Button btn;
        btn.text = btnConfig.text;
        btn.action = StringToAction(btnConfig.action);

        // Calculate position
        if (btnConfig.useNormalizedPosition) {
            // Normalized position (0-1 range)
            btn.position.x = btnConfig.posX * m_windowWidth;
            btn.position.y = btnConfig.posY * m_windowHeight;
        } else {
            // Absolute position
            btn.position.x = btnConfig.posX;
            btn.position.y = btnConfig.posY;
        }

        btn.size.x = btnConfig.width;
        btn.size.y = btnConfig.height;

        m_buttons.push_back(btn);
    }
}

MainMenu::MenuAction MainMenu::StringToAction(const std::string& actionStr) {
    if (actionStr == "StartGame") return MenuAction::StartGame;
    if (actionStr == "OpenSettings") return MenuAction::OpenSettings;
    if (actionStr == "QuitGame") return MenuAction::QuitGame;
    return MenuAction::None;
}

std::string MainMenu::ActionToString(MenuAction action) {
    switch (action) {
        case MenuAction::StartGame: return "StartGame";
        case MenuAction::OpenSettings: return "OpenSettings";
        case MenuAction::QuitGame: return "QuitGame";
        default: return "None";
    }
}

void MainMenu::CreateButtonsFromScene(GP2Engine::Registry& registry) {
    m_buttons.clear();

    // Find button entities in the scene and create hitboxes for them
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        auto* tag = registry.GetComponent<GP2Engine::Tag>(entity);
        auto* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
        auto* textComp = registry.GetComponent<GP2Engine::TextComponent>(entity);

        if (tag && transform && textComp && textComp->font &&
            (tag->name == "StartButton" || tag->name == "QuitButton" || tag->name == "SettingsButton")) {

            Button button;
            button.text = textComp->text;

            // Determine action based on tag name
            if (tag->name == "StartButton") {
                button.action = MenuAction::StartGame;
            } else if (tag->name == "QuitButton") {
                button.action = MenuAction::QuitGame;
            } else if (tag->name == "SettingsButton") {
                button.action = MenuAction::OpenSettings;
            }

            // Calculate actual text dimensions with proper font metrics
            float textWidth = textComp->font->CalculateTextWidth(textComp->text, textComp->scale);
            float fontSize = static_cast<float>(textComp->font->GetFontSize()) * textComp->scale;

            // Add padding for click area
            float paddingX = 20.0f;
            float paddingY = 10.0f;

            // Set hitbox size
            button.size.x = textWidth + paddingX * 2.0f;
            button.size.y = fontSize + paddingY * 2.0f;

            // Position the hitbox around the text (TOP-LEFT coordinate system)
            // transform->position is at the text baseline, need to offset up by font height
            button.position.x = transform->position.x - paddingX;
            button.position.y = transform->position.y - fontSize * 0.8f - paddingY;

            // Store text position for rendering (if we were rendering from MainMenu)
            button.textPosition = glm::vec2(transform->position.x, transform->position.y);

            m_buttons.push_back(button);
        }
    }

    std::cout << "Created " << m_buttons.size() << " menu buttons from scene entities" << std::endl;
}

} // namespace GP2Engine
