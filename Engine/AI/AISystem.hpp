/**
 * @file AISystem.hpp
 * @author Asri (100%)
 * @brief ECS AI system for pathfinding and chase behavior
 *
 * Processes all entities with AIComponent to provide A* pathfinding and
 * intelligent chase behavior. Follows proper ECS architecture where
 * any entity can have AI by attaching AIComponent.
 *
 * Features:
 * - A* pathfinding algorithm for smart navigation
 * - Grid-based obstacle detection
 * - Target tracking and chase behavior
 * - Path recalculation when target moves
 * - Dynamic animation switching based on movement direction
 */

#pragma once

#include <ECS/Registry.hpp>
#include <ECS/Component.hpp>
#include <Graphics/DebugRenderer.hpp>
#include <vector>
#include <queue>
#include <unordered_map>

namespace GP2Engine {

    /**
     * @brief Grid node for A* pathfinding
     */
    struct GridNode {
        int x, y;

        GridNode() : x(0), y(0) {}
        GridNode(int nx, int ny) : x(nx), y(ny) {}

        bool operator==(const GridNode& other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const GridNode& other) const {
            return !(*this == other);
        }

        // Hash function for unordered_map
        struct Hash {
            size_t operator()(const GridNode& node) const {
                return std::hash<int>()(node.x) ^ (std::hash<int>()(node.y) << 1);
            }
        };
    };

    /**
     * @brief ECS System for AI pathfinding and behavior
     *
     * Processes all entities with AIComponent to handle:
     * - A* pathfinding with obstacle avoidance
     * - Target tracking and chase behavior
     * - Grid-based navigation
     * - Path recalculation
     * - Dynamic animation switching
     *
     * The system maintains a shared pathfinding grid that all AI entities use.
     */
    class AISystem {
    public:
        AISystem() = default;

        /**
         * @brief Update all entities with AIComponent
         * @param registry ECS registry
         * @param deltaTime Frame time in seconds
         */
        void Update(Registry& registry, float deltaTime);

        /**
         * @brief Setup pathfinding grid based on tilemap
         * @param gridWidth Width of navigation grid
         * @param gridHeight Height of navigation grid
         * @param tileSize Size of each tile in pixels
         */
        void SetupPathfindingGrid(int gridWidth, int gridHeight, float tileSize);

        /**
         * @brief Mark a grid cell as walkable or obstacle
         * @param x Grid X coordinate
         * @param y Grid Y coordinate
         * @param walkable True if walkable, false for obstacle
         */
        void SetWalkable(int x, int y, bool walkable);

        /**
         * @brief Render debug visualization for all AI entities
         * @param debugRenderer Debug renderer for drawing debug shapes
         * @param registry ECS registry
         */
        void RenderDebug(DebugRenderer& debugRenderer, Registry& registry);

    private:
        // Shared pathfinding grid (all AI entities use this)
        int m_gridWidth = 0;
        int m_gridHeight = 0;
        float m_tileSize = 64.0f;
        std::vector<std::vector<bool>> m_obstacles;  // false = walkable, true = obstacle

        // Per-entity path storage
        std::unordered_map<EntityID, std::vector<GridNode>> m_entityPaths;

        // Helper methods
        Vector2D ComputeDirectionToTarget(Registry& registry, EntityID aiEntity, EntityID targetEntity, float detectionRange);
        bool WouldCollide(Registry& registry, EntityID entity, const Vector2D& position);
        void UpdateAnimation(Registry& registry, EntityID entity, const Vector2D& direction, AIComponent& aiComp);

        // A* pathfinding methods
        struct AStarNode {
            GridNode node;
            float gCost, hCost, fCost;
            AStarNode() : gCost(0), hCost(0), fCost(0) {}
            AStarNode(const GridNode& n, float g, float h) : node(n), gCost(g), hCost(h), fCost(g + h) {}
            bool operator>(const AStarNode& other) const { return fCost > other.fCost; }
        };

        std::vector<GridNode> FindPath(const GridNode& start, const GridNode& goal);
        float Heuristic(const GridNode& a, const GridNode& b) const;
        std::vector<GridNode> GetNeighbors(const GridNode& node) const;
        bool IsInBounds(int x, int y) const;
        bool IsWalkable(int x, int y) const;
        std::vector<GridNode> ReconstructPath(
            const std::unordered_map<GridNode, GridNode, GridNode::Hash>& cameFrom,
            const GridNode& start,
            const GridNode& goal
        );
        GridNode WorldToGrid(const Vector2D& worldPos) const;
        Vector2D GridToWorld(const GridNode& node) const;
    };

} // namespace GP2Engine
