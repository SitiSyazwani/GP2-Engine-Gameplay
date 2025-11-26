/**
 * @file Matrix3x3.cpp
 * @author Rifqah (100%)
 * @brief 3x3 transformation matrix for 2D graphics and physics
 *
 * Implements comprehensive 2D transformation capabilities including translation,
 * rotation, scaling, and matrix operations optimized for game development
 */

#include "Matrix3x3.hpp"
#include <cmath>
#include <cstring>

namespace GP2Engine {

    // Static constant
    const Matrix3x3 Matrix3x3::Identity = Matrix3x3(1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f);  // Identity matrix (no transformation)

    // Constructors
    Matrix3x3::Matrix3x3() {
        memcpy(m, Identity.m, sizeof(m));  // Initialize as identity matrix
    }

    Matrix3x3::Matrix3x3(const float elements[9]) {
        memcpy(m, elements, sizeof(m));    // Copy from array
    }

    Matrix3x3::Matrix3x3(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22) {
        // Column-major ordering: [col0, col1, col2]
        m[0] = m00; m[3] = m01; m[6] = m02;
        m[1] = m10; m[4] = m11; m[7] = m12;
        m[2] = m20; m[5] = m21; m[8] = m22;
    }

    // Static factory methods
    Matrix3x3 Matrix3x3::CreateIdentity() {
        return Matrix3x3::Identity;
    }

    Matrix3x3 Matrix3x3::CreateTranslation(const Vector2D& translation) {
        return CreateTranslation(translation.x, translation.y);
    }

    Matrix3x3 Matrix3x3::CreateTranslation(float x, float y) {
        return Matrix3x3(1.0f, 0.0f, x,
            0.0f, 1.0f, y,
            0.0f, 0.0f, 1.0f);
    }
    Matrix3x3 Matrix3x3::CreateRotation(float radians) {
        float a = std::cos(radians);  // Cosine component
        float b = std::sin(radians);  // Sine component
        return Matrix3x3(a, -b, 0.0f,  // Rotation matrix (preserves z=1 for 2D)
            b, a, 0.0f,
            0.0f, 0.0f, 1.0f);
    }

    Matrix3x3 Matrix3x3::CreateRotationDeg(float degrees) {
        float radians = degrees * (3.14159265358979323846f / 180.0f);  // Convert degrees to radians
        return CreateRotation(radians);
    }

    Matrix3x3 Matrix3x3::CreateScale(const Vector2D& scale) {
        return CreateScale(scale.x, scale.y);
    }

    Matrix3x3 Matrix3x3::CreateScale(float x, float y) {
        return Matrix3x3(x, 0.0f, 0.0f,
            0.0f, y, 0.0f,
            0.0f, 0.0f, 1.0f);
    }

    Matrix3x3 Matrix3x3::CreateScale(float uniformScale) {
        return CreateScale(uniformScale, uniformScale);
    }


    // Operator overloads
    Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other)const {
        Matrix3x3 result;
        // Standard 3x3 matrix multiplication: row * column
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                float sum = 0.0f;
                for (int i = 0; i < 3; i++) {
                    sum += GetElement(row, i) * other.GetElement(i, col);
                }
                result.SetElement(row, col, sum);
            }
        }
        return result;
    }

    Vector2D Matrix3x3::operator*(const Vector2D& vector)const {
        // Transform 2D point (homogeneous coordinates: z=1 implied)
        float x = m[0] * vector.x + m[3] * vector.y + m[6];  // x' = m00*x + m01*y + m02
        float y = m[1] * vector.x + m[4] * vector.y + m[7];  // y' = m10*x + m11*y + m12
        return Vector2D(x, y);
    }

    Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other) {
        *this = other * (*this);  // Apply other's transform after this (right multiplication)
        return *this;
    }

    bool Matrix3x3::operator==(const Matrix3x3& other)const {
        const float epsilon = 1e-5f;  // Tolerance for floating-point comparison
        for (int i = 0; i < 9; i++) {
            if (std::abs(m[i] - other.m[i]) > epsilon) {
                return false;
            }
        }
        return true;
    }

    bool Matrix3x3::operator!=(const Matrix3x3& other)const {
        return !(*this == other);
    }

    // Matrix operations
    Matrix3x3 Matrix3x3::Transposed() const {
        Matrix3x3 result;
        // Swap rows and columns
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result.SetElement(row, col, GetElement(col, row));
            }
        }
        return result;
    }

    void Matrix3x3::Transpose() {
        *this = this->Transposed();
    }

    Matrix3x3 Matrix3x3::Inverted() const {
        Matrix3x3 result = *this;
        result.Invert();
        return result;
    }

    bool Matrix3x3::Invert() {
        float det = Determinant();
        // Check if matrix is singular (non-invertible)
        if (std::abs(det) < 1e-10f) {
            return false;
        }

        float invDet = 1.0f / det;  // Precompute reciprocal of determinant

        // Calculate inverse using adjugate matrix formula for 3x3
        Matrix3x3 temp;
        temp.m[0] = (m[4] * m[8] - m[5] * m[7]) * invDet;  // Cofactor for (0,0)
        temp.m[3] = (m[5] * m[6] - m[3] * m[8]) * invDet;  // Cofactor for (0,1)
        temp.m[6] = (m[3] * m[7] - m[6] * m[4]) * invDet;  // Cofactor for (0,2)

        temp.m[1] = (m[7] * m[2] - m[1] * m[8]) * invDet;  // Cofactor for (1,0)
        temp.m[4] = (m[0] * m[8] - m[6] * m[2]) * invDet;  // Cofactor for (1,1)
        temp.m[7] = (m[6] * m[1] - m[0] * m[7]) * invDet;  // Cofactor for (1,2)

        temp.m[2] = (m[1] * m[5] - m[4] * m[2]) * invDet;  // Cofactor for (2,0)
        temp.m[5] = (m[3] * m[2] - m[0] * m[5]) * invDet;  // Cofactor for (2,1)
        temp.m[8] = (m[0] * m[4] - m[3] * m[1]) * invDet;  // Cofactor for (2,2)

        *this = temp;
        return true;
    }

    // Transformation modifiers
    void Matrix3x3::Translate(const Vector2D& translation) {
        *this = CreateTranslation(translation) * (*this);
    }

    void Matrix3x3::Translate(float x, float y) {
        Translate(Vector2D(x, y));
    }

    void Matrix3x3::Rotate(float radians) {
        *this = CreateRotation(radians) * (*this);
    }

    void Matrix3x3::RotateDeg(float degrees) {
        Rotate(degrees * (3.14159265358979323846f / 180.0f));
    }

    void Matrix3x3::Scale(const Vector2D& scale) {
        *this = CreateScale(scale) * (*this);
    }

    void Matrix3x3::Scale(float x, float y) {
        Scale(Vector2D(x, y));
    }

    void Matrix3x3::Scale(float uniformScale) {
        Scale(Vector2D(uniformScale, uniformScale));
    }

    float Matrix3x3::Determinant()const {
        // Laplace expansion for 3x3 determinant
        return m[0] * (m[4] * m[8] - m[5] * m[7]) -  // a(ei - fh)
            m[3] * (m[1] * m[8] - m[2] * m[7]) +  // b(di - fg)
            m[6] * (m[1] * m[5] - m[2] * m[4]);   // c(dh - eg)
    }

    // Utility functions
    Vector2D Matrix3x3::GetTranslation() const {
        return Vector2D(m[6], m[7]);
    }

    Vector2D Matrix3x3::GetScale() const {
        // Extract scale from first two columns (ignoring rotation)
        float scaleX = Vector2D(m[0], m[1]).length();  // Length of first basis vector
        float scaleY = Vector2D(m[3], m[4]).length();  // Length of second basis vector
        return Vector2D(scaleX, scaleY);
    }

    float Matrix3x3::GetRotation() const {
        // Extract rotation angle from first column (atan2(y, x) of first basis vector)
        return std::atan2(m[1], m[0]);
    }

    // Element access
    float Matrix3x3::GetElement(int row, int col) const {
        return m[col * 3 + row];  // Column-major indexing
    }

    void Matrix3x3::SetElement(int row, int col, float value) {
        m[col * 3 + row] = value;  // Column-major indexing
    }

    // Non-member function
    Matrix3x3 operator*(float scalar, const Matrix3x3& matrix) {
        Matrix3x3 result;
        // Scalar multiplication of all matrix elements
        for (int i = 0; i < 9; ++i) {
            result.m[i] = matrix.m[i] * scalar;
        }
        return result;
    }
}
