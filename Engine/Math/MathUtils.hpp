/**
 * @file MathUtils.hpp
 * @author Rifqah (100%)
 * @brief Comprehensive mathematics utility library for 2D game development
 *
 * Provides mathematical constants, functions, and collision detection for 2D games
 * Includes trigonometry, vector math, distance calculations, and collision systems
 */

#pragma once

#include "Vector2D.hpp"
#include "Matrix3x3.hpp"
#include <cmath>
#include <cfloat>
#include <climits>
#include <cstdint>

namespace GP2Engine {

    /**
     * @brief Comprehensive mathematics utility library for 2D game development
     *
     * Provides mathematical constants, functions, and collision detection systems
     * optimized for 2D game development with both static and dynamic collision tests
     */
    struct MathUtils {

        /**
         * @brief Mathematical constants for game development
         */
        static const float PI;              // Pi constant for circular calculations
        static const float twoPI;           // 2 * Pi representing full rotation in radians
        static const float halfPI;          // Pi / 2 representing 90 degrees in radians
        static const float Deg_to_Rad;      // Conversion factor from degrees to radians
        static const float Rad_to_Deg;      // Conversion factor from radians to degrees
        static const float Epsilon;         // Small value for floating-point comparisons

        /**
         * @brief Angle conversion utilities
         */
        static float DegToRad(float degrees);       // Convert degrees to radians
        static float RadToDeg(float radians);       // Convert radians to degrees

        /**
         * @brief Trigonometric functions using radians
         */
        static float sinRad(float radians);         // Sine function (radians)
        static float cosRad(float radians);         // Cosine function (radians)
        static float tanRad(float radians);         // Tangent function (radians)
        static float ASinRad(float val);            // Arc sine returning radians
        static float ACosRad(float val);            // Arc cosine returning radians
        static float AtanRad(float val);            // Arc tangent returning radians
        static float ATan2Rad(float y, float x);    // Arc tangent of y/x returning radians

        /**
         * @brief Trigonometric functions using degrees
         */
        static float sinDeg(float degrees);         // Sine function (degrees)
        static float cosDeg(float degrees);         // Cosine function (degrees)
        static float tanDeg(float degrees);         // Tangent function (degrees)
        static float ASinDeg(float val);            // Arc sine returning degrees
        static float ACosDeg(float val);            // Arc cosine returning degrees
        static float ATanDeg(float val);            // Arc tangent returning degrees
        static float ATan2Deg(float y, float x);    // Arc tangent of y/x returning degrees

        /**
         * @brief Mathematical utility functions
         */
        static bool IsPowof2(uint32_t x);           // Check if number is power of 2
        static uint32_t NextPowOf2(uint32_t x);     // Find next power of 2 greater than or equal to x
        static uint32_t LogBase2(uint32_t x);       // Calculate log base 2 of integer
        static float Clamp(float val, float min, float max); // Clamp value between min and max
        static float Wrap(float val, float x0, float x1);    // Wrap value within range [x0, x1]
        static float Min(float x, float y);         // Return minimum of two values
        static float Max(float x, float y);         // Return maximum of two values
        static bool InRange(float x, float x0, float x1);    // Check if value is in range [x0, x1]
        static float Lerp(float a, float b, float c);       // Linear interpolation between a and b
        static float Abs(float val);                // Absolute value
        static float Sqrt(float val);               // Square root
        static float Pow(float base, float exponent); // Power function

        /**
         * @brief Distance calculation functions between geometric shapes
         */
        static float DistPointToCircle(const Vector2D& point, const Vector2D& center, float radius); // Distance from point to circle edge
        static float DistPointToRect(const Vector2D& point, const Vector2D& rectCtr, float sizeX, float sizeY); // Distance from point to rectangle edge
        static float DistPointToLineSeg(const Vector2D& point, const Vector2D& line0, const Vector2D& line1); // Distance from point to line segment
        static float DistPointToConvexPoly(const Vector2D& pPos, const Vector2D* pVtx, uint32_t vtxNum); // Distance from point to convex polygon
        static float DistCircToCirc(const Vector2D& center1, float radius1, const Vector2D& center2, float radius2); // Distance between two circles
        static float DistCircToRect(const Vector2D& circCenter, float circRadius, const Vector2D& rectCenter, float sizeX, float sizeY); // Distance between circle and rectangle
        static float DistRectToRect(const Vector2D& rect0Center, float sizeX0, float sizeY0, const Vector2D& rect1Center, float sizeX1, float sizeY1); // Distance between two rectangles

        /**
         * @brief Collision detection functions for geometric shapes
         */
        static bool TestPointToCirc(const Vector2D& point, const Vector2D& center, float radius); // Test if point is inside circle
        static bool TestPointToRect(const Vector2D& point, const Vector2D& rectCenter, float sizeX, float sizeY); // Test if point is inside rectangle
        static bool TestCircToCirc(const Vector2D& center0, float radius0, const Vector2D& center1, float radius1); // Test circle-circle collision
        static bool TestCircToRect(const Vector2D& circCenter, float circRadius, const Vector2D& rectCenter, float sizeX, float sizeY); // Test circle-rectangle collision
        static bool TestRectToRect(const Vector2D& rect0Center, float sizeX0, float sizeY0, const Vector2D& rect1Center, float sizeX1, float sizeY1); // Test rectangle-rectangle collision

        /**
         * @brief Line segment representation for collision and reflection systems
         */
        struct LineSeg2 {
            Vector2D start;      // Start point of line segment
            Vector2D end;        // End point of line segment
            Vector2D normal;     // Normal vector (perpendicular to line direction)

            LineSeg2();                             // Default constructor
            LineSeg2(const Vector2D& s, const Vector2D& e); // Constructor with start and end points

            void CalcNormal();                      // Calculate normal vector from line direction
            float Length() const;                   // Calculate segment length
            Vector2D Direction() const;              // Calculate normalized direction vector
        };

        /**
         * @brief Line segment collision and distance functions
         */
        static float DistancePointToLineSegment(const Vector2D& point, const LineSeg2& line); // Distance from point to line segment
        static bool TestPointToLineSegment(const Vector2D& point, const LineSeg2& line, float thickness); // Test if point is near line segment
        static float StaticPointToStaticLineSegment(const Vector2D& point, const LineSeg2& line); // Distance from static point to static line
        static float AnimatedPointToStaticLineSegment(const Vector2D& start, const Vector2D& end, const LineSeg2& line, Vector2D& intersection); // Moving point vs static line collision
        static float AnimatedCircleToStaticLineSegment(const Vector2D& start, const Vector2D& end, float radius, const LineSeg2& line, Vector2D& intersection); // Moving circle vs static line collision

        /**
         * @brief Reflection and collision response functions
         */
        static Vector2D ReflectPointOnLine(const Vector2D& point, const LineSeg2& line); // Reflect point over line
        static Vector2D ReflectVectorOnNormal(const Vector2D& vector, const Vector2D& normal); // Reflect vector over normal
        static float ReflectAnimatedPointOnStaticLineSegment(const Vector2D& start, const Vector2D& end, const LineSeg2& line, Vector2D& intersection, Vector2D& reflection); // Reflect moving point off line
        static float ReflectAnimatedCircleOnStaticLineSegment(const Vector2D& start, const Vector2D& end, float radius, const LineSeg2& line, Vector2D& intersection, Vector2D& reflection); // Reflect moving circle off line
        static float AnimatedPointToStaticCircle(const Vector2D& start, const Vector2D& end, const Vector2D& circleCenter, float radius, Vector2D& intersection); // Moving point vs static circle collision
        static float ReflectAnimatedPointOnStaticCircle(const Vector2D& start, const Vector2D& end, const Vector2D& circleCenter, float radius, Vector2D& intersection, Vector2D& reflection); // Reflect moving point off circle
        static float AnimatedCircleToStaticCircle(const Vector2D& start0, const Vector2D& end0, float radius0, const Vector2D& circleCenter1, float radius1, Vector2D& intersection); // Moving circle vs static circle collision
        static float ReflectAnimatedCircleOnStaticCircle(const Vector2D& start0, const Vector2D& end0, float radius0, const Vector2D& circleCenter1, float radius1, Vector2D& intersection, Vector2D& reflection); // Reflect moving circle off circle
    };
}