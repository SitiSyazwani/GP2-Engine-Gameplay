/**
 * @file PhysicsSystem.cpp
 * @author Fauzan (100%)
 * @brief Enhanced 2D physics with comprehensive collision detection
 *
 * FIXES:
 * Implements AABB collision detection, player movement with stamina and hiding mechanics,
 * ghost AI (patrol/chase), and sound propagation for the game world.
 */

#include "PhysicsSystem.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GP2Engine {

    // ============================================================================
    // CIRCLE IMPLEMENTATION
    // ============================================================================

    Circle::Circle() : center(0.0f, 0.0f), radius(0.0f) {}

    Circle::Circle(const Vector2D& center, float radius)
        : center(center), radius(radius) {
    }

    Circle::Circle(float x, float y, float radius)
        : center(x, y), radius(radius) {
    }

    bool Circle::Intersects(const Circle& other) const {
        float radiusSum = radius + other.radius;
        float distSq = Vector2D::distanceSquared(center, other.center);
        return distSq <= (radiusSum * radiusSum);
    }

    bool Circle::Contains(const Vector2D& point) const {
        float distSq = Vector2D::distanceSquared(center, point);
        return distSq <= (radius * radius);
    }

    AABB Circle::GetAABB() const {
        return AABB(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
    }

    // ============================================================================
    // POLYGON IMPLEMENTATION
    // ============================================================================

    Polygon::Polygon() : position(0.0f, 0.0f) {}

    Polygon::Polygon(const Vector2D& pos, const std::vector<Vector2D>& verts)
        : position(pos), vertices(verts) {
        ComputeNormals();
    }

    std::vector<Vector2D> Polygon::GetTransformedVertices() const {
        std::vector<Vector2D> transformed;
        transformed.reserve(vertices.size());
        for (const auto& vertex : vertices) {
            transformed.push_back(vertex + position);
        }
        return transformed;
    }

    void Polygon::ComputeNormals() {
        normals.clear();
        size_t vertexCount = vertices.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            Vector2D v1 = vertices[i];
            Vector2D v2 = vertices[(i + 1) % vertexCount];

            Vector2D edge = v2 - v1;
            Vector2D normal(-edge.y, edge.x);  // Perpendicular
            normal.normalize();
            normals.push_back(normal);
        }
    }

    Polygon Polygon::CreateBox(const Vector2D& center, float width, float height) {
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;

        std::vector<Vector2D> vertices = {
            Vector2D(-halfW, -halfH),
            Vector2D(halfW, -halfH),
            Vector2D(halfW, halfH),
            Vector2D(-halfW, halfH)
        };

        return Polygon(center, vertices);
    }

    // ============================================================================
    // COLLISION MANIFOLD IMPLEMENTATION
    // ============================================================================

    CollisionManifold::CollisionManifold()
        : hasCollision(false), normal(0.0f, 0.0f),
        penetration(0.0f), contactPoint(0.0f, 0.0f) {
    }

    // ============================================================================
    // AABB IMPLEMENTATION
    // ============================================================================

    AABB::AABB(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height) {
    }

    bool AABB::Intersects(const AABB& other) const {
        return (x < other.x + other.width &&
            x + width > other.x &&
            y < other.y + other.height &&
            y + height > other.y);
    }

    bool AABB::Contains(const Vector2D& point) const {
        return (point.x >= x && point.x <= x + width &&
            point.y >= y && point.y <= y + height);
    }

    Vector2D AABB::GetCenter() const {
        return Vector2D(x + width * 0.5f, y + height * 0.5f);
    }

    Vector2D AABB::GetOverlap(const AABB& other) const {
        float overlapX = std::min(x + width, other.x + other.width) - std::max(x, other.x);
        float overlapY = std::min(y + height, other.y + other.height) - std::max(y, other.y);
        return Vector2D(overlapX, overlapY);
    }

    // ============================================================================
    // COLLISION DETECTION IMPLEMENTATION
    // ============================================================================

    namespace CollisionDetection {

        // Projection implementation
        Projection::Projection() : min(0.0f), max(0.0f) {}

        Projection::Projection(float min, float max) : min(min), max(max) {}

        float Projection::GetOverlap(const Projection& other) const {
            return std::min(max, other.max) - std::max(min, other.min);
        }

        bool Projection::Overlaps(const Projection& other) const {
            return GetOverlap(other) > 0.0f;
        }

        // AABB vs AABB (Enhanced with manifold)
        CollisionManifold CheckAABBvsAABB(const AABB& a, const AABB& b) {
            CollisionManifold manifold;

            if (!a.Intersects(b)) {
                return manifold;
            }

            manifold.hasCollision = true;
            Vector2D overlap = a.GetOverlap(b);

            if (overlap.x < overlap.y) {
                manifold.penetration = overlap.x;
                Vector2D aCenter = a.GetCenter();
                Vector2D bCenter = b.GetCenter();

                if (aCenter.x < bCenter.x) {
                    manifold.normal = Vector2D(1.0f, 0.0f);
                }
                else {
                    manifold.normal = Vector2D(-1.0f, 0.0f);
                }

                float contactX = aCenter.x < bCenter.x ? a.x + a.width : a.x;
                manifold.contactPoint = Vector2D(contactX, aCenter.y);
            }
            else {
                manifold.penetration = overlap.y;
                Vector2D aCenter = a.GetCenter();
                Vector2D bCenter = b.GetCenter();

                if (aCenter.y < bCenter.y) {
                    manifold.normal = Vector2D(0.0f, 1.0f);
                }
                else {
                    manifold.normal = Vector2D(0.0f, -1.0f);
                }

                float contactY = aCenter.y < bCenter.y ? a.y + a.height : a.y;
                manifold.contactPoint = Vector2D(aCenter.x, contactY);
            }

            return manifold;
        }

        // Circle vs Circle
        CollisionManifold CheckCirclevsCircle(const Circle& a, const Circle& b) {
            CollisionManifold manifold;

            Vector2D delta = b.center - a.center;
            float distSq = delta.lengthSquare();
            float radiusSum = a.radius + b.radius;
            float radiusSumSq = radiusSum * radiusSum;

            if (distSq >= radiusSumSq) {
                return manifold;
            }

            manifold.hasCollision = true;
            float distance = std::sqrt(distSq);
            manifold.penetration = radiusSum - distance;

            if (distance > 0.0001f) {
                manifold.normal = delta / distance;
            }
            else {
                manifold.normal = Vector2D(1.0f, 0.0f);
            }

            manifold.contactPoint = a.center + (manifold.normal * a.radius);
            return manifold;
        }

        // Circle vs AABB
        CollisionManifold CheckCirclevsAABB(const Circle& circle, const AABB& aabb) {
            CollisionManifold manifold;

            Vector2D aabbMin(aabb.x, aabb.y);
            Vector2D aabbMax(aabb.x + aabb.width, aabb.y + aabb.height);

            // Find closest point on AABB
            float closestX = std::max(aabbMin.x, std::min(circle.center.x, aabbMax.x));
            float closestY = std::max(aabbMin.y, std::min(circle.center.y, aabbMax.y));
            Vector2D closest(closestX, closestY);

            Vector2D delta = circle.center - closest;
            float distSq = delta.lengthSquare();
            float radiusSq = circle.radius * circle.radius;

            if (distSq > radiusSq) {
                return manifold;
            }

            manifold.hasCollision = true;

            // Circle center inside AABB
            if (aabb.Contains(circle.center)) {
                float leftDist = circle.center.x - aabbMin.x;
                float rightDist = aabbMax.x - circle.center.x;
                float topDist = circle.center.y - aabbMin.y;
                float bottomDist = aabbMax.y - circle.center.y;

                float minDist = std::min({ leftDist, rightDist, topDist, bottomDist });

                if (minDist == leftDist) {
                    manifold.normal = Vector2D(-1.0f, 0.0f);
                    manifold.penetration = circle.radius + leftDist;
                    manifold.contactPoint = Vector2D(aabbMin.x, circle.center.y);
                }
                else if (minDist == rightDist) {
                    manifold.normal = Vector2D(1.0f, 0.0f);
                    manifold.penetration = circle.radius + rightDist;
                    manifold.contactPoint = Vector2D(aabbMax.x, circle.center.y);
                }
                else if (minDist == topDist) {
                    manifold.normal = Vector2D(0.0f, -1.0f);
                    manifold.penetration = circle.radius + topDist;
                    manifold.contactPoint = Vector2D(circle.center.x, aabbMin.y);
                }
                else {
                    manifold.normal = Vector2D(0.0f, 1.0f);
                    manifold.penetration = circle.radius + bottomDist;
                    manifold.contactPoint = Vector2D(circle.center.x, aabbMax.y);
                }
            }
            else {
                float distance = std::sqrt(distSq);

                if (distance > 0.0001f) {
                    manifold.normal = delta / distance;
                }
                else {
                    manifold.normal = Vector2D(0.0f, 1.0f);
                }

                manifold.penetration = circle.radius - distance;
                manifold.contactPoint = closest;
            }

            return manifold;
        }

        // SAT Helper Function
        Projection ProjectPolygon(const std::vector<Vector2D>& vertices, const Vector2D& axis) {
            if (vertices.empty()) return Projection();

            float min = axis.dot(vertices[0]);
            float max = min;

            for (size_t i = 1; i < vertices.size(); ++i) {
                float projection = axis.dot(vertices[i]);
                min = std::min(min, projection);
                max = std::max(max, projection);
            }

            return Projection(min, max);
        }

        // Polygon vs Polygon (SAT)
        CollisionManifold CheckPolygonvsPolygon(const Polygon& a, const Polygon& b) {
            CollisionManifold manifold;

            std::vector<Vector2D> vertsA = a.GetTransformedVertices();
            std::vector<Vector2D> vertsB = b.GetTransformedVertices();

            float minOverlap = std::numeric_limits<float>::max();
            Vector2D smallestAxis;

            // Test axes from polygon A
            for (const auto& normal : a.normals) {
                Projection projA = ProjectPolygon(vertsA, normal);
                Projection projB = ProjectPolygon(vertsB, normal);

                if (!projA.Overlaps(projB)) {
                    return manifold;  // No collision
                }

                float overlap = projA.GetOverlap(projB);
                if (overlap < minOverlap) {
                    minOverlap = overlap;
                    smallestAxis = normal;
                }
            }

            // Test axes from polygon B
            for (const auto& normal : b.normals) {
                Projection projA = ProjectPolygon(vertsA, normal);
                Projection projB = ProjectPolygon(vertsB, normal);

                if (!projA.Overlaps(projB)) {
                    return manifold;
                }

                float overlap = projA.GetOverlap(projB);
                if (overlap < minOverlap) {
                    minOverlap = overlap;
                    smallestAxis = normal;
                }
            }

            // Collision detected
            manifold.hasCollision = true;
            manifold.penetration = minOverlap;
            manifold.normal = smallestAxis;

            // Approximate contact point
            Vector2D centerA(0, 0), centerB(0, 0);
            for (const auto& v : vertsA) centerA += v;
            for (const auto& v : vertsB) centerB += v;
            centerA = centerA / static_cast<float>(vertsA.size());
            centerB = centerB / static_cast<float>(vertsB.size());

            // Ensure normal points from A to B
            if ((centerB - centerA).dot(smallestAxis) < 0) {
                manifold.normal = smallestAxis * -1.0f;
            }

            manifold.contactPoint = (centerA + centerB) * 0.5f;
            return manifold;
        }

    } // namespace CollisionDetection

    // ============================================================================
    // PHYSICS BODY IMPLEMENTATION
    // ============================================================================

    PhysicsBody::PhysicsBody(float x, float y, float width, float height, float mass)
        : position(x, y), bounds(x - width / 2, y - height / 2, width, height),
        friction(0.8f), maxSpeed(200.0f), mass(mass) {

        // Default to AABB shape (backward compatible)
        shapeType = ShapeType::AABB_SHAPE;
        restitution = 0.3f;
    }

    void PhysicsBody::InitAsCircle(float radius) {
        shapeType = ShapeType::CIRCLE_SHAPE;
        collisionCircle = Circle(position, radius);
        bounds = collisionCircle.GetAABB();
    }

    void PhysicsBody::Update(float deltaTime) {
        velocity += acceleration * deltaTime;
        velocity *= std::pow(friction, deltaTime);

        if (velocity.length() > maxSpeed) {
            velocity = velocity.normalized() * maxSpeed;
        }

        position += velocity * deltaTime;
        UpdateBounds();
        acceleration = Vector2D(0.0f, 0.0f);
    }

    void PhysicsBody::ApplyForce(const Vector2D& force) {
        acceleration += force / mass;
    }

    void PhysicsBody::ApplyImpulse(const Vector2D& impulse) {
        velocity += impulse / mass;
    }

    void PhysicsBody::SetPosition(const Vector2D& newPos) {
        position = newPos;
        UpdateBounds();
    }

    void PhysicsBody::UpdateBounds() {
        if (shapeType == ShapeType::CIRCLE_SHAPE) {
            collisionCircle.center = position;
            bounds = collisionCircle.GetAABB();
        }
        else {
            bounds.x = position.x - bounds.width * 0.5f;
            bounds.y = position.y - bounds.height * 0.5f;
        }
    }

    Circle PhysicsBody::GetCircle() const {
        return collisionCircle;
    }

    // ============================================================================
    // PLAYER PHYSICS IMPLEMENTATION
    // ============================================================================

    PlayerPhysics::Input::Input()
        : left(false), right(false), up(false), down(false),
        sprint(false), hide(false), flashlightToggle(false) {
    }

    PlayerPhysics::PlayerPhysics(float x, float y) : PhysicsBody(x, y, 20.0f, 20.0f) {
        // Use circle collision for smoother player movement!
        InitAsCircle(10.0f);

        stamina = maxStamina = 100.0f;
        battery = maxBattery = 100.0f;
        soundLevel = 0.0f;
        maxSoundLevel = 100.0f;

        moveSpeed = 300.0f;
        sprintSpeed = 500.0f;
        staminaDrain = 50.0f;
        staminaRegen = 30.0f;
        batteryDrain = 10.0f;

        flashlightOn = false;
        flashlightAngle = 0.0f;
        flashlightRange = 150.0f;
        flashlightConeAngle = static_cast<float>(M_PI) / 3.0f;

        isHidden = false;
        isSprinting = false;
    }

    void PlayerPhysics::Update(float deltaTime, const std::vector<AABB>& hideSpots) {
        if (input.flashlightToggle) {
            ToggleFlashlight();
            input.flashlightToggle = false;
        }

        UpdateFlashlightDirection();

        Vector2D moveDirection = GetMoveDirection();

        isSprinting = input.sprint && stamina > 0.0f && !(moveDirection.x == 0.0f && moveDirection.y == 0.0f);

        if (!(moveDirection.x == 0.0f && moveDirection.y == 0.0f)) {
            float currentSpeed = isSprinting ? sprintSpeed : moveSpeed;
            ApplyForce(moveDirection.normalized() * currentSpeed);
        }

        PhysicsBody::Update(deltaTime);

        UpdateStamina(deltaTime);
        UpdateBattery(deltaTime);
        UpdateSound(deltaTime);
        CheckHiding(hideSpots);
    }

    Vector2D PlayerPhysics::GetMoveDirection() const {
        Vector2D direction(0.0f, 0.0f);
        if (input.left)  direction.x -= 1.0f;
        if (input.right) direction.x += 1.0f;
        if (input.up)    direction.y -= 1.0f;
        if (input.down)  direction.y += 1.0f;
        return direction;
    }

    void PlayerPhysics::UpdateStamina(float deltaTime) {
        if (isSprinting) {
            stamina -= staminaDrain * deltaTime;
            if (stamina < 0.0f) stamina = 0.0f;
        }
        else {
            stamina += staminaRegen * deltaTime;
            if (stamina > maxStamina) stamina = maxStamina;
        }
    }

    void PlayerPhysics::UpdateBattery(float deltaTime) {
        if (flashlightOn) {
            battery -= batteryDrain * deltaTime;
            if (battery < 0.0f) {
                battery = 0.0f;
                flashlightOn = false;
            }
        }
    }

    void PlayerPhysics::UpdateSound(float deltaTime) {
        float targetSound = 0.0f;

        if (isSprinting) {
            targetSound = 80.0f;
        }
        else if (!(velocity.x == 0.0f && velocity.y == 0.0f)) {
            targetSound = 30.0f;
        }

        if (isHidden) {
            targetSound = 0.0f;
        }

        float lerpSpeed = 5.0f;
        soundLevel += (targetSound - soundLevel) * lerpSpeed * deltaTime;

        if (soundLevel < 1.0f) soundLevel = 0.0f;
        if (soundLevel > maxSoundLevel) soundLevel = maxSoundLevel;
    }

    void PlayerPhysics::CheckHiding(const std::vector<AABB>& hideSpots) {
        isHidden = false;

        if (input.hide) {
            for (const auto& spot : hideSpots) {
                if (spot.Contains(position)) {
                    isHidden = true;
                    velocity = Vector2D(0.0f, 0.0f);
                    break;
                }
            }
        }
    }

    void PlayerPhysics::ToggleFlashlight() {
        if (battery > 0.0f) {
            flashlightOn = !flashlightOn;
        }
    }

    void PlayerPhysics::UpdateFlashlightDirection() {
        Vector2D toMouse = input.mousePosition - position;
        flashlightAngle = std::atan2(toMouse.y, toMouse.x);
    }

    bool PlayerPhysics::IsPointInFlashlight(const Vector2D& point) const {
        if (!flashlightOn || battery <= 0.0f) return false;

        Vector2D toPoint = point - position;
        float distance = toPoint.length();

        if (distance > flashlightRange) return false;

        float angleToPoint = std::atan2(toPoint.y, toPoint.x);
        float angleDiff = std::abs(angleToPoint - flashlightAngle);

        constexpr float PI_F = static_cast<float>(M_PI);
        while (angleDiff > PI_F) angleDiff -= 2.0f * PI_F;
        angleDiff = std::abs(angleDiff);

        return angleDiff <= flashlightConeAngle * 0.5f;
    }

    // ============================================================================
    // GHOST AI IMPLEMENTATION
    // ============================================================================

    GhostAI::GhostAI(float x, float y) : PhysicsBody(x, y, 30.0f, 30.0f) {
        // Ghosts can also use circle collision
        InitAsCircle(15.0f);

        state = State::Patrol;
        patrolSpeed = 100.0f;
        chaseSpeed = 200.0f;
        maxSpeed = 200.0f;
        detectionRange = 200.0f;
        hearingRange = 150.0f;
        currentPatrolIndex = 0;
    }

    void GhostAI::SetPatrolPath(const std::vector<Vector2D>& path) {
        patrolPath = path;
        currentPatrolIndex = 0;
    }

    void GhostAI::Update(float deltaTime, const PlayerPhysics& player) {
        switch (state) {
        case State::Patrol:
            UpdatePatrol(deltaTime);
            CheckPlayerDetection(player);
            break;
        case State::Chase:
            UpdateChase(deltaTime, player);
            break;
        case State::Investigate:
            UpdateInvestigate(deltaTime);
            CheckPlayerDetection(player);
            break;
        }

        PhysicsBody::Update(deltaTime);
    }

    void GhostAI::UpdatePatrol(float /*deltaTime*/) {
        if (patrolPath.empty()) return;

        Vector2D target = patrolPath[currentPatrolIndex];
        Vector2D direction = target - position;
        float distance = direction.length();

        if (distance < 10.0f) {
            currentPatrolIndex = (currentPatrolIndex + 1) % patrolPath.size();
        }
        else {
            ApplyForce(direction.normalized() * patrolSpeed);
        }
    }

    void GhostAI::UpdateChase(float /*deltaTime*/, const PlayerPhysics& player) {
        if (player.isHidden) {
            state = State::Patrol;
            return;
        }

        Vector2D direction = player.position - position;
        float distance = direction.length();

        if (distance > detectionRange * 2.0f) {
            state = State::Patrol;
        }
        else {
            ApplyForce(direction.normalized() * chaseSpeed);
        }
    }

    void GhostAI::UpdateInvestigate(float /*deltaTime*/) {
        Vector2D direction = investigateTarget - position;
        float distance = direction.length();

        if (distance < 20.0f) {
            state = State::Patrol;
        }
        else {
            ApplyForce(direction.normalized() * patrolSpeed);
        }
    }

    void GhostAI::CheckPlayerDetection(const PlayerPhysics& player) {
        if (player.isHidden) return;

        float distance = Vector2D::distance(position, player.position);

        if (player.flashlightOn && player.IsPointInFlashlight(position)) {
            state = State::Chase;
            return;
        }

        if (distance < detectionRange) {
            state = State::Chase;
            return;
        }

        if (player.soundLevel > 50.0f && distance < hearingRange) {
            state = State::Investigate;
            investigateTarget = player.position;
        }
    }

    // ============================================================================
    // COLLISION SYSTEM IMPLEMENTATION
    // ============================================================================

    void CollisionSystem::ResolveWallCollision(PhysicsBody& body, const AABB& wall) {
        CollisionManifold manifold;

        // Use appropriate collision detection based on shape type
        if (body.shapeType == ShapeType::CIRCLE_SHAPE) {
            manifold = CollisionDetection::CheckCirclevsAABB(body.GetCircle(), wall);
        }
        else {
            manifold = CollisionDetection::CheckAABBvsAABB(body.bounds, wall);
        }

        if (!manifold.hasCollision) return;

        // Apply collision response with physics
        const float percent = 0.8f;
        const float slop = 0.01f;
        float correctionAmount = std::max(manifold.penetration - slop, 0.0f) * percent;
        Vector2D correction = manifold.normal * correctionAmount;

        body.position += correction;
        body.UpdateBounds();

        // Reflect velocity along normal (with restitution for bouncing)
        float velAlongNormal = body.velocity.dot(manifold.normal);
        if (velAlongNormal < 0) {
            Vector2D impulse = manifold.normal * (-velAlongNormal * (1.0f + body.restitution));
            body.velocity += impulse;
        }
    }

    void CollisionSystem::HandleWallCollisions(PhysicsBody& body, const std::vector<AABB>& walls) {
        for (const auto& wall : walls) {
            ResolveWallCollision(body, wall);
        }
    }

    void CollisionSystem::ResolveBodyCollision(PhysicsBody& a, PhysicsBody& b) {
        CollisionManifold manifold;

        // Determine collision type based on shapes
        if (a.shapeType == ShapeType::CIRCLE_SHAPE && b.shapeType == ShapeType::CIRCLE_SHAPE) {
            manifold = CollisionDetection::CheckCirclevsCircle(a.GetCircle(), b.GetCircle());
        }
        else if (a.shapeType == ShapeType::CIRCLE_SHAPE && b.shapeType == ShapeType::AABB_SHAPE) {
            manifold = CollisionDetection::CheckCirclevsAABB(a.GetCircle(), b.bounds);
        }
        else if (a.shapeType == ShapeType::AABB_SHAPE && b.shapeType == ShapeType::CIRCLE_SHAPE) {
            manifold = CollisionDetection::CheckCirclevsAABB(b.GetCircle(), a.bounds);
            manifold.normal = manifold.normal * -1.0f;
        }
        else {
            manifold = CollisionDetection::CheckAABBvsAABB(a.bounds, b.bounds);
        }

        if (!manifold.hasCollision) return;

        // Calculate relative velocity
        Vector2D relativeVel = b.velocity - a.velocity;
        float velAlongNormal = relativeVel.dot(manifold.normal);

        if (velAlongNormal > 0) return;  // Bodies separating

        // Calculate impulse
        float restitution = std::min(a.restitution, b.restitution);
        float impulseScalar = -(1.0f + restitution) * velAlongNormal;
        impulseScalar /= (1.0f / a.mass + 1.0f / b.mass);

        Vector2D impulse = manifold.normal * impulseScalar;
        a.velocity -= impulse / a.mass;
        b.velocity += impulse / b.mass;

        // Positional correction
        const float percent = 0.4f;
        const float slop = 0.01f;
        float correctionAmount = std::max(manifold.penetration - slop, 0.0f) * percent;
        Vector2D correction = manifold.normal * correctionAmount;

        float totalInvMass = 1.0f / a.mass + 1.0f / b.mass;
        a.position -= correction * (1.0f / a.mass / totalInvMass);
        b.position += correction * (1.0f / b.mass / totalInvMass);

        a.UpdateBounds();
        b.UpdateBounds();
    }

    // ============================================================================
    // SOUND SYSTEM IMPLEMENTATION
    // ============================================================================

    SoundSystem::SoundEvent::SoundEvent(const Vector2D& pos, float intens, float time, float dur)
        : position(pos), intensity(intens), timestamp(time), duration(dur) {
    }

    void SoundSystem::AddSound(const Vector2D& position, float intensity, float currentTime) {
        activeSounds.emplace_back(position, intensity, currentTime);
    }

    void SoundSystem::Update(float currentTime) {
        activeSounds.erase(
            std::remove_if(activeSounds.begin(), activeSounds.end(),
                [currentTime](const SoundEvent& sound) {
                    return currentTime - sound.timestamp > sound.duration;
                }),
            activeSounds.end()
        );
    }

    float SoundSystem::GetSoundIntensityAt(const Vector2D& position) const {
        float totalIntensity = 0.0f;

        for (const auto& sound : activeSounds) {
            float distance = Vector2D::distance(position, sound.position);
            float attenuation = 1.0f / (1.0f + distance * 0.01f);
            totalIntensity += sound.intensity * attenuation;
        }

        return totalIntensity;
    }

    // ============================================================================
    // PHYSICS WORLD IMPLEMENTATION
    // ============================================================================

    PhysicsWorld::PhysicsWorld() : player(100.0f, 100.0f), ghost(400.0f, 300.0f), currentTime(0.0f) {
        InitializeWorld();
    }

    void PhysicsWorld::Update(float deltaTime) {
        currentTime += deltaTime;

        player.Update(deltaTime, hideSpots);
        ghost.Update(deltaTime, player);

        // Handle collisions
        CollisionSystem::HandleWallCollisions(player, walls);
        CollisionSystem::HandleWallCollisions(ghost, walls);

        // Check player vs ghost collision (Circle vs Circle!)
        if (!player.isHidden) {
            CollisionManifold manifold =
                CollisionDetection::CheckCirclevsCircle(player.GetCircle(), ghost.GetCircle());

            if (manifold.hasCollision) {
                // Game over or damage logic here
            }
        }

        if (player.soundLevel > 10.0f) {
            soundSystem.AddSound(player.position, player.soundLevel, currentTime);
        }
        soundSystem.Update(currentTime);

        CheckGameConditions();
    }

    void PhysicsWorld::SetPlayerInput(bool left, bool right, bool up, bool down, bool sprint, bool hide) {
        player.input.left = left;
        player.input.right = right;
        player.input.up = up;
        player.input.down = down;
        player.input.sprint = sprint;
        player.input.hide = hide;
    }

    void PhysicsWorld::SetMouseInput(const Vector2D& mousePos, bool flashlightToggle) {
        player.input.mousePosition = mousePos;
        player.input.flashlightToggle = flashlightToggle;
    }

    void PhysicsWorld::InitializeWorld() {
        walls = {
            AABB(0, 0, 800, 20),
            AABB(0, 580, 800, 20),
            AABB(0, 0, 20, 600),
            AABB(780, 0, 20, 600),
            AABB(150, 100, 20, 200),
            AABB(300, 200, 200, 20),
            AABB(500, 80, 20, 140),
            AABB(650, 350, 20, 150),
            AABB(250, 400, 150, 20),
        };

        hideSpots = {
            AABB(50, 50, 40, 30),
            AABB(700, 500, 50, 40),
            AABB(350, 450, 30, 40),
        };

        std::vector<Vector2D> patrolPath = {
            Vector2D(400, 300), Vector2D(600, 300),
            Vector2D(600, 150), Vector2D(200, 150)
        };
        ghost.SetPatrolPath(patrolPath);
    }

    void PhysicsWorld::CheckGameConditions() {
        // Check if ghost caught player using circle collision
        if (!player.isHidden) {
            CollisionManifold manifold =
                CollisionDetection::CheckCirclevsCircle(player.GetCircle(), ghost.GetCircle());

            if (manifold.hasCollision) {
                // Game over logic
            }
        }
    }

} // namespace GP2Engine