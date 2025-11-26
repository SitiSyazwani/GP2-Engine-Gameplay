/**
 * @file AISystem.cpp
 * @author Asri (100%)
 * @brief Implementation of ECS AI system with A* pathfinding
 *
 * Implements the AISystem that processes all entities with AIComponent,
 * providing intelligent pathfinding and chase behavior following proper
 * ECS architecture principles.
 */

#include "AISystem.hpp"
#include <Graphics/Sprite.hpp>
#include <Graphics/Texture.hpp>
#include <Physics/PhysicsSystem.hpp>
#include <algorithm>
#include <iostream>

namespace GP2Engine {

    void AISystem::Update(Registry& registry, float deltaTime) {
        // Iterate over all entities with AIComponent
        for (EntityID entity : registry.GetActiveEntities()) {
            AIComponent* aiComp = registry.GetComponent<AIComponent>(entity);
            if (!aiComp) continue; // Skip entities without AI

            Transform2D* transform = registry.GetComponent<Transform2D>(entity);
            if (!transform) continue; // AI requires Transform2D

            // Get target entity transform
            if (aiComp->targetEntity == INVALID_ENTITY) continue;
            Transform2D* targetTransform = registry.GetComponent<Transform2D>(aiComp->targetEntity);
            if (!targetTransform) continue;

            // Calculate direction to target (for range checking)
            Vector2D directionToTarget = ComputeDirectionToTarget(
                registry, entity, aiComp->targetEntity, aiComp->detectionRange
            );

            // Update chasing state
            aiComp->isChasing = (directionToTarget.x != 0.0f || directionToTarget.y != 0.0f);

            // If target is out of range, don't chase
            if (aiComp->isChasing) {
                Vector2D movementDirection(0.0f, 0.0f);

                // Only use pathfinding (no fallback)
                if (aiComp->usePathfinding && m_gridWidth > 0 && m_gridHeight > 0) {
                    // Update path recalculation timer
                    aiComp->pathRecalculateTimer += deltaTime;

                    // Get entity's path (or create if doesn't exist)
                    std::vector<GridNode>& currentPath = m_entityPaths[entity];

                    // Recalculate path periodically or if we don't have a path
                    if (currentPath.empty() || aiComp->pathRecalculateTimer >= aiComp->pathRecalculateInterval) {
                        aiComp->pathRecalculateTimer = 0.0f;

                        // Convert world positions to grid coordinates
                        GridNode startNode = WorldToGrid(transform->position);
                        GridNode goalNode = WorldToGrid(targetTransform->position);

                        // Find path using A*
                        currentPath = FindPath(startNode, goalNode);
                        aiComp->currentPathIndex = 0;
                    }

                    // Follow the path if we have one
                    if (!currentPath.empty() && aiComp->currentPathIndex < currentPath.size()) {
                        // Get target waypoint in world coordinates
                        Vector2D targetWaypoint = GridToWorld(currentPath[aiComp->currentPathIndex]);

                        // Calculate direction to waypoint
                        Vector2D toWaypoint = targetWaypoint - transform->position;
                        float distanceToWaypoint = toWaypoint.length();

                        // Check if we've reached the current waypoint
                        float waypointReachThreshold = m_tileSize * 0.3f; // 30% of tile size
                        if (distanceToWaypoint < waypointReachThreshold) {
                            // Move to next waypoint
                            aiComp->currentPathIndex++;

                            // Check if we reached the end of the path
                            if (aiComp->currentPathIndex >= currentPath.size()) {
                                // Reached destination, clear path
                                currentPath.clear();
                                aiComp->currentPathIndex = 0;
                            }
                        } else {
                            // Move towards waypoint
                            movementDirection = toWaypoint.normalized();
                        }
                    }
                }

                // Apply movement if we have a direction
                if (movementDirection.x != 0.0f || movementDirection.y != 0.0f) {
                    // Move towards target
                    Vector2D newPosition = transform->position;
                    newPosition.x += movementDirection.x * aiComp->chaseSpeed * deltaTime;
                    newPosition.y += movementDirection.y * aiComp->chaseSpeed * deltaTime;

                    // Try to move to new position
                    if (!WouldCollide(registry, entity, newPosition)) {
                        transform->position = newPosition;
                    }
                    else {
                        // Try sliding along obstacles (separate horizontal and vertical)
                        Vector2D horizontalMove = transform->position;
                        horizontalMove.x += movementDirection.x * aiComp->chaseSpeed * deltaTime;
                        if (!WouldCollide(registry, entity, horizontalMove)) {
                            transform->position = horizontalMove;
                        }
                        else {
                            Vector2D verticalMove = transform->position;
                            verticalMove.y += movementDirection.y * aiComp->chaseSpeed * deltaTime;
                            if (!WouldCollide(registry, entity, verticalMove)) {
                                transform->position = verticalMove;
                            }
                        }
                    }

                    // Update animation based on movement direction
                    UpdateAnimation(registry, entity, movementDirection, *aiComp);
                }
            }

            // Always update sprite animation timer
            SpriteComponent* spriteComp = registry.GetComponent<SpriteComponent>(entity);
            if (spriteComp && spriteComp->sprite) {
                spriteComp->sprite->UpdateAnimation(deltaTime);
            }
        }
    }

    Vector2D AISystem::ComputeDirectionToTarget(
        Registry& registry,
        EntityID aiEntity,
        EntityID targetEntity,
        float detectionRange
    ) {
        Transform2D* aiTransform = registry.GetComponent<Transform2D>(aiEntity);
        Transform2D* targetTransform = registry.GetComponent<Transform2D>(targetEntity);

        if (!aiTransform || !targetTransform) {
            return Vector2D(0.0f, 0.0f);
        }

        // Calculate direction vector
        Vector2D direction = targetTransform->position - aiTransform->position;
        float distance = direction.length();

        // Check if target is within detection range
        if (distance > detectionRange) {
            return Vector2D(0.0f, 0.0f);
        }

        // Return normalized direction
        if (distance > 0.0001f) {
            return direction.normalized();
        }

        return Vector2D(0.0f, 0.0f);
    }

    bool AISystem::WouldCollide(Registry& registry, EntityID entity, const Vector2D& position) {
        // Inline collision check to avoid dependency on EntityCollisionSystem
        auto* sprite = registry.GetComponent<SpriteComponent>(entity);
        auto* transform = registry.GetComponent<Transform2D>(entity);
        if (!sprite || !transform) return false;

        // Create AABB at test position
        float scaledWidth = sprite->size.x * transform->scale.x;
        float scaledHeight = sprite->size.y * transform->scale.y;
        AABB testBox(position.x - scaledWidth * 0.5f, position.y - scaledHeight * 0.5f, scaledWidth, scaledHeight);

        // Test against all other entities
        for (EntityID other : registry.GetActiveEntities()) {
            if (other == entity) continue;

            auto* otherSprite = registry.GetComponent<SpriteComponent>(other);
            auto* otherTransform = registry.GetComponent<Transform2D>(other);
            auto* otherTag = registry.GetComponent<Tag>(other);
            if (!otherSprite || !otherTransform || !otherTag) continue;

            // Skip non-solid entities
            if (otherTag->name == "Background" || otherTag->name == "StressTest") continue;

            // Create AABB for other entity
            float otherScaledWidth = otherSprite->size.x * otherTransform->scale.x;
            float otherScaledHeight = otherSprite->size.y * otherTransform->scale.y;
            AABB otherBox(otherTransform->position.x - otherScaledWidth * 0.5f,
                         otherTransform->position.y - otherScaledHeight * 0.5f,
                         otherScaledWidth, otherScaledHeight);

            // Check intersection
            if (testBox.Intersects(otherBox)) return true;
        }

        return false;
    }

    void AISystem::UpdateAnimation(
        Registry& registry,
        EntityID entity,
        const Vector2D& direction,
        AIComponent& aiComp
    ) {
        SpriteComponent* spriteComp = registry.GetComponent<SpriteComponent>(entity);
        if (!spriteComp || !spriteComp->sprite) {
            return;
        }

        // Cast texture pointers back to TexturePtr
        Texture* horizontalTexture = static_cast<Texture*>(aiComp.horizontalTexture);
        Texture* verticalTexture = static_cast<Texture*>(aiComp.verticalTexture);

        // Safety check: ensure textures are valid
        if (!horizontalTexture || !verticalTexture) {
            return;
        }

        std::string targetAnimation = "monster_walk_right";
        bool shouldFlipX = false;
        Texture* targetTexture = horizontalTexture;

        // Determine animation and texture based on movement direction
        if (direction.x != 0.0f || direction.y != 0.0f) {
            // Check if movement is more horizontal or vertical
            bool isHorizontalDominant = std::abs(direction.x) >= std::abs(direction.y);

            if (isHorizontalDominant) {
                // Horizontal movement - use horizontal texture
                targetTexture = horizontalTexture;
                targetAnimation = "monster_walk_right";

                // Flip horizontally based on direction
                if (direction.x < 0.0f) {
                    shouldFlipX = false;
                } else {
                    shouldFlipX = true;
                }
            } else {
                // Vertical movement - use vertical texture
                targetTexture = verticalTexture;
                shouldFlipX = false; // No flip for vertical movement

                if (direction.y < 0.0f) {
                    // Moving up
                    targetAnimation = "monster_walk_up";
                } else {
                    // Moving down
                    targetAnimation = "monster_walk_down";
                }
            }
        }

        // Switch texture if needed (with safety check)
        if (targetTexture) {
            Texture* currentTexture = spriteComp->sprite->GetTexture().get();
            if (currentTexture != targetTexture) {
                // Need to wrap raw pointer back to shared_ptr - this is a design issue
                // For now, skip texture switching in animation
            }
        }

        // Apply horizontal flip
        spriteComp->sprite->SetFlipX(shouldFlipX);

        // Only play animation if it's different from current
        if (targetAnimation != aiComp.currentAnimation) {
            spriteComp->sprite->PlayAnimation(targetAnimation);
            aiComp.currentAnimation = targetAnimation;
        } else {
            // Same animation, make sure it's playing
            spriteComp->sprite->ResumeAnimation();
        }
    }

    void AISystem::SetupPathfindingGrid(int gridWidth, int gridHeight, float tileSize) {
        m_gridWidth = gridWidth;
        m_gridHeight = gridHeight;
        m_tileSize = tileSize;

        // Initialize grid (all walkable by default)
        m_obstacles.resize(m_gridHeight);
        for (int y = 0; y < m_gridHeight; ++y) {
            m_obstacles[y].resize(m_gridWidth, false);
        }
    }

    void AISystem::SetWalkable(int x, int y, bool walkable) {
        if (!IsInBounds(x, y)) return;
        m_obstacles[y][x] = !walkable; // true = obstacle, false = walkable
    }

    bool AISystem::IsWalkable(int x, int y) const {
        if (!IsInBounds(x, y)) return false;
        return !m_obstacles[y][x]; // Invert: obstacle = false, walkable = true
    }

    bool AISystem::IsInBounds(int x, int y) const {
        return x >= 0 && x < m_gridWidth && y >= 0 && y < m_gridHeight;
    }

    GridNode AISystem::WorldToGrid(const Vector2D& worldPos) const {
        return GridNode(
            static_cast<int>(worldPos.x / m_tileSize),
            static_cast<int>(worldPos.y / m_tileSize)
        );
    }

    Vector2D AISystem::GridToWorld(const GridNode& node) const {
        // Return center of tile
        return Vector2D(
            (node.x + 0.5f) * m_tileSize,
            (node.y + 0.5f) * m_tileSize
        );
    }

    std::vector<GridNode> AISystem::FindPath(const GridNode& start, const GridNode& goal) {
        // Validate start and goal
        if (!IsWalkable(start.x, start.y) || !IsWalkable(goal.x, goal.y)) {
            return {}; // No path if start or goal is blocked
        }

        // Open set: nodes to evaluate (priority queue by fCost)
        std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;

        // Closed set: nodes already evaluated
        std::unordered_map<GridNode, bool, GridNode::Hash> closedSet;

        // G-costs: best known cost from start to each node
        std::unordered_map<GridNode, float, GridNode::Hash> gCosts;

        // Parent tracking for path reconstruction
        std::unordered_map<GridNode, GridNode, GridNode::Hash> cameFrom;

        // Initialize start node
        float startH = Heuristic(start, goal);
        openSet.push(AStarNode(start, 0, startH));
        gCosts[start] = 0;

        // A* main loop
        while (!openSet.empty()) {
            // Get node with lowest fCost
            AStarNode current = openSet.top();
            openSet.pop();

            // Skip if already processed
            if (closedSet[current.node]) continue;

            // Mark as processed
            closedSet[current.node] = true;

            // Goal reached!
            if (current.node == goal) {
                return ReconstructPath(cameFrom, start, goal);
            }

            // Check all neighbors
            std::vector<GridNode> neighbors = GetNeighbors(current.node);
            for (const GridNode& neighbor : neighbors) {
                // Skip if already evaluated
                if (closedSet[neighbor]) continue;

                // Calculate tentative gCost (always 1.0 for orthogonal movement)
                float moveCost = 1.0f;
                float tentativeGCost = current.gCost + moveCost;

                // Check if this path to neighbor is better
                auto it = gCosts.find(neighbor);
                if (it == gCosts.end() || tentativeGCost < it->second) {
                    // Update best path to neighbor
                    gCosts[neighbor] = tentativeGCost;
                    cameFrom[neighbor] = current.node;

                    // Add to open set
                    float hCost = Heuristic(neighbor, goal);
                    openSet.push(AStarNode(neighbor, tentativeGCost, hCost));
                }
            }
        }

        // No path found
        return {};
    }

    float AISystem::Heuristic(const GridNode& a, const GridNode& b) const {
        // Manhattan distance for 4-directional movement
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        return static_cast<float>(dx + dy);
    }

    std::vector<GridNode> AISystem::GetNeighbors(const GridNode& node) const {
        std::vector<GridNode> neighbors;

        // 4-directional neighbors (orthogonal only - no diagonal)
        const int dx[] = { 0, 1, 0, -1 };
        const int dy[] = { -1, 0, 1, 0 };

        for (int i = 0; i < 4; ++i) {
            int nx = node.x + dx[i];
            int ny = node.y + dy[i];

            if (IsWalkable(nx, ny)) {
                neighbors.push_back(GridNode(nx, ny));
            }
        }

        return neighbors;
    }

    std::vector<GridNode> AISystem::ReconstructPath(
        const std::unordered_map<GridNode, GridNode, GridNode::Hash>& cameFrom,
        const GridNode& start,
        const GridNode& goal
    ) {
        std::vector<GridNode> path;
        GridNode current = goal;

        // Trace back from goal to start
        while (current != start) {
            path.push_back(current);

            auto it = cameFrom.find(current);
            if (it == cameFrom.end()) {
                // Path reconstruction failed
                return {};
            }

            current = it->second;
        }

        // Add start node
        path.push_back(start);

        // Reverse to get path from start to goal
        std::reverse(path.begin(), path.end());

        return path;
    }

    void AISystem::RenderDebug(DebugRenderer& debugRenderer, Registry& registry) {
        // Iterate over all entities with AIComponent
        for (EntityID entity : registry.GetActiveEntities()) {
            AIComponent* aiComp = registry.GetComponent<AIComponent>(entity);
            if (!aiComp) continue;

            // Skip if not using pathfinding or not chasing
            if (!aiComp->usePathfinding || !aiComp->isChasing) {
                continue;
            }

            Transform2D* transform = registry.GetComponent<Transform2D>(entity);
            if (!transform) continue;

            // Draw AI entity position (red circle)
            debugRenderer.DrawCircle(
                transform->position,
                m_tileSize * 0.4f,
                Color::GetRed(),
                false
            );

            // Draw target position (blue circle) if valid
            if (aiComp->targetEntity != INVALID_ENTITY) {
                Transform2D* targetTransform = registry.GetComponent<Transform2D>(aiComp->targetEntity);
                if (targetTransform) {
                    debugRenderer.DrawCircle(
                        targetTransform->position,
                        m_tileSize * 0.4f,
                        Color::GetBlue(),
                        false
                    );
                }
            }

            // Draw current path if we have one
            auto pathIt = m_entityPaths.find(entity);
            if (pathIt != m_entityPaths.end() && !pathIt->second.empty()) {
                const std::vector<GridNode>& currentPath = pathIt->second;

                // Draw path as connected lines (green)
                for (size_t i = 0; i < currentPath.size() - 1; ++i) {
                    Vector2D start = GridToWorld(currentPath[i]);
                    Vector2D end = GridToWorld(currentPath[i + 1]);

                    debugRenderer.DrawLine(
                        start,
                        end,
                        Color::GetGreen(),
                        2.0f
                    );

                    // Draw waypoint markers (small green circles)
                    debugRenderer.DrawCircle(
                        start,
                        m_tileSize * 0.15f,
                        Color::GetGreen(),
                        true
                    );
                }

                // Draw final waypoint
                if (!currentPath.empty()) {
                    Vector2D lastWaypoint = GridToWorld(currentPath.back());
                    debugRenderer.DrawCircle(
                        lastWaypoint,
                        m_tileSize * 0.15f,
                        Color::GetGreen(),
                        true
                    );
                }

                // Draw current target waypoint (yellow circle)
                if (aiComp->currentPathIndex < currentPath.size()) {
                    Vector2D currentTarget = GridToWorld(currentPath[aiComp->currentPathIndex]);
                    debugRenderer.DrawCircle(
                        currentTarget,
                        m_tileSize * 0.3f,
                        Color::GetYellow(),
                        false
                    );
                }
            }
        }
    }

} // namespace GP2Engine
