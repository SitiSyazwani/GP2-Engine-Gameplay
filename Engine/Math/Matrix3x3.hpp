/**
 * @file Matrix3x3.hpp
 * @author Rifqah (100%)
 * @brief 3x3 transformation matrix for 2D graphics and physics
 *
 * Provides comprehensive 2D transformation capabilities including translation,
 * rotation, scaling, and matrix operations optimized for game development
 */

#pragma once

#include "Vector2D.hpp"
#include <cmath>

namespace GP2Engine {

    /**
     * @brief 3x3 transformation matrix for 2D graphics operations
     *
     * Implements column-major 3x3 matrices for efficient 2D transformations
     * including translation, rotation, scaling, and composite transformations
     * Essential for 2D sprite manipulation, camera systems, and physics
     */
    struct Matrix3x3 {

        /**
         * @brief Matrix data storage in column-major format
         *
         * Layout:
         * m[0] m[3] m[6]   // Col0, Col1, Col2
         * m[1] m[4] m[7]
         * m[2] m[5] m[8]
         * Third column holds translation components (x, y, 1)
         */
        float m[9];                                // 9-element array representing 3x3 matrix

        Matrix3x3();                               // Default constructor initializes as identity
        Matrix3x3(const float elements[9]);        // Constructor from float array
        Matrix3x3(float m00, float m01, float m02, // Constructor from individual elements
            float m10, float m11, float m12,
            float m20, float m21, float m22);

        /**
         * @brief Static factory methods for common transformations
         */
        static Matrix3x3 CreateIdentity();              // Creates identity matrix (no transformation)
        static Matrix3x3 CreateTranslation(const Vector2D& translation); // Creates translation matrix from Vector2D
        static Matrix3x3 CreateTranslation(float x, float y); // Creates translation matrix from coordinates
        static Matrix3x3 CreateRotation(float radians); // Creates rotation matrix (radians)
        static Matrix3x3 CreateRotationDeg(float degrees); // Creates rotation matrix (degrees)
        static Matrix3x3 CreateScale(const Vector2D& scale); // Creates scaling matrix from Vector2D
        static Matrix3x3 CreateScale(float x, float y);  // Creates scaling matrix from coordinates
        static Matrix3x3 CreateScale(float uniformScale); // Creates uniform scaling matrix

        /**
         * @brief Matrix operator overloads
         */
        Matrix3x3 operator*(const Matrix3x3& other)const; // Matrix multiplication
        Vector2D operator*(const Vector2D& vector)const;   // Transform vector by matrix
        Matrix3x3& operator*=(const Matrix3x3& other);   // In-place matrix multiplication
        bool operator==(const Matrix3x3& other)const;    // Equality comparison with epsilon tolerance
        bool operator!=(const Matrix3x3& other)const;    // Inequality comparison

        /**
         * @brief Fundamental matrix operations
         */
        Matrix3x3 Transposed() const;                   // Returns transposed matrix
        void Transpose();                               // Transposes this matrix in-place
        Matrix3x3 Inverted()const;                      // Returns inverted matrix
        bool Invert();                                  // Inverts this matrix in-place
        float Determinant()const;                       // Calculates matrix determinant

        /**
         * @brief Transformation modification methods
         */
        void Translate(const Vector2D& translation);     // Applies translation to this matrix
        void Translate(float x, float y);               // Applies translation (x,y) to this matrix
        void Rotate(float radians);                     // Applies rotation (radians) to this matrix
        void RotateDeg(float degrees);                  // Applies rotation (degrees) to this matrix
        void Scale(const Vector2D& scale);               // Applies scaling to this matrix
        void Scale(float x, float y);                   // Applies scaling (x,y) to this matrix
        void Scale(float uniformScale);                 // Applies uniform scaling to this matrix

        /**
         * @brief Matrix decomposition utilities
         */
        Vector2D GetTranslation()const;                  // Extracts translation component as Vector2D
        Vector2D GetScale()const;                       // Extracts scaling component as Vector2D
        float GetRotation()const;                      // Extracts rotation angle in radians

        /**
         * @brief Element access methods with row/column indexing
         */
        float GetElement(int row, int col)const;       // Gets element at (row, col)
        void SetElement(int row, int col, float val);  // Sets element at (row, col)

        /**
         * @brief Matrix constants
         */
        static const Matrix3x3 Identity;               // Identity matrix constant

    };

    /**
     * @brief Non-member scalar multiplication operator
     */
    Matrix3x3 operator*(float scalar, const Matrix3x3& matrix); // Scalar multiplication
}