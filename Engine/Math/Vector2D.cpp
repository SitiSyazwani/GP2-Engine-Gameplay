/**
 * @file Vector2D.cpp
 * @author Rifqah (100%)
 * @brief 2D vector mathematics library for game development
 *
 * Implement comprehensive 2D vector operations including arithmetic,
 * geometric calculations, and utility functions for game development
 */

#include "Vector2D.hpp"
#include <cmath>

namespace GP2Engine {
    // Static constants
    const Vector2D Vector2D::zero(0.0f, 0.0f);      // Zero vector constant
    const Vector2D Vector2D::unitX(1.0f, 0.0f);     // Unit vector in X direction
    const Vector2D Vector2D::unitY(0.0f, 1.0f);     // Unit vector in Y direction



    // Constructors
    Vector2D::Vector2D() : x(0.0f), y(0.0f) {}      // Default constructor (initializes to zero)
    Vector2D::Vector2D(float x, float y) : x(x), y(y) {} // Parameterized constructor



    // Operator overloads
    Vector2D Vector2D::operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);  // Component-wise addition
    }

    Vector2D Vector2D::operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);  // Component-wise subtraction
    }

    Vector2D& Vector2D::operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;                // In-place component-wise addition
        return *this;
    }

    Vector2D& Vector2D::operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;                // In-place component-wise subtraction
        return *this;
    }

    Vector2D& Vector2D::operator*=(const Vector2D& other) {
        x *= other.x;
        y *= other.y;                // In-place component-wise multiplication
        return *this;
    }

    Vector2D& Vector2D::operator/=(const Vector2D& other) {
        x /= other.x;
        y /= other.y;                // In-place component-wise division
        return *this;
    }

    Vector2D Vector2D::operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);    // Scalar multiplication
    }

    Vector2D Vector2D::operator/(float scalar) const {
        // Avoid division by zero - return unchanged vector if scalar is zero
        if (scalar == 0.0f) return *this;
        return Vector2D(x / scalar, y / scalar);    // Scalar division
    }

    Vector2D& Vector2D::operator*=(float scalar) {
        x *= scalar;
        y *= scalar;                  // In-place scalar multiplication
        return *this;
    }

    Vector2D& Vector2D::operator/=(float scalar) {
        // Avoid division by zero - only divide if scalar is not zero
        if (scalar != 0.0f) {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    bool Vector2D::operator==(const Vector2D& other) const {
        // Floating-point comparison with epsilon tolerance
        const float epsilon = 1e-5f;
        return (std::abs(x - other.x) < epsilon) &&
            (std::abs(y - other.y) < epsilon);
    }

    bool Vector2D::operator!=(const Vector2D& other) const {
        return !(*this == other);                  // Inequality is negation of equality
    }



    // Member functions
    float Vector2D::length() const {
        return std::sqrt(x * x + y * y);           // Euclidean magnitude (Pythagorean theorem)
    }

    float Vector2D::lengthSquare() const {
        return (x * x + y * y);                    // Squared magnitude (avoids sqrt, useful for comparisons)
    }

    void Vector2D::normalize() {
        float len = length();                      // Convert to unit vector (length = 1)
        if (len > 0.0f) {
            x /= len;
            y /= len;
        }    // Only normalize if length is not zero
    }

    Vector2D Vector2D::normalized() const {
        Vector2D result = *this;                    // Returns normalized copy without modifying original
        result.normalize();
        return result;
    }

    float Vector2D::dot(const Vector2D& other) const {
        return (x * other.x + y * other.y);        // Dot product (scalar result)
    }

    float Vector2D::cross(const Vector2D& other) const {
        return (x * other.y - y * other.x);        // 2D cross product (scalar result representing z-component)
    }

    // Convenience methods for common operations
    Vector2D Vector2D::Scale(float scalar) const {
        return Vector2D(x * scalar, y * scalar);   // Return scaled copy of vector
    }

    Vector2D Vector2D::Add(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y); // Return sum of vectors
    }

    Vector2D Vector2D::Subtract(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y); // Return difference of vectors
    }

    Vector2D Vector2D::Normalize() const {
        return normalized(); // Return normalized copy (capital N for consistency)
    }



    // Static utility functions
    float Vector2D::distance(const Vector2D& a, const Vector2D& b) {
        Vector2D diff = a - b;                      // Euclidean distance between two points
        return diff.length();
    }

    float Vector2D::distanceSquared(const Vector2D& a, const Vector2D& b) {
        float dx = b.x - a.x;                      // Squared distance (faster, avoids sqrt)
        float dy = b.y - a.y;
        return (dx * dx + dy * dy);
    }

    Vector2D Vector2D::linearInterpolation(const Vector2D& a, const Vector2D& b, float t) {
        // Clamp interpolation parameter t between 0 and 1
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
        return Vector2D(a.x + (b.x - a.x) * t,      // Linear interpolation between two vectors
            a.y + (b.y - a.y) * t);
    }



    // Non-member scalar multiplication (allows scalar * vector syntax)
    Vector2D operator*(float scalar, const Vector2D& vector) {
        return vector * scalar;                    // Commutative property of scalar multiplication
    }
}
