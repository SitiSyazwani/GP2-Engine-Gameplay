/**
 * @file TileMap.hpp
 * @author Siti Syazwani (100%)
 * @brief Defines the TileMap class and related structures for managing the game's tile-based map data.(to be fully integrate in M3)
 */

#pragma once

#include <Engine.hpp>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "Graphics/Texture.hpp"

namespace GP2Engine {

    // EXTERN DECLARATION: The correct path constant, defined in TileMap.cpp.
    extern const std::string TILEMAP_FILEPATH;

    /**
     * @brief Defines the properties of a single tile type read from config.
     * * Used to map the integer ID in m_TilemapData to its texture, name, and collision property.
     */
    struct TileDefinition {
        int tileID;                         // The value stored in m_TilemapData
        std::string name;                   // tile name
        std::string texturePath;            // For configuration/debugging
        bool isCollidable = false;          // True if the tile blocks movement

        // The loaded texture pointer
        std::shared_ptr<GP2Engine::Texture> texture;

        /**
         * @brief Utility to get the texture ID for ImGui or Renderer.
         * @return The OpenGL texture ID, or 1 (the default white texture) if texture is null.
         */
        unsigned int GetTextureID() const {
            return texture ? texture->GetTextureID() : 1;
        }
    };

    class TileMap {
    public:

        TileMap();

        bool LoadTileDefinitionsFromJSON(const std::string& filepath);

        void LoadMap(const std::string& filepath);

        /**
         * @brief Gets the tile value (ID) at the given tile coordinates.
         */
        int GetTileValue(int col, int row) const;

        /**
         * @brief Sets the tile value (ID) at the given tile coordinates.
         */
        void SetTileValue(int col, int row, int newValue);

        void SaveMap(const std::string& filepath);
        void ReloadTileDefinitions();

        // --- Getters ---
        const std::vector<int>& getTileMapData() const {
            return m_TilemapData;
        }

        const std::vector<TileDefinition>& GetTileDefinitions() const {
            return m_TileDefinitions;
        }

        const TileDefinition* GetTileDefinitionByID(int id) const;

        int GetGridCols() const { return m_GridCols; }
        int GetGridRows() const { return m_GridRows; }
        int GetMapSize() const { return m_GridCols * m_GridRows; }
        unsigned int getDefaultTexture() const { return m_WhiteTextureID; }

    private:
        std::vector<TileDefinition> m_TileDefinitions;
        std::vector<int> m_TilemapData;

        int m_GridCols = 0;
        int m_GridRows = 0;

        unsigned int m_WhiteTextureID = 1; // Fallback texture ID
    };
}