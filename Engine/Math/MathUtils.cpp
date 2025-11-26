/**
 * @file MathUtils.cpp
 * @author Rifqah (100%)
 * @brief Implementation of comprehensive mathematics utility library for 2D game development
 *
 * Implements mathematical constants, functions, and collision detection for 2D games
 * Includes trigonometry, vector math, distance calculations, and collision systems
 */

#include "MathUtils.hpp"

namespace GP2Engine {
    // Constants
    const float MathUtils::PI = 3.14159265358979323846f;
    const float MathUtils::twoPI = 2.0f * PI;
    const float MathUtils::halfPI = PI / 2.0f;
    const float MathUtils::Deg_to_Rad = PI / 180.0f;
    const float MathUtils::Rad_to_Deg = 180.0f / PI;
    const float MathUtils::Epsilon = 1e-6f;

    // Angle conversion
    float MathUtils::DegToRad(float degrees) {
        return degrees * Deg_to_Rad;
    }

    float MathUtils::RadToDeg(float radians) {
        return radians * Rad_to_Deg;
    }

    // Radian trig functions
    float MathUtils::sinRad(float radians) {
        return std::sin(radians);
    }

    float MathUtils::cosRad(float radians) {
        return std::cos(radians);
    }

    float MathUtils::tanRad(float radians) {
        return std::tan(radians);
    }

    float MathUtils::ASinRad(float val) {
        return std::asin(val);
    }

    float MathUtils::ACosRad(float val) {
        return std::acos(val);
    }

    float MathUtils::AtanRad(float val) {
        return std::atan(val);
    }

    float MathUtils::ATan2Rad(float y, float x) {
        return std::atan2(y, x);
    }

    // Degree trig functions
    float MathUtils::sinDeg(float degrees) {
        return sinRad(DegToRad(degrees));
    }

    float MathUtils::cosDeg(float degrees) {
        return cosRad(DegToRad(degrees));
    }

    float MathUtils::tanDeg(float degrees) {
        return tanRad(DegToRad(degrees));
    }

    float MathUtils::ASinDeg(float val) {
        return RadToDeg(ASinRad(val));
    }

    float MathUtils::ACosDeg(float val) {
        return RadToDeg(ACosRad(val));
    }

    float MathUtils::ATanDeg(float val) {
        return RadToDeg(AtanRad(val));
    }

    float MathUtils::ATan2Deg(float y, float x) {
        return RadToDeg(ATan2Rad(y, x));
    }

    // Math utils
    bool MathUtils::IsPowof2(uint32_t x) {
        return x != 0 && (x & (x - 1)) == 0;
    }

    uint32_t MathUtils::NextPowOf2(uint32_t x) {
        // Bit manipulation to find next power of 2
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x++;
        return x;
    }

    uint32_t MathUtils::LogBase2(uint32_t x) {
        uint32_t result = 0;
        while (x >>= 1) result++;
        return result;
    }

    float MathUtils::Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    float MathUtils::Wrap(float value, float min, float max) {
        float range = max - min;
        if (range <= 0.0f) return min;

        float result = value;
        while (result < min) result += range;
        while (result > max) result -= range;
        return result;
    }

    float MathUtils::Min(float a, float b) {
        return (a < b) ? a : b;
    }

    float MathUtils::Max(float a, float b) {
        return (a > b) ? a : b;
    }

    bool MathUtils::InRange(float value, float min, float max) {
        return value >= min && value <= max;
    }

    float MathUtils::Lerp(float a, float b, float t) {
        return a + (b - a) * Clamp(t, 0.0f, 1.0f);
    }

    float MathUtils::Abs(float value) {
        return std::abs(value);
    }

    float MathUtils::Sqrt(float value) {
        return std::sqrt(value);
    }

    float MathUtils::Pow(float base, float exponent) {
        return std::pow(base, exponent);
    }

    // Distance calculation functions
    float MathUtils::DistPointToCircle(const Vector2D& point, const Vector2D& center, float radius) {
        float dist = (point - center).length();
        return dist - radius;
    }

    float MathUtils::DistPointToRect(const Vector2D& point, const Vector2D& rectCtr, float sizeX, float sizeY) {
        Vector2D halfSize(sizeX * 0.5f, sizeY * 0.5f);
        Vector2D delta = point - rectCtr;

        float dx = Abs(delta.x) - halfSize.x;
        float dy = Abs(delta.y) - halfSize.y;

        // Point is inside rectangle
        if (dx <= 0.0f && dy <= 0.0f) {
            return -Min(-dx, -dy); // penetration depth (negative)
        }

        // Point is outside rectangle
        float clampedDx = Max(dx, 0.0f);
        float clampedDy = Max(dy, 0.0f);
        return Sqrt(clampedDx * clampedDx + clampedDy * clampedDy);
    }


    float MathUtils::DistPointToLineSeg(const Vector2D& point, const Vector2D& line0, const Vector2D& line1) {
        Vector2D lineDir = line1 - line0;
        float lineLength = lineDir.length();

        if (lineLength < Epsilon) {
            return (point - line0).length();
        }

        Vector2D normalizedDir = lineDir / lineLength;
        Vector2D toPoint = point - line0;

        float t = toPoint.dot(normalizedDir);
        t = Clamp(t, 0.0f, lineLength);

        Vector2D closestPoint = line0 + normalizedDir * t;
        return (point - closestPoint).length();
    }

    float MathUtils::DistPointToConvexPoly(const Vector2D& pPos, const Vector2D* pVtx, uint32_t vtxNum) {
        if (vtxNum < 3) return FLT_MAX;

        float minDistance = FLT_MAX;
        bool inside = true;

        for (uint32_t i = 0; i < vtxNum; ++i) {
            uint32_t j = (i + 1) % vtxNum;
            const Vector2D& v0 = pVtx[i];
            const Vector2D& v1 = pVtx[j];

            // Check if point is to the right of the edge (outside)
            Vector2D edge = v1 - v0;
            Vector2D toPoint = pPos - v0;
            float cross = edge.cross(toPoint);

            if (cross < 0) {
                inside = false;
            }

            // Calculate distance to this edge
            float edgeDistance = DistPointToLineSeg(pPos, v0, v1);
            minDistance = Min(minDistance, edgeDistance);
        }

        return inside ? -minDistance : minDistance;
    }

    float MathUtils::DistCircToCirc(const Vector2D& center1, float radius1, const Vector2D& center2, float radius2) {
        float distance = (center1 - center2).length();
        return distance - (radius1 + radius2);
    }

    float MathUtils::DistCircToRect(const Vector2D& circCenter, float circRadius, const Vector2D& rectCenter, float sizeX, float sizeY) {
        Vector2D halfSize(sizeX * 0.5f, sizeY * 0.5f);
        Vector2D delta = circCenter - rectCenter;
        Vector2D closestPoint;

        closestPoint.x = Clamp(delta.x, -halfSize.x, halfSize.x);
        closestPoint.y = Clamp(delta.y, -halfSize.y, halfSize.y);

        Vector2D penetration = delta - closestPoint;
        float distance = penetration.length();

        return distance - circRadius;
    }

    float MathUtils::DistRectToRect(const Vector2D& rect0Center, float sizeX0, float sizeY0, const Vector2D& rect1Center, float sizeX1, float sizeY1) {
        Vector2D halfSize0(sizeX0 * 0.5f, sizeY0 * 0.5f);
        Vector2D halfSize1(sizeX1 * 0.5f, sizeY1 * 0.5f);
        Vector2D delta = rect1Center - rect0Center;

        // Calculate overlap on each axis
        float overlapX = halfSize0.x + halfSize1.x - Abs(delta.x);
        float overlapY = halfSize0.y + halfSize1.y - Abs(delta.y);

        if (overlapX < 0 || overlapY < 0) {
            // No collision, return separation distance
            return Max(overlapX, overlapY);
        }

        // Collision detected, return penetration depth (negative)
        return -Min(overlapX, overlapY);
    }

    // Collision test functions
    bool MathUtils::TestPointToCirc(const Vector2D& point, const Vector2D& center, float radius) {
        return (point - center).lengthSquare() <= (radius * radius);
    }

    bool MathUtils::TestPointToRect(const Vector2D& point, const Vector2D& rectCenter, float sizeX, float sizeY) {
        Vector2D halfSize(sizeX * 0.5f, sizeY * 0.5f);
        Vector2D delta = point - rectCenter;
        return Abs(delta.x) <= halfSize.x && Abs(delta.y) <= halfSize.y;
    }

    bool MathUtils::TestCircToCirc(const Vector2D& center0, float radius0, const Vector2D& center1, float radius1) {
        float combinedRadius = radius0 + radius1;
        return (center0 - center1).lengthSquare() <= (combinedRadius * combinedRadius);
    }

    bool MathUtils::TestCircToRect(const Vector2D& circCenter, float circRadius, const Vector2D& rectCenter, float sizeX, float sizeY) {
        Vector2D halfSize(sizeX * 0.5f, sizeY * 0.5f);
        Vector2D delta = circCenter - rectCenter;
        Vector2D closestPoint;

        closestPoint.x = Clamp(delta.x, -halfSize.x, halfSize.x);
        closestPoint.y = Clamp(delta.y, -halfSize.y, halfSize.y);

        Vector2D penetration = delta - closestPoint;
        return penetration.lengthSquare() <= (circRadius * circRadius);
    }

    bool MathUtils::TestRectToRect(const Vector2D& rect0Center, float sizeX0, float sizeY0, const Vector2D& rect1Center, float sizeX1, float sizeY1) {
        Vector2D halfSize0(sizeX0 * 0.5f, sizeY0 * 0.5f);
        Vector2D halfSize1(sizeX1 * 0.5f, sizeY1 * 0.5f);
        Vector2D delta = rect1Center - rect0Center;

        return Abs(delta.x) <= (halfSize0.x + halfSize1.x) &&
            Abs(delta.y) <= (halfSize0.y + halfSize1.y);
    }

    // Line segment struct implementation
    MathUtils::LineSeg2::LineSeg2() : start(), end(), normal() {}

    MathUtils::LineSeg2::LineSeg2(const Vector2D& s, const Vector2D& e) : start(s), end(e) {
        CalcNormal();
    }

    void MathUtils::LineSeg2::CalcNormal() {
        Vector2D dir = end - start;
        normal = Vector2D(-dir.y, dir.x).normalized();
    }

    float MathUtils::LineSeg2::Length() const {
        return (end - start).length();
    }

    Vector2D MathUtils::LineSeg2::Direction() const {
        return (end - start).normalized();
    }

    // Line segment functions
    float MathUtils::DistancePointToLineSegment(const Vector2D& point, const LineSeg2& line) {
        return DistPointToLineSeg(point, line.start, line.end);
    }

    bool MathUtils::TestPointToLineSegment(const Vector2D& point, const LineSeg2& line, float thickness) {
        return DistancePointToLineSegment(point, line) <= thickness;
    }

    float MathUtils::StaticPointToStaticLineSegment(const Vector2D& point, const LineSeg2& line) {
        return DistancePointToLineSegment(point, line);
    }

    float MathUtils::AnimatedPointToStaticLineSegment(const Vector2D& start, const Vector2D& end,
        const LineSeg2& line, Vector2D& intersection) {
        Vector2D movement = end - start;
        Vector2D lineDir = line.end - line.start;
        Vector2D lineNormal = line.normal;

        // Check if movement is parallel to line
        if (Abs(movement.dot(lineNormal)) < Epsilon) {
            return -1.0f; // No collision possible
        }

        // Calculate time of intersection using plane equation
        Vector2D toLineStart = line.start - start;
        float time = toLineStart.dot(lineNormal) / movement.dot(lineNormal);

        if (time < 0.0f || time > 1.0f) {
            return -1.0f; // Intersection not within movement range
        }

        // Calculate intersection point
        intersection = start + movement * time;

        // Check if intersection point is within line segment bounds
        Vector2D toIntersection = intersection - line.start;
        float projection = toIntersection.dot(lineDir) / lineDir.lengthSquare();

        if (projection < 0.0f || projection > 1.0f) {
            return -1.0f; // Intersection not within line segment
        }

        return time;
    }

    float MathUtils::AnimatedCircleToStaticLineSegment(const Vector2D& start, const Vector2D& end, float radius,
        const LineSeg2& line, Vector2D& intersection) {
        // Expand line by circle radius along normal
        Vector2D expandedLineStart = line.start - line.normal * radius;
        Vector2D expandedLineEnd = line.end - line.normal * radius;
        LineSeg2 expandedLine(expandedLineStart, expandedLineEnd);

        return AnimatedPointToStaticLineSegment(start, end, expandedLine, intersection);
    }

    // Reflection functions
    Vector2D MathUtils::ReflectPointOnLine(const Vector2D& point, const LineSeg2& line) {
        Vector2D lineDir = line.Direction();
        Vector2D toPoint = point - line.start;

        float projection = toPoint.dot(lineDir);
        Vector2D closestPoint = line.start + lineDir * projection;

        Vector2D reflection = point + (closestPoint - point) * 2.0f;
        return reflection;
    }

    Vector2D MathUtils::ReflectVectorOnNormal(const Vector2D& vector, const Vector2D& normal) {
        return vector - normal * 2.0f * vector.dot(normal);
    }

    float MathUtils::ReflectAnimatedPointOnStaticLineSegment(const Vector2D& start, const Vector2D& end,
        const LineSeg2& line, Vector2D& intersection,
        Vector2D& reflection) {
        float time = AnimatedPointToStaticLineSegment(start, end, line, intersection);

        if (time < 0.0f) return -1.0f;

        // Calculate reflection vector using line normal
        Vector2D incoming = end - start;
        reflection = ReflectVectorOnNormal(incoming, line.normal);

        return time;
    }

    float MathUtils::ReflectAnimatedCircleOnStaticLineSegment(const Vector2D& start, const Vector2D& end, float radius,
        const LineSeg2& line, Vector2D& intersection,
        Vector2D& reflection) {
        float time = AnimatedCircleToStaticLineSegment(start, end, radius, line, intersection);

        if (time < 0.0f) return -1.0f;

        Vector2D incoming = end - start;
        reflection = ReflectVectorOnNormal(incoming, line.normal);

        return time;
    }

    float MathUtils::AnimatedPointToStaticCircle(const Vector2D& start, const Vector2D& end,
        const Vector2D& circleCenter, float radius, Vector2D& intersection) {
        Vector2D movement = end - start;
        Vector2D toCircle = circleCenter - start;
        float movementLength = movement.length();

        if (movementLength < Epsilon) {
            return -1.0f; // No movement
        }

        Vector2D movementDir = movement / movementLength;
        float projection = toCircle.dot(movementDir);

        // Find closest point on movement trajectory to circle center
        Vector2D closestPoint = start + movementDir * projection;
        float distanceToCenter = (circleCenter - closestPoint).length();

        if (distanceToCenter > radius) {
            return -1.0f; // No collision
        }

        // Calculate intersection time using Pythagorean theorem
        float offset = Sqrt(radius * radius - distanceToCenter * distanceToCenter);
        float time = (projection - offset) / movementLength;

        if (time < 0.0f || time > 1.0f) {
            return -1.0f; // Intersection not within movement range
        }

        intersection = start + movement * time;
        return time;
    }

    float MathUtils::ReflectAnimatedPointOnStaticCircle(const Vector2D& start, const Vector2D& end,
        const Vector2D& circleCenter, float radius,
        Vector2D& intersection, Vector2D& reflection) {
        float time = AnimatedPointToStaticCircle(start, end, circleCenter, radius, intersection);

        if (time < 0.0f) return -1.0f;

        // Calculate reflection normal (radial direction from circle center)
        Vector2D normal = (intersection - circleCenter).normalized();
        Vector2D incoming = end - start;
        reflection = ReflectVectorOnNormal(incoming, normal);

        return time;
    }

    float MathUtils::AnimatedCircleToStaticCircle(const Vector2D& start0, const Vector2D& end0, float radius0,
        const Vector2D& circleCenter1, float radius1, Vector2D& intersection) {
        // Reduce to point-circle collision with combined radius
        float combinedRadius = radius0 + radius1;
        return AnimatedPointToStaticCircle(start0, end0, circleCenter1, combinedRadius, intersection);
    }

    float MathUtils::ReflectAnimatedCircleOnStaticCircle(const Vector2D& start0, const Vector2D& end0, float radius0,
        const Vector2D& circleCenter1, float radius1,
        Vector2D& intersection, Vector2D& reflection) {
        float time = AnimatedCircleToStaticCircle(start0, end0, radius0, circleCenter1, radius1, intersection);

        if (time < 0.0f) return -1.0f;

        // Calculate reflection normal (direction between centers at collision)
        Vector2D collisionCenter = intersection;
        Vector2D normal = (collisionCenter - circleCenter1).normalized();
        Vector2D incoming = end0 - start0;
        reflection = ReflectVectorOnNormal(incoming, normal);

        return time;
    }
}