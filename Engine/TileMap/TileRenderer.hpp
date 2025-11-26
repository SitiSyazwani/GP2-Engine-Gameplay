/**
 * @file TileRenderer.hpp
 * @author Siti Syazwani (100%)
 * @brief Defines the TileRenderer class for rendering and managing the game's tile-based map. (to be fully integrate in M3)
 */
#pragma once
#include <Engine.hpp>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "../Graphics/Camera.hpp"
#include "Graphics/Texture.hpp"

namespace GP2Engine {

    class TileMap;
    class Application;

    // Constants 
    const float TILE_PIXEL_WIDTH = 64.0f;
    const float TILE_PIXEL_HEIGHT = 64.0f;

    class TileRenderer {
    public:
        const TileMap* m_tileMap; 

        // **Constructor to accept the TileMap pointer**
        TileRenderer(const TileMap* tileMap) : m_tileMap(tileMap) {}

        // **Default constructor for when TileMap is not immediately available (though not ideal)**
        TileRenderer() : m_tileMap(nullptr) {}
        /**
         * @brief Renders the textured tilemap (floor/wall quads).
         *
         * Iterates over all tiles and draws the appropriate texture (floor or wall)
         * based on the tile's value.
         * @param renderer Reference to the main GP2Engine Renderer instance.
         */
        void Render(GP2Engine::Renderer& renderer);

        /**
         * @brief Renders the debug grid lines (separate from the textured tiles).
         *
         * Uses the DebugRenderer to draw lines outlining each tile for visualization
         * in the TileMap Editor.
         * @param renderer Reference to the main GP2Engine Renderer instance.
         */
        void RenderDebugGrid(GP2Engine::Renderer& renderer);

        /**
         * @brief Converts screen coordinates (e.g., mouse position) to map coordinates.
         * Now takes camera parameter to account for camera transformations.
         */
        GP2Engine::Vector2D ScreenToTileCoords(const GP2Engine::Vector2D& screenPos, const GP2Engine::Camera& camera) const;

        /**
         * @brief Overload - converts screen coordinates (e.g., mouse position) to map coordinates.
         *
         * Takes window coordinates and converts them to (column, row) indices.
         * @param screenPos The screen position (relative to the window top-left).
         * @return GP2Engine::Vector2D containing (col, row) of the tile, or (-1, -1) if out of bounds.
         */
        GP2Engine::Vector2D ScreenToTileCoords(const GP2Engine::Vector2D& screenPos) const {
            // Default to no camera transformation (for backward compatibility)
            return ScreenToTileCoordsNoCamera(screenPos);
        }

        GP2Engine::Vector2D ScreenToWorldCoords(
            const Vector2D& screenPos,
            const Camera& camera)
        {
            // Use stored window dimensions from InitializeDimensions()
            float windowWidth = m_windowWidth;
            float windowHeight = m_windowHeight;

            if (windowWidth <= 0.0f || windowHeight <= 0.0f)
                return Vector2D(0.0f, 0.0f);

            // Convert from screen (pixels) to normalized device coords [-1, 1]
            glm::vec2 ndc(
                (2.0f * screenPos.x) / windowWidth - 1.0f,
                1.0f - (2.0f * screenPos.y) / windowHeight
            );

            // Transform NDC to world coordinates
            glm::mat4 invViewProj = glm::inverse(camera.GetProjectionMatrix() * camera.GetViewMatrix());
            glm::vec4 worldPos = invViewProj * glm::vec4(ndc, 0.0f, 1.0f);

            return Vector2D(worldPos.x, worldPos.y);
        }

        /**
         * @brief Renders a selection box outline around a specific tile.
         *
         * Used by the DebugUI to highlight the currently hovered tile.
         * @param renderer Reference to the main GP2Engine Renderer instance.
         * @param col The column index of the tile to highlight.
         * @param row The row index of the tile to highlight.
         */
        void RenderSelectionBox(GP2Engine::Renderer& renderer, int col, int row);

        /**
         * @brief Must be called after configuration and TileMap loading to set up dimensions.
         * @param windowWidth The window width from the configuration file.
         * @param windowHeight The window height from the configuration file.
         */
        void InitializeDimensions();

        //Getters
        float GetMapPixelWidth() const;
        float GetMapPixelHeight() const;


    private:

        // --- Debug Colors ---

        /**
         * @brief Debug color used for wall quads if textures fail to load.
         */
        const GP2Engine::Color WALL_DEBUG_COLOR = GP2Engine::Color(0.2f, 0.2f, 0.2f, 1.0f);

        /**
         * @brief Debug color used for floor quads if textures fail to load.
         */
        const GP2Engine::Color FLOOR_DEBUG_COLOR = GP2Engine::Color(0.8f, 0.8f, 0.8f, 1.0f);

        float m_windowWidth = 0.0f;
        float m_windowHeight = 0.0f;

        const float CELL_WIDTH = TILE_PIXEL_WIDTH;
        const float CELL_HEIGHT = TILE_PIXEL_HEIGHT;

        /**
         * @brief Original screen-to-tile conversion without camera (for backward compatibility)
         */
        GP2Engine::Vector2D ScreenToTileCoordsNoCamera(const GP2Engine::Vector2D& screenPos) const;
    };
}