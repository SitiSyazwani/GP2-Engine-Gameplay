/**
 * @file Vector2D.hpp
 * @author Rifqah (100%)
 * @brief 2D vector mathematics library for game development
 *
 * Provides comprehensive 2D vector operations including arithmetic,
 * geometric calculations, and utility functions for game development
 */

#pragma once

namespace GP2Engine {

    /**
     * @brief 2D vector class for mathematics and geometry operations
     *
     * Implements comprehensive 2D vector operations including arithmetic,
     * geometric calculations, normalization, and interpolation functions
     * Essential for position, velocity, direction, and force calculations
     */
    struct Vector2D {
        float x, y;

        /**
         * @brief Vector constructors
         */
        Vector2D();                               // Default constructor initializes to zero
        Vector2D(float x, float y);               // Constructor with x and y components

        /**
         * @brief Static vector constants
         */
        static const Vector2D zero;    // Zero vector (0, 0) for origin references
        static const Vector2D unitX;   // Unit vector in X direction (1, 0) for right direction
        static const Vector2D unitY;   // Unit vector in Y direction (0, 1) for up direction

        /**
         * @brief Vector arithmetic operators
         */
        Vector2D operator+(const Vector2D& other) const;    // Vector addition
        Vector2D operator-(const Vector2D& other) const;    // Vector subtraction
        Vector2D& operator+=(const Vector2D& other);        // Compound vector addition
        Vector2D& operator-=(const Vector2D& other);        // Compound vector subtraction
        Vector2D& operator*=(const Vector2D& other);        // Compound component-wise multiplication
        Vector2D& operator/=(const Vector2D& other);        // Compound component-wise division
        Vector2D operator*(float scalar) const;            // Scalar multiplication
        Vector2D operator/(float scalar) const;            // Scalar division
        Vector2D& operator*=(float scalar);                // Compound scalar multiplication
        Vector2D& operator/=(float scalar);                // Compound scalar division
        bool operator==(const Vector2D& other) const;      // Equality comparison with epsilon tolerance
        bool operator!=(const Vector2D& other) const;      // Inequality comparison

        /**
         * @brief Vector geometric operations
         */
        float length() const;                 // Calculate vector magnitude (Euclidean length)
        float lengthSquare() const;           // Calculate squared magnitude (faster for comparisons)
        void normalize();                     // Normalize this vector to unit length in-place
        Vector2D normalized() const;           // Return normalized copy of vector
        float dot(const Vector2D& other) const;// Dot product with another vector
        float cross(const Vector2D& other) const;// 2D cross product (scalar result representing z-component)

        /**
         * @brief Convenience methods for common vector operations
         */
        Vector2D Scale(float scalar) const;   // Return scaled copy of vector
        Vector2D Add(const Vector2D& other) const;    // Return sum of vectors
        Vector2D Subtract(const Vector2D& other) const; // Return difference of vectors
        Vector2D Normalize() const;          // Return normalized copy (capital N for consistency)

        /**
         * @brief Static utility functions for vector mathematics
         */
        static float distance(const Vector2D& a, const Vector2D& b);                // Euclidean distance between two points
        static float distanceSquared(const Vector2D& a, const Vector2D& b);         // Squared distance (faster computation)
        static Vector2D linearInterpolation(const Vector2D& a, const Vector2D& b, float t);// Linear interpolation between vectors
    };

    /**
     * @brief Non-member scalar multiplication operator
     */
    Vector2D operator*(float scalar, const Vector2D& vector); // Scalar multiplication (scalar first)
}