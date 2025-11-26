/**
 * @file EditorDebugRenderer.cpp
 * @author Adi (100%)
 * @brief Implementation of editor debug visualization
 *
 * Renders collision box overlays in the editor viewport to help with scene
 * layout and collision debugging. Uses the engine's DebugRenderer system.
 */

#include "EditorDebugRenderer.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;


constexpr int ENTITY_COUNT = 1000;
constexpr float TILE_SIZE = 64.0f;
constexpr float START_POS = 100.0f;
const std::string OUTPUT_FILE = "../../Sandbox/assets/scenes/stress_test_scene.json";

// Paths for the randomly distributed entities
const std::vector<std::string> ENTITY_TEXTURES = {
    "../../Sandbox/assets/textures/woodentile.png",
    "../../Sandbox/assets/textures/walltile.png",
    "../../Sandbox/assets/textures/SS_Monster_Vertical.png",
    "../../Sandbox/assets/textures/SS_Idle.png",
    "../../Sandbox/assets/textures/background.jpg",
    "../../Sandbox/assets/textures/ceramictile.png"
};

void EditorDebugRenderer::RenderCollisionBoxes(GP2Engine::Registry& registry) {
    // Get renderer and debug renderer from engine
    auto& renderer = GP2Engine::Renderer::GetInstance();
    auto& debugRenderer = renderer.GetDebugRenderer();

    // Begin debug rendering batch
    debugRenderer.Begin();

    // Iterate through all active entities in the scene
    for (GP2Engine::EntityID entity : registry.GetActiveEntities()) {
        // Get components needed for collision box calculation
        auto* transform = registry.GetComponent<GP2Engine::Transform2D>(entity);
        auto* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(entity);
        auto* tag = registry.GetComponent<GP2Engine::Tag>(entity);

        // Skip entities without transform or sprite
        if (!transform || !spriteComp) continue;

        // Skip background entities to reduce visual clutter
        if (tag && tag->name == "Background") continue;

        // Calculate hitbox size
        float baseSizeX = spriteComp->size.x * spriteComp->uvSize.x;  // Sprite width * UV width
        float baseSizeY = spriteComp->size.y * spriteComp->uvSize.y;  // Sprite height * UV height

        // Apply entity scale to get final collision box dimensions
        GP2Engine::Vector2D scaledSize(
            baseSizeX * transform->scale.x,
            baseSizeY * transform->scale.y
        );

        // Color: Green for named entities, Yellow for unnamed/untagged
        GP2Engine::Color boxColor = (tag && !tag->name.empty())
            ? GP2Engine::Color::GetGreen()
            : GP2Engine::Color::GetYellow();

        // Draw collision box outline around entity
        debugRenderer.DrawRectangle(
            transform->position,    // Position (center of entity)
            scaledSize,             // Size (width and height)
            boxColor,               // Color (green or yellow)
            false                   // Outline only
        );

        // Draw small white point at entity center
        debugRenderer.DrawPoint(transform->position, GP2Engine::Color::GetWhite(), 4.0f);
    }

    // Flush all debug rendering to screen
    debugRenderer.Flush(renderer);
}

void EditorDebugRenderer::GenerateStressTestScene() {
    std::cout << "--- Generating Stress Test Scene with " << ENTITY_COUNT << " Entities ---" << std::endl;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int grid_size = static_cast<int>(std::ceil(std::sqrt(ENTITY_COUNT)));

    json scene_data;
    json entities_data;

    // 1. Generate 998 Random Entities
    // Loop runs from 0 up to 997 (for 998 entities)
    for (int i = 0; i < ENTITY_COUNT - 2; ++i) {
        // Calculate position in a grid pattern
        int col = i % grid_size;
        int row = i / grid_size;

        float x_pos = START_POS + (col * TILE_SIZE * 1.5f);
        float y_pos = START_POS + (row * TILE_SIZE * 1.5f);

        // Randomly select one of the mixed textures
        int texture_index = std::rand() % ENTITY_TEXTURES.size();
        std::string texture_path = ENTITY_TEXTURES[texture_index];

        // Build the entity components
        json entity;
        entity["Transform2D"] = {
            {"x", x_pos},
            {"y", y_pos},
            {"rotation", 0.0f},
            {"scale_x", 1.0f},
            {"scale_y", 1.0f}
        };

        entity["SpriteComponent"] = {
            {"render_layer", 1},
            {"visible", true},
            {"width", TILE_SIZE},
            {"height", TILE_SIZE},
            {"color_r", 1.0f},
            {"color_g", 1.0f},
            {"color_b", 1.0f},
            {"color_a", 1.0f},
            {"uv_offset_x", 0.0f},
            {"uv_offset_y", 0.0f},
            {"uv_size_x", 1.0f},
            {"uv_size_y", 1.0f},
            {"sprite_texture_path", texture_path},
            {"direct_texture_path", ""}
        };

        entity["Tag"] = {
            {"name", "Random Entity " + std::to_string(i)},
            {"group", "stress_test"}
        };

        entities_data["entity_" + std::to_string(i)] = entity;
    }

    // --- 2. Create Special Entity: player ---
    // This will be entity_998
    {
        float x_pos = 100.0f; // Place character near origin
        float y_pos = 100.0f;
        json character_entity;

        character_entity["Transform2D"] = {
            {"x", x_pos},
            {"y", y_pos},
            {"rotation", 0.0f},
            {"scale_x", 1.0f},
            {"scale_y", 1.0f}
        };
        character_entity["SpriteComponent"] = {
            {"render_layer", 10}, // Higher layer so it's visible over the grid
            {"visible", true},
            {"width", 64.0f},
            {"height", 64.0f},
            {"color_r", 1.0f},
            {"color_g", 1.0f},
            {"color_b", 1.0f},
            {"color_a", 1.0f},
            {"uv_offset_x", 0.0f},
            {"uv_offset_y", 0.0f},
            {"uv_size_x", 1.0f},
            {"uv_size_y", 1.0f},
            {"sprite_texture_path", ENTITY_TEXTURES[3]}, // SS_Idle.png
            {"direct_texture_path", ""}
        };
        character_entity["Tag"] = {
            {"name", "Character"},
            {"group", "player"}
        };
        entities_data["entity_" + std::to_string(ENTITY_COUNT - 2)] = character_entity;
    }

    // --- 3. Create Special Entity: Monster ---
    // This will be entity_999
    {
        float x_pos = 500.0f; // Place monster near character
        float y_pos = 100.0f;
        json monster_entity;

        monster_entity["Transform2D"] = {
            {"x", x_pos},
            {"y", y_pos},
            {"rotation", 0.0f},
            {"scale_x", 1.0f},
            {"scale_y", 1.0f}
        };
        monster_entity["SpriteComponent"] = {
            {"render_layer", 10},
            {"visible", true},
            {"width", 45.0f},
            {"height", 90.0f},
            {"color_r", 1.0f},
            {"color_g", 1.0f},
            {"color_b", 1.0f},
            {"color_a", 1.0f},
            {"uv_offset_x", 0.0f},
            {"uv_offset_y", 0.0f},
            {"uv_size_x", 1.0f},
            {"uv_size_y", 1.0f},
            {"sprite_texture_path", ENTITY_TEXTURES[2]}, 
            {"direct_texture_path", ""}
        };
        monster_entity["Tag"] = {
            {"name", "Monster1"},
            {"group", "monsters"}
        };
        entities_data["entity_" + std::to_string(ENTITY_COUNT - 1)] = monster_entity;
    }


    // 4. Build the final JSON structure

    // Tile Definitions
    json tile_definitions = json::array({
        {
          {"name", "Floor"},
          {"texture_path", "../../Sandbox/assets/textures/ceramictile.png"},
          {"is_collidable", false}
        },
        {
          {"name", "Wall"},
          {"texture_path", "../../Sandbox/assets/textures/walltile.png"},
          {"is_collidable", true}
        },
        {
          {"name", "Wood"},
          {"texture_path", "../../Sandbox/assets/textures/woodentile.png"},
          {"is_collidable", false}
        }
        });

    json root;
    root["tile_definitions"] = tile_definitions;
    root["scene"] = {
        {"entity_count", ENTITY_COUNT},
        {"entities", entities_data}
    };

    // 5. Write to file
    try {
        std::ofstream file(OUTPUT_FILE);
        if (file.is_open()) {
            file << std::setw(2) << root << std::endl;
            file.close();
            std::cout << "Generated scene file with 1000 entities: " << OUTPUT_FILE << std::endl;
        }
        else {
            std::cerr << "ERROR: Could not open output file: " << OUTPUT_FILE << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "JSON Generation Error: " << e.what() << std::endl;
    }
}
