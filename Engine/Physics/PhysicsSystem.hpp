/**
 * @file PhysicsSystem.hpp
 * @author Fauzan(100%)
 * @brief Enhanced 2D physics system header with comprehensive collision detection
 *
 * Defines the AABB structure and the PhysicsWorld class, managing collision detection,
 * entity state updates, AI behavior, sound propagation, and win/lose game conditions.
 */

#pragma once

#include "../Math/Vector2D.hpp"
#include "../Math/Matrix3x3.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

namespace GP2Engine {

    // Forward declarations
    struct Circle;
    struct Polygon;
    struct CollisionManifold;
    enum class ShapeType;

    // ============================================================================
    // AABB (Axis-Aligned Bounding Box)
    // ============================================================================

    struct AABB {
        float x, y;
        float width, height;

        AABB() : x(0), y(0), width(0), height(0) {}
        AABB(float x, float y, float width, float height);

        bool Intersects(const AABB& other) const;
        bool Contains(const Vector2D& point) const;
        Vector2D GetCenter() const;
        Vector2D GetOverlap(const AABB& other) const;
    };

    // ============================================================================
    // CIRCLE (NEW!)
    // ============================================================================

    struct Circle {
        Vector2D center;
        float radius;

        Circle();
        Circle(const Vector2D& center, float radius);
        Circle(float x, float y, float radius);

        bool Intersects(const Circle& other) const;
        bool Contains(const Vector2D& point) const;
        AABB GetAABB() const;
    };

    // ============================================================================
    // POLYGON (NEW! - for SAT)
    // ============================================================================

    struct Polygon {
        Vector2D position;
        std::vector<Vector2D> vertices;
        std::vector<Vector2D> normals;

        Polygon();
        Polygon(const Vector2D& pos, const std::vector<Vector2D>& verts);

        std::vector<Vector2D> GetTransformedVertices() const;
        void ComputeNormals();

        static Polygon CreateBox(const Vector2D& center, float width, float height);
    };

    // ============================================================================
    // COLLISION MANIFOLD (NEW!)
    // ============================================================================

    struct CollisionManifold {
        bool hasCollision;
        Vector2D normal;
        float penetration;
        Vector2D contactPoint;

        CollisionManifold();
    };

    // ============================================================================
    // SHAPE TYPE ENUM (NEW!)
    // ============================================================================

    enum class ShapeType {
        AABB_SHAPE,
        CIRCLE_SHAPE,
        POLYGON_SHAPE
    };

    // ============================================================================
    // COLLISION DETECTION NAMESPACE (NEW!)
    // ============================================================================

    namespace CollisionDetection {

        // AABB vs AABB (Enhanced with manifold)
        CollisionManifold CheckAABBvsAABB(const AABB& a, const AABB& b);

        // Circle vs Circle (NEW!)
        CollisionManifold CheckCirclevsCircle(const Circle& a, const Circle& b);

        // Circle vs AABB (NEW!)
        CollisionManifold CheckCirclevsAABB(const Circle& circle, const AABB& aabb);

        // Polygon vs Polygon - SAT (NEW!)
        CollisionManifold CheckPolygonvsPolygon(const Polygon& a, const Polygon& b);

        // Helper structures for SAT
        struct Projection {
            float min, max;
            Projection();
            Projection(float min, float max);
            float GetOverlap(const Projection& other) const;
            bool Overlaps(const Projection& other) const;
        };

        Projection ProjectPolygon(const std::vector<Vector2D>& vertices, const Vector2D& axis);
    }

    // ============================================================================
    // PHYSICS BODY (ENHANCED)
    // ============================================================================

    class PhysicsBody {
    public:
        Vector2D position;
        Vector2D velocity;
        Vector2D acceleration;
        AABB bounds;
        float mass;
        float friction;
        float maxSpeed;

        // NEW: Collision shape support
        ShapeType shapeType;
        Circle collisionCircle;
        float restitution;  // Bounciness (0 = no bounce, 1 = perfect bounce)

        PhysicsBody(float x, float y, float width, float height, float mass = 1.0f);

        virtual void Update(float deltaTime);
        void ApplyForce(const Vector2D& force);
        void ApplyImpulse(const Vector2D& impulse);  // NEW!
        void SetPosition(const Vector2D& newPos);

        // NEW: Shape initialization
        void InitAsCircle(float radius);
        Circle GetCircle() const;

        // FIXED: Made public so CollisionSystem can call it
        void UpdateBounds();

        // Public wrapper for backward compatibility
        void UpdateBoundsPublic() { UpdateBounds(); }
    };

    // ============================================================================
    // PLAYER PHYSICS
    // ============================================================================

    class PlayerPhysics : public PhysicsBody {
    public:
        struct Input {
            bool left, right, up, down;
            bool sprint, hide;
            Vector2D mousePosition;
            bool flashlightToggle;

            Input();
        } input;

        float stamina, maxStamina;
        float battery, maxBattery;
        float soundLevel, maxSoundLevel;

        float moveSpeed, sprintSpeed;
        float staminaDrain, staminaRegen;
        float batteryDrain;

        bool flashlightOn;
        float flashlightAngle;
        float flashlightRange;
        float flashlightConeAngle;

        bool isHidden;
        bool isSprinting;

        PlayerPhysics(float x, float y);

        void Update(float deltaTime, const std::vector<AABB>& hideSpots);
        bool IsPointInFlashlight(const Vector2D& point) const;

    private:
        Vector2D GetMoveDirection() const;
        void UpdateStamina(float deltaTime);
        void UpdateBattery(float deltaTime);
        void UpdateSound(float deltaTime);
        void CheckHiding(const std::vector<AABB>& hideSpots);
        void ToggleFlashlight();
        void UpdateFlashlightDirection();
    };

    // ============================================================================
    // GHOST AI
    // ============================================================================

    class GhostAI : public PhysicsBody {
    public:
        enum class State {
            Patrol,
            Chase,
            Investigate
        };

        State state;
        float patrolSpeed;
        float chaseSpeed;
        float detectionRange;
        float hearingRange;

        std::vector<Vector2D> patrolPath;
        int currentPatrolIndex;
        Vector2D investigateTarget;

        GhostAI(float x, float y);

        void SetPatrolPath(const std::vector<Vector2D>& path);
        void Update(float deltaTime, const PlayerPhysics& player);

    private:
        void UpdatePatrol(float deltaTime);
        void UpdateChase(float deltaTime, const PlayerPhysics& player);
        void UpdateInvestigate(float deltaTime);
        void CheckPlayerDetection(const PlayerPhysics& player);
    };

    // ============================================================================
    // COLLISION SYSTEM (ENHANCED)
    // ============================================================================

    class CollisionSystem {
    public:
        static void ResolveWallCollision(PhysicsBody& body, const AABB& wall);
        static void HandleWallCollisions(PhysicsBody& body, const std::vector<AABB>& walls);

        // NEW: Resolve collision between two physics bodies
        static void ResolveBodyCollision(PhysicsBody& a, PhysicsBody& b);
    };

    // ============================================================================
    // SOUND SYSTEM
    // ============================================================================

    class SoundSystem {
    public:
        struct SoundEvent {
            Vector2D position;
            float intensity;
            float timestamp;
            float duration;

            SoundEvent(const Vector2D& pos, float intens, float time, float dur = 2.0f);
        };

        std::vector<SoundEvent> activeSounds;

        void AddSound(const Vector2D& position, float intensity, float currentTime);
        void Update(float currentTime);
        float GetSoundIntensityAt(const Vector2D& position) const;
    };

    // ============================================================================
    // PHYSICS WORLD
    // ============================================================================

    class PhysicsWorld {
    public:
        PlayerPhysics player;
        GhostAI ghost;
        std::vector<AABB> walls;
        std::vector<AABB> hideSpots;
        SoundSystem soundSystem;
        float currentTime;

        PhysicsWorld();

        void Update(float deltaTime);
        void SetPlayerInput(bool left, bool right, bool up, bool down, bool sprint, bool hide);
        void SetMouseInput(const Vector2D& mousePos, bool flashlightToggle);

    private:
        void InitializeWorld();
        void CheckGameConditions();
    };

} // namespace GP2Engine