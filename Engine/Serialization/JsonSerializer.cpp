/**
 * @file JsonSerializer.cpp
 * @author Rifqah (80%), Adi (20%)
 * @brief Saves and loads game data to/from JSON files
 *
 * Implementation of the class handles reading and writing game information like player stats,
 * level data, and object positions using JSON format files.
 * It can save game progress or load previously saved games.
 * Also handles ECS scene serialization (entities and components).
 */

#include "JsonSerializer.hpp"
#include "../Core/Logger.hpp"
#include "../ECS/Registry.hpp"
#include "../ECS/Component.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/Sprite.hpp"
#include "../Graphics/Font.hpp"
#include "../Resources/ResourceManager.hpp"

namespace GP2Engine {

    JsonSerializer::JsonSerializer() : m_isValid(false), m_currentContext(nullptr) {}
    JsonSerializer::~JsonSerializer() {}

    bool JsonSerializer::LoadFromFile(const std::string& filename) {
        std::cout << "Loading JSON from: " << filename << std::endl;

        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cout << "FAILED to open JSON file: " << filename << std::endl;
                m_isValid = false;
                return false;
            }

            m_jsonData = nlohmann::json::parse(file);
            m_currentContext = &m_jsonData;
            m_isValid = true;

            std::cout << "JSON file loaded and parsed successfully!" << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR(std::string("Error parsing JSON: ") + e.what());
            m_isValid = false;
            return false;
        }
    }

    bool JsonSerializer::BeginObject(const std::string& name) {
        if (!m_isValid || !m_currentContext) return false;

        if (name.empty()) {
            m_contextStack.push(m_currentContext);
            return true;
        }

        // Navigate into nested object
        if (m_currentContext->contains(name) && (*m_currentContext)[name].is_object()) {
            m_contextStack.push(m_currentContext);
            m_currentContext = &((*m_currentContext)[name]);
            std::cout << "Begin object: " << name << std::endl;
            return true;
        }

        return false;
    }

    void JsonSerializer::EndObject() {
        if (!m_contextStack.empty()) {
            m_currentContext = m_contextStack.top(); // Pop back to parent
            m_contextStack.pop();
            std::cout << "End object" << std::endl;
        }
    }

    bool JsonSerializer::HasObject(const std::string& name) const {
        if (!m_isValid || !m_currentContext) return false;
        return m_currentContext->contains(name) && (*m_currentContext)[name].is_object();
    }

    // Serialization methods - read data from JSON into variables
    void JsonSerializer::Serialize(int& value, const std::string& name) {
        if (!m_isValid || !m_currentContext || name.empty() || !m_currentContext->contains(name)) return;
        try { value = (*m_currentContext)[name].get<int>(); std::cout << "Read int: " << name << " = " << value << std::endl; }
        catch (const std::exception& e) { LOG_ERROR(std::string("Error reading int '") + name + "': " + e.what()); }
    }

    void JsonSerializer::Serialize(float& value, const std::string& name) {
        if (!m_isValid || !m_currentContext || name.empty() || !m_currentContext->contains(name)) return;
        try { value = (*m_currentContext)[name].get<float>(); std::cout << "Read float: " << name << " = " << value << std::endl; }
        catch (const std::exception& e) { LOG_ERROR(std::string("Error reading float '") + name + "': " + e.what()); }
    }

    void JsonSerializer::Serialize(bool& value, const std::string& name) {
        if (!m_isValid || !m_currentContext || name.empty() || !m_currentContext->contains(name)) return;
        try { value = (*m_currentContext)[name].get<bool>(); std::cout << "Read bool: " << name << " = " << (value ? "true" : "false") << std::endl; }
        catch (const std::exception& e) { LOG_ERROR(std::string("Error reading bool '") + name + "': " + e.what()); }
    }

    void JsonSerializer::Serialize(std::string& value, const std::string& name) {
        if (!m_isValid || !m_currentContext || name.empty() || !m_currentContext->contains(name)) return;
        try { value = (*m_currentContext)[name].get<std::string>(); std::cout << "Read string: " << name << " = " << value << std::endl; }
        catch (const std::exception& e) { LOG_ERROR(std::string("Error reading string '") + name + "': " + e.what()); }
    }

    // Scene serialization methods
    bool JsonSerializer::SaveScene(Registry& registry, const std::string& filename) {
        try {
            // Build JSON directly using nlohmann::json
            nlohmann::json sceneJson;

            // Save entity count
            sceneJson["scene"]["entity_count"] = static_cast<int>(registry.GetEntityCount());

            // Save each active entity
            nlohmann::json& entitiesJson = sceneJson["scene"]["entities"];
            int entityIndex = 0;
            for (EntityID entity : registry.GetActiveEntities()) {
                std::string entityKey = "entity_" + std::to_string(entityIndex++);
                nlohmann::json& entityJson = entitiesJson[entityKey];

                // Save Transform2D if present
                if (Transform2D* transform = registry.GetComponent<Transform2D>(entity)) {
                    entityJson["Transform2D"]["x"] = transform->position.x;
                    entityJson["Transform2D"]["y"] = transform->position.y;
                    entityJson["Transform2D"]["rotation"] = transform->rotation;
                    entityJson["Transform2D"]["scale_x"] = transform->scale.x;
                    entityJson["Transform2D"]["scale_y"] = transform->scale.y;
                }

                // Save SpriteComponent if present
                if (SpriteComponent* spriteComp = registry.GetComponent<SpriteComponent>(entity)) {
                    nlohmann::json& spriteJson = entityJson["SpriteComponent"];

                    // Basic properties
                    spriteJson["render_layer"] = spriteComp->renderLayer;
                    spriteJson["visible"] = spriteComp->visible;

                    // Size and color
                    spriteJson["width"] = spriteComp->size.x;
                    spriteJson["height"] = spriteComp->size.y;
                    spriteJson["color_r"] = spriteComp->color.r;
                    spriteJson["color_g"] = spriteComp->color.g;
                    spriteJson["color_b"] = spriteComp->color.b;
                    spriteJson["color_a"] = spriteComp->color.a;

                    // UV coordinates
                    spriteJson["uv_offset_x"] = spriteComp->uvOffset.x;
                    spriteJson["uv_offset_y"] = spriteComp->uvOffset.y;
                    spriteJson["uv_size_x"] = spriteComp->uvSize.x;
                    spriteJson["uv_size_y"] = spriteComp->uvSize.y;

                    // Texture path (if textured)
                    if (spriteComp->sprite && spriteComp->sprite->GetTexture()) {
                        spriteJson["sprite_texture_path"] = spriteComp->sprite->GetTexture()->GetFilePath();
                    } else {
                        spriteJson["sprite_texture_path"] = "";
                    }
                }

                // Save TextComponent if present
                if (TextComponent* textComp = registry.GetComponent<TextComponent>(entity)) {
                    nlohmann::json& textJson = entityJson["TextComponent"];

                    // Text content
                    textJson["text"] = textComp->text;

                    // Font path (if font is valid)
                    std::string fontPath = "";
                    unsigned int fontSize = 48;
                    if (textComp->font && textComp->font->IsValid()) {
                        fontPath = textComp->font->GetFontPath();
                        fontSize = textComp->font->GetFontSize();
                    }
                    textJson["font_path"] = fontPath;
                    textJson["font_size"] = static_cast<int>(fontSize);

                    // Color
                    textJson["color_r"] = textComp->color.r;
                    textJson["color_g"] = textComp->color.g;
                    textJson["color_b"] = textComp->color.b;
                    textJson["color_a"] = textComp->color.a;

                    // Scale and rendering properties
                    textJson["scale"] = textComp->scale;
                    textJson["visible"] = textComp->visible;
                    textJson["render_layer"] = textComp->renderLayer;

                    // Offset
                    textJson["offset_x"] = textComp->offset.x;
                    textJson["offset_y"] = textComp->offset.y;
                }

                // Save Tag if present
                if (Tag* tag = registry.GetComponent<Tag>(entity)) {
                    entityJson["Tag"]["name"] = tag->name;
                    entityJson["Tag"]["group"] = tag->group;
                }
            }

            // Write JSON to file
            std::ofstream outFile(filename);
            if (!outFile.is_open()) {
                std::cerr << "ERROR: Failed to open file for writing: " << filename << std::endl;
                return false;
            }

            // Write formatted JSON with 2-space indentation
            outFile << sceneJson.dump(2) << std::endl;
            outFile.close();

            std::cout << "Scene saved successfully to: " << filename << std::endl;
            return true;

        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Error saving scene: ") + e.what());
            return false;
        }
    }

    bool JsonSerializer::LoadScene(Registry& registry, const std::string& filename) {
        try {
            JsonSerializer serializer;

            // Load the JSON file
            if (!serializer.LoadFromFile(filename)) {
                std::cerr << "Failed to load scene file: " << filename << std::endl;
                return false;
            }

            // Clear existing entities to ensure clean loading
            // Create a copy since GetActiveEntities() will change during destruction
            std::vector<EntityID> entitiesToDestroy;
            for (EntityID entity : registry.GetActiveEntities()) {
                entitiesToDestroy.push_back(entity);
            }
            for (EntityID entity : entitiesToDestroy) {
                registry.DestroyEntity(entity);
            }

            // Reset entity ID counter to ensure deterministic entity ID assignment
            registry.ResetEntityIDs();

            // Clear all component storage to prevent data leakage from previous scenes
            registry.ClearAllComponents();

            // Load the scene
            if (!serializer.BeginObject("scene")) {
                std::cerr << "Invalid scene file format" << std::endl;
                return false;
            }

            // Read entity count
            int entityCount = 0;
            serializer.Serialize(entityCount, "entity_count");

            // Load entities
            if (serializer.BeginObject("entities")) {
                for (int i = 0; i < entityCount; ++i) {
                    std::string entityKey = "entity_" + std::to_string(i);
                    if (serializer.BeginObject(entityKey)) {

                        // Create new entity
                        EntityID entity = registry.CreateEntity();

                        // Load Transform2D if present
                        if (serializer.BeginObject("Transform2D")) {
                            float x, y, rotation, scaleX, scaleY;
                            serializer.Serialize(x, "x");
                            serializer.Serialize(y, "y");
                            serializer.Serialize(rotation, "rotation");
                            serializer.Serialize(scaleX, "scale_x");
                            serializer.Serialize(scaleY, "scale_y");

                            Transform2D transform;
                            transform.position = Vector2D(x, y);
                            transform.rotation = rotation;
                            transform.scale = Vector2D(scaleX, scaleY);

                            registry.AddComponent<Transform2D>(entity, transform);
                            serializer.EndObject();
                        }


                        // Load SpriteComponent if present
                        if (serializer.BeginObject("SpriteComponent")) {
                            int layer;
                            bool visible;
                            float width, height, r, g, b, a;
                            float uvOffsetX, uvOffsetY, uvSizeX, uvSizeY;
                            std::string spriteTexturePath;

                            // Load basic properties
                            serializer.Serialize(layer, "render_layer");
                            serializer.Serialize(visible, "visible");
                            serializer.Serialize(width, "width");
                            serializer.Serialize(height, "height");
                            serializer.Serialize(r, "color_r");
                            serializer.Serialize(g, "color_g");
                            serializer.Serialize(b, "color_b");
                            serializer.Serialize(a, "color_a");

                            // Load UV coordinates
                            serializer.Serialize(uvOffsetX, "uv_offset_x");
                            serializer.Serialize(uvOffsetY, "uv_offset_y");
                            serializer.Serialize(uvSizeX, "uv_size_x");
                            serializer.Serialize(uvSizeY, "uv_size_y");

                            // Load texture path
                            serializer.Serialize(spriteTexturePath, "sprite_texture_path");

                            // Create SpriteComponent
                            SpriteComponent spriteComp;
                            spriteComp.renderLayer = layer;
                            spriteComp.visible = visible;
                            spriteComp.size = Vector2D(width, height);
                            spriteComp.color = glm::vec4(r, g, b, a);
                            spriteComp.uvOffset = Vector2D(uvOffsetX, uvOffsetY);
                            spriteComp.uvSize = Vector2D(uvSizeX, uvSizeY);

                            // Load texture if path is provided
                            if (!spriteTexturePath.empty()) {
                                // Use ResourceManager for texture loading
                                auto& resMgr = ResourceManager::GetInstance();
                                auto texture = resMgr.LoadTexture(spriteTexturePath);

                                if (texture && texture->IsValid()) {
                                    spriteComp.sprite = std::make_shared<Sprite>(texture);
                                    spriteComp.sprite->SetSize(Vector2D(width, height));
                                    spriteComp.sprite->SetColor(glm::vec4(r, g, b, a));
                                } else {
                                    std::cerr << "Warning: Failed to load texture from ResourceManager: " << spriteTexturePath << std::endl;
                                    // Create sprite anyway (without texture) so it can be set later
                                    spriteComp.sprite = std::make_shared<Sprite>(nullptr);
                                    spriteComp.sprite->SetSize(Vector2D(width, height));
                                    spriteComp.sprite->SetColor(glm::vec4(r, g, b, a));
                                }
                            }
                            // If no texture path, it's a colored quad (default)

                            registry.AddComponent<SpriteComponent>(entity, spriteComp);
                            serializer.EndObject();
                        }


                        // Load TextComponent if present
                        if (serializer.BeginObject("TextComponent")) {
                            std::string text, fontPath;
                            int fontSize;
                            float r, g, b, a;
                            float scale;
                            bool visible;
                            int layer;
                            float offsetX, offsetY;

                            // Load text and font
                            serializer.Serialize(text, "text");
                            serializer.Serialize(fontPath, "font_path");
                            serializer.Serialize(fontSize, "font_size");

                            // Load color
                            serializer.Serialize(r, "color_r");
                            serializer.Serialize(g, "color_g");
                            serializer.Serialize(b, "color_b");
                            serializer.Serialize(a, "color_a");

                            // Load scale and rendering properties
                            serializer.Serialize(scale, "scale");
                            serializer.Serialize(visible, "visible");
                            serializer.Serialize(layer, "render_layer");

                            // Load offset
                            serializer.Serialize(offsetX, "offset_x");
                            serializer.Serialize(offsetY, "offset_y");

                            // Create TextComponent
                            TextComponent textComp;
                            textComp.text = text;
                            textComp.color = glm::vec4(r, g, b, a);
                            textComp.scale = scale;
                            textComp.visible = visible;
                            textComp.renderLayer = layer;
                            textComp.offset = Vector2D(offsetX, offsetY);

                            // Load font if path is provided
                            if (!fontPath.empty()) {
                                // Use ResourceManager for font loading
                                auto& resMgr = ResourceManager::GetInstance();
                                textComp.font = resMgr.LoadFont(fontPath, static_cast<unsigned int>(fontSize));

                                if (!textComp.font || !textComp.font->IsValid()) {
                                    std::cerr << "Warning: Failed to load font from ResourceManager: " << fontPath << std::endl;
                                }
                            }

                            registry.AddComponent<TextComponent>(entity, textComp);
                            serializer.EndObject();
                        }

                        // Load Tag if present
                        if (serializer.BeginObject("Tag")) {
                            std::string name, group;
                            serializer.Serialize(name, "name");
                            serializer.Serialize(group, "group");

                            Tag tag;
                            tag.name = name;
                            tag.group = group;

                            registry.AddComponent<Tag>(entity, tag);
                            serializer.EndObject();
                        }

                        serializer.EndObject(); // End entity
                    }
                }
                serializer.EndObject(); // End entities
            }

            serializer.EndObject(); // End scene

            std::cout << "Scene loaded from: " << filename << std::endl;
            return true;

        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Error loading scene: ") + e.what());
            return false;
        }
    }

} // namespace GP2Engine

