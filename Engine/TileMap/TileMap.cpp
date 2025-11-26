/**
 * @file TileMap.cpp
 * @author Siti Syazwani (100%)
 * @brief Implementation of the TileMap class for map data management.(to be fully integrate in M3)
 */

#include "TileMap.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Engine.hpp> 
#include "Graphics/Renderer.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <filesystem>

namespace GP2Engine {

    const std::string TILEMAP_FILEPATH = "../../Sandbox/assets/textures/tilemap.txt";

    TileMap::TileMap() {
        //reload tiles (to get update with latest changes)
        ReloadTileDefinitions();

        //Load Map Data
        LoadMap(TILEMAP_FILEPATH);
    }

    bool TileMap::LoadTileDefinitionsFromJSON(const std::string& filepath) {
        m_TileDefinitions.clear();
        int tileID = 0;

        try {
            // Read the JSON file content
            std::ifstream file(filepath);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open scene file for tile definitions: " << filepath << std::endl;
                return false;
            }
            nlohmann::json jsonData = nlohmann::json::parse(file);

            // Check if the tile_definitions array exists
            if (!jsonData.contains("tile_definitions") || !jsonData["tile_definitions"].is_array()) {
                std::cerr << "Error: Scene file " << filepath << " is missing the 'tile_definitions' array." << std::endl;
                return false;
            }

            // Iterate over the JSON array to load each tile definition
            for (const auto& jDef : jsonData["tile_definitions"]) {
                TileDefinition def;
                def.tileID = tileID;

                // Use .value() for safe reading with default fallback
                def.name = jDef.value("name", "Unknown Tile");
                def.texturePath = jDef.value("texture_path", "");
                // The JSON value is boolean, which is safely read into a boolean
                def.isCollidable = jDef.value("is_collidable", false);

                // Load the texture dynamically
                if (!def.texturePath.empty()) {
                    def.texture = Texture::Create(def.texturePath);
                    if (!def.texture) {
                        std::cerr << "Failed to load tile texture: " << def.texturePath << std::endl;
                    }
                }

                m_TileDefinitions.push_back(std::move(def));
                tileID++;
            }

            std::cout << "Successfully loaded " << m_TileDefinitions.size() << " tile definitions from: " << filepath << std::endl;
            return true;

        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON Parsing Error in " << filepath << ": " << e.what() << std::endl;
            return false;
        }
        catch (const std::exception& e) {
            std::cerr << "General Error loading tile definitions: " << e.what() << std::endl;
            return false;
        }
    }

    void TileMap::LoadMap(const std::string& filepath) {

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open tilemap file: " << filepath << std::endl;
            // Fallback: Use a default size if file fails to open
            m_GridCols = 16;
            m_GridRows = 12;
            m_TilemapData.assign(m_GridCols * m_GridRows, 0);
            return;
        }

        if (!(file >> m_GridCols >> m_GridRows)) {
            std::cerr << "Error: Failed to read dimensions from tilemap file: " << filepath << std::endl;
            // Fallback: If dimensions cannot be read, use a default size (or assert/exit)
            m_GridCols = 16;
            m_GridRows = 12;
            m_TilemapData.assign(m_GridCols * m_GridRows, 0);
            return;
        }

        const int MAP_SIZE = m_GridCols * m_GridRows;
        m_TilemapData.clear();


        int tileValue; 
        while (file >> tileValue) {
            
            m_TilemapData.push_back(tileValue);
        }

        if (m_TilemapData.size() != MAP_SIZE) {
            std::cerr << "Warning: Tilemap size mismatch! Expected " << MAP_SIZE
                << ", got " << m_TilemapData.size() << std::endl;
            m_TilemapData.resize(MAP_SIZE, 0);
        }
       
    }

    int TileMap::GetTileValue(int col, int row) const {
        // Check bounds
        if (col < 0 || col >= m_GridCols || row < 0 || row >= m_GridRows) {
            return -1; // Out of bounds
        }

        // Calculate linear index
        int index = row * m_GridCols + col;

        // Check bounds for vector safety (should be covered by col/row check, but safer)
        if (index >= (int)m_TilemapData.size()) {
            return -1;
        }

        return m_TilemapData[index];
    }

    void TileMap::SetTileValue(int col, int row, int newValue) {
        // Sanity check bounds again
        if (col < 0 || col >= m_GridCols || row < 0 || row >= m_GridRows) {
            return;
        }

        int index = row * m_GridCols + col;

        if (index < m_TilemapData.size()) {
            m_TilemapData[index] = newValue;
        }
    }

    const TileDefinition* TileMap::GetTileDefinitionByID(int id) const {
        for (const auto& def : m_TileDefinitions) {
            if (def.tileID == id) {
                return &def;
            }
        }
        return nullptr;
    }

    void TileMap::SaveMap(const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for saving: " << filepath << std::endl;
            return;
        }

        file << m_GridCols << " " << m_GridRows << "\n";

        // Write the Tile Data
        for (int row = 0; row < m_GridRows; ++row) {
            for (int col = 0; col < m_GridCols; ++col) {
                // Calculate the linear index: index = row * m_GridCols + col;
                int index = row * m_GridCols + col;

                // Safety check
                if (index < (int)m_TilemapData.size()) {
                    // Write the tile value (0, 1, 2, etc.) followed by a space
                    file << m_TilemapData[index] << " ";
                }
            }
            // Write a newline character after each row for human-readability
            file << "\n";
        }

        // Close and Confirm
        file.close();
        std::cout << "Tilemap successfully saved to: " << filepath << std::endl;
    }

    void TileMap::ReloadTileDefinitions() {
        GP2Engine::ConfigLoader& config = GP2Engine::ConfigLoader::GetInstance();

        m_TileDefinitions.clear();

        //Loop and load definitions (Same logic as previously in your constructor)
        int tileID = 0;
        while (true) {
            std::string id_str = std::to_string(tileID);
            std::string nameKey = "tile_definition.tile_" + id_str + "_name";
            std::string name = config.GetString(nameKey, "");

            if (name.empty()) {
                break;
            }

            TileDefinition def;
            def.tileID = tileID;
            def.name = name;

            // Read texture path and collidable flag
            std::string pathKey = "tile_definition.tile_" + id_str + "_path";
            std::string collidableKey = "tile_definition.tile_" + id_str + "_collidable";

            def.texturePath = config.GetString(pathKey, "");
            def.isCollidable = config.GetString(collidableKey, "false") == "true";

            // Call Texture::Create, which re-reads the file from disk 
            // and re-uploads a NEW texture to the GPU.
            def.texture = Texture::Create(def.texturePath);

            if (def.texture == nullptr) {
                std::cerr << "TileMap: Failed to reload texture for: " << def.name << std::endl;
            }

            m_TileDefinitions.push_back(std::move(def));
            tileID++;
        }
    }
}
