///**
// * @file TileRenderer.cpp
// * @author Siti Syazwani (100%)
// * @brief Defines the TileRenderer class for rendering and managing the game's tile-based map.(to be fully integrate in M3)
// */
#include "TileRenderer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Engine.hpp> 
#include "../Graphics/Renderer.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include "TileMap.hpp"

namespace GP2Engine {

    void TileRenderer::Render(Renderer& renderer) {

        if (!m_tileMap) {
            std::cout << "TileRenderer: No tilemap available!" << std::endl;
            return;
        }

        const std::vector<int>& tileMapData = m_tileMap->getTileMapData();

        // DEBUG: Print tilemap info
        //std::cout << "TileRenderer: Rendering " << tileMapData.size() << " tiles ("
        //    << m_tileMap->GetGridCols() << "x" << m_tileMap->GetGridRows() << ")" << std::endl;

        // Normalized UV coordinates for the full texture (0, 0, 1, 1). 
        const Color fullUVCoords = Color(0.0f, 0.0f, 1.0f, 1.0f);

        const float HALF_CELL_WIDTH = CELL_WIDTH * 0.5f;
        const float HALF_CELL_HEIGHT = CELL_HEIGHT * 0.5f;
        const Vector2D size = Vector2D(CELL_WIDTH, CELL_HEIGHT);

        // Convert static size and UVs to glm types once
        const glm::vec2 glm_size = glm::vec2(size.x, size.y);
        const glm::vec4 glm_fullUVCoords = glm::vec4(fullUVCoords.r, fullUVCoords.g, fullUVCoords.b, fullUVCoords.a);
        const glm::vec4 glm_white = glm::vec4(Color::GetWhite().r, Color::GetWhite().g, Color::GetWhite().b, Color::GetWhite().a);

        int renderedTiles = 0;
        int fallbackTiles = 0;

        for (int row = 0; row < m_tileMap->GetGridRows(); ++row) {
            for (int col = 0; col < m_tileMap->GetGridCols(); ++col) {
                int index = row * m_tileMap->GetGridCols() + col;
                if (index >= (int)tileMapData.size()) break;

                // Calculate position (center of the cell)
                Vector2D position = Vector2D(
                    (col * CELL_WIDTH) + HALF_CELL_WIDTH,
                    (row * CELL_HEIGHT) + HALF_CELL_HEIGHT
                );

                int tileID = tileMapData[index];
                TexturePtr texture = nullptr;

                // Determine texture based on tile value
                const TileDefinition* def = m_tileMap->GetTileDefinitionByID(tileID);
                if (def && def->texture) {
                    texture = def->texture;
                }

                // Convert dynamic position to glm::vec2
                const glm::vec2 glm_position = glm::vec2(position.x, position.y);

                if (texture != nullptr) {
                    // Texture loaded successfully
                    unsigned int textureID = texture->GetTextureID();

                    // Draw the textured tile 
                    renderer.DrawTexturedQuad(
                        glm_position,
                        glm_size,
                        textureID,
                        glm_fullUVCoords,
                        glm_white
                    );
                    renderedTiles++;
                }
                else {
                    // FALLBACK - Texture failed to load, draw a colored quad
                    Color fallbackColor = (tileID == 1) ? WALL_DEBUG_COLOR : FLOOR_DEBUG_COLOR;

                    // Convert fallback color to glm::vec4
                    const glm::vec4 glm_fallbackColor = glm::vec4(fallbackColor.r, fallbackColor.g, fallbackColor.b, fallbackColor.a);

                    unsigned int defaultTex = m_tileMap->getDefaultTexture();

                    // Use DrawTexturedQuad with the White Texture ID and the fallback color tint
                    renderer.DrawTexturedQuad(
                        glm_position,
                        glm_size,
                        defaultTex,
                        glm_fullUVCoords,
                        glm_fallbackColor
                    );
                    fallbackTiles++;
                }
            }
        }

        // DEBUG: Print rendering results
 /*       std::cout << "TileRenderer: Rendered " << renderedTiles << " textured tiles, "
            << fallbackTiles << " fallback tiles" << std::endl;*/
    }

    void TileRenderer::RenderDebugGrid(Renderer& renderer) {
        if (!m_tileMap) return;

        auto& debugRenderer = renderer.GetDebugRenderer();
        const Color GRAY_COLOR = Color(0.5f, 0.5f, 0.5f, 1.0f);

        for (int row = 0; row < m_tileMap->GetGridRows(); ++row) {
            for (int col = 0; col < m_tileMap->GetGridCols(); ++col) {

                Vector2D topLeft = Vector2D(col * CELL_WIDTH, row * CELL_HEIGHT);
                Vector2D topRight = Vector2D(topLeft.x + CELL_WIDTH, topLeft.y);
                Vector2D bottomLeft = Vector2D(topLeft.x, topLeft.y + CELL_HEIGHT);
                Vector2D bottomRight = Vector2D(topLeft.x + CELL_WIDTH, topLeft.y + CELL_HEIGHT);

                // Draw the four lines that form the cell's border
                debugRenderer.DrawLine(topLeft, topRight, GRAY_COLOR);
                debugRenderer.DrawLine(topRight, bottomRight, GRAY_COLOR);
                debugRenderer.DrawLine(bottomRight, bottomLeft, GRAY_COLOR);
                debugRenderer.DrawLine(bottomLeft, topLeft, GRAY_COLOR);
            }
        }
    }

    GP2Engine::Vector2D TileRenderer::ScreenToTileCoords(const GP2Engine::Vector2D& screenPos, const GP2Engine::Camera& camera) const {
        if (!m_tileMap) return GP2Engine::Vector2D(-1.0f, -1.0f);

        // 1. Get Camera Properties
        // camPos is the World coordinate of the viewport's Top-Left corner (the pan offset).
        const glm::vec3& camPos = camera.GetPosition();
        float currentZoom = camera.GetZoom();

        // Constants for tile size (assuming TILE_PIXEL_WIDTH/HEIGHT are defined, e.g., 64.0f)
        const float TILE_WIDTH = TILE_PIXEL_WIDTH;
        const float TILE_HEIGHT = TILE_PIXEL_HEIGHT;

        // 2. Convert Screen to World Coordinates

        // Calculate how far the mouse is from the camera's top-left origin, 
        // scaled by the current zoom level.
        // Example: mouse at (100, 100) screen, zoom 2.0. World delta is (50, 50).
        float worldDeltaX = screenPos.x / currentZoom;
        float worldDeltaY = screenPos.y / currentZoom;

        // World Position = Camera World Position (Pan) + World Delta (Mouse Offset from Camera)
        float worldX = camPos.x + worldDeltaX;
        float worldY = camPos.y + worldDeltaY;

        // 3. Convert World Coordinates to Tile Coordinates (col, row)
        int col = static_cast<int>(std::floor(worldX / TILE_WIDTH));
        int row = static_cast<int>(std::floor(worldY / TILE_HEIGHT));

        // 4. Boundary Check
        if (col < 0 || col >= m_tileMap->GetGridCols() || row < 0 || row >= m_tileMap->GetGridRows()) {
            return GP2Engine::Vector2D(-1.0f, -1.0f);
        }

        return GP2Engine::Vector2D(static_cast<float>(col), static_cast<float>(row));
    }

    Vector2D TileRenderer::ScreenToTileCoordsNoCamera(const GP2Engine::Vector2D& screenPos) const {
        if (!m_tileMap || m_tileMap->GetGridCols() <= 0 || m_tileMap->GetGridRows() <= 0) {
            return GP2Engine::Vector2D(-1.0f, -1.0f);
        }

        // Convert screen position to tile indices (original logic)
        int col = static_cast<int>(screenPos.x / CELL_WIDTH);
        int row = static_cast<int>(screenPos.y / CELL_HEIGHT);

        // Check bounds
        if (col < 0 || col >= m_tileMap->GetGridCols() ||
            row < 0 || row >= m_tileMap->GetGridRows()) {
            return GP2Engine::Vector2D(-1.0f, -1.0f); // Out of bounds
        }

        return GP2Engine::Vector2D(static_cast<float>(col), static_cast<float>(row));
    }
    //Vector2D TileRenderer::ScreenToTileCoords(const GP2Engine::Vector2D& screenPos) const {

    //    if (!m_tileMap || m_tileMap->GetGridCols() <= 0 || m_tileMap->GetGridRows() <= 0) {
    //        return GP2Engine::Vector2D(-1.0f, -1.0f);
    //    }

    //    // Convert screen position to tile indices
    //    int col = static_cast<int>(screenPos.x / CELL_WIDTH);
    //    int row = static_cast<int>(screenPos.y / CELL_HEIGHT);

    //    // Check bounds
    //    if (col < 0 || col >= m_tileMap->GetGridCols() ||
    //        row < 0 || row >= m_tileMap->GetGridRows()) {
    //        return GP2Engine::Vector2D(-1.0f, -1.0f); // Out of bounds
    //    }

    //    return GP2Engine::Vector2D(static_cast<float>(col), static_cast<float>(row));
    //}

    void TileRenderer::RenderSelectionBox(GP2Engine::Renderer& renderer, int col, int row) {
        if (col < 0 || row < 0) return;

        auto& debugRenderer = renderer.GetDebugRenderer();
        const GP2Engine::Color HIGHLIGHT_COLOR = GP2Engine::Color(1.0f, 1.0f, 0.0f, 0.8f); // Yellow

        // Calculate the four corners of the tile
        float left = col * CELL_WIDTH;
        float top = row * CELL_HEIGHT;
        float right = left + CELL_WIDTH;
        float bottom = top + CELL_HEIGHT;

        // Define the corners
        GP2Engine::Vector2D p1(left, top);       // Top-Left
        GP2Engine::Vector2D p2(right, top);      // Top-Right
        GP2Engine::Vector2D p3(right, bottom);   // Bottom-Right
        GP2Engine::Vector2D p4(left, bottom);    // Bottom-Left

        // Draw the four sides of the rectangle using DrawLine:

        // 1. Top Edge (P1 -> P2)
        debugRenderer.DrawLine(p1, p2, HIGHLIGHT_COLOR, 3.0f);

        // 2. Right Edge (P2 -> P3)
        debugRenderer.DrawLine(p2, p3, HIGHLIGHT_COLOR, 3.0f);

        // 3. Bottom Edge (P3 -> P4)
        debugRenderer.DrawLine(p3, p4, HIGHLIGHT_COLOR, 3.0f);

        // 4. Left Edge (P4 -> P1)
        debugRenderer.DrawLine(p4, p1, HIGHLIGHT_COLOR, 3.0f);
    }

    void TileRenderer::InitializeDimensions() {
        ConfigLoader& config = ConfigLoader::GetInstance();

        int width = config.GetInt("window.width", 1024);
        int height = config.GetInt("window.height", 768);

        // Store the values
        m_windowWidth = (float)width;
        m_windowHeight = (float)height;
    }

    float TileRenderer::GetMapPixelWidth() const {
        if (m_tileMap) {
            return m_tileMap->GetGridCols() * CELL_WIDTH;
        }
        return 0.0f;
    }

    float TileRenderer::GetMapPixelHeight() const {
        if (m_tileMap) {
            return m_tileMap->GetGridRows() * CELL_HEIGHT;
        }
        return 0.0f;
    }
}