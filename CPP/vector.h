# pragma once

// =================================================================================================
// Vector math for 3D rendering

#include <math.h>
#include <string.h>

class CVector {

    public:
        struct {
            float   x, y, z, w;
        } m_data;

        CVector () {
            m_data.x = 0.0f;
            m_data.y = 0.0f;
            m_data.z = 0.0f;
            m_data.w = 0.0f;
        }


        CVector(float x, float y, float z, float w = 0.0f) {
            m_data.x = x;
            m_data.y = y;
            m_data.z = z;
            m_data.w = w;
        }


        inline float& X(void) {
            return m_data.x;
        }

        inline float& Y(void) {
            return m_data.y;
        }

        inline float& Z(void) {
            return m_data.z;
        }

        inline float& W(void) {
            return m_data.w;
        }

        inline CVector& operator= (const CVector& other) {
            if (this != &other)
                memcpy(&m_data, &other.m_data, sizeof(m_data));
            return *this;
        }

        inline CVector& operator+= (const CVector& other) {
            m_data.x += other.m_data.x;
            m_data.y += other.m_data.y;
            m_data.z += other.m_data.z;
            m_data.w += other.m_data.w;
            return *this;
        }


        inline CVector& operator-= (CVector const& other) {
            m_data.x -= other.m_data.x;
            m_data.y -= other.m_data.y;
            m_data.z -= other.m_data.z;
            m_data.w -= other.m_data.w;
            return *this;
        }


        inline CVector operator*= (float const& n) {
            m_data.x *= n, m_data.y *= n, m_data.z *= n;
            return *this;
        }


        inline CVector operator/= (float const& n) {
            m_data.x /= n, m_data.y /= n, m_data.z /= n;
            return *this;
        }


        inline CVector operator+ (const CVector& other) const {
            return CVector(m_data.x + other.m_data.x, m_data.y + other.m_data.y, m_data.z + other.m_data.z);
        }


        inline CVector operator- (const CVector& other) const {
            return CVector(m_data.x - other.m_data.x, m_data.y - other.m_data.y, m_data.z - other.m_data.z);
        }


        inline CVector operator* (const float n) const {
            return CVector(m_data.x * n, m_data.y * n, m_data.z * n);
        }


        inline CVector operator/ (const float n) const {
            return CVector(m_data.x / n, m_data.y / n, m_data.z / n);
        }


        inline CVector operator- (void) {
            return CVector(-m_data.x, -m_data.y, -m_data.z);
        }


        inline const bool operator== (const CVector& other) const {
            return (m_data.x == other.m_data.x) && (m_data.y == other.m_data.y) && (m_data.z == other.m_data.z);
        }


        inline const bool operator!= (const CVector& other) const {
            return (m_data.x != other.m_data.x) || (m_data.y != other.m_data.y) || (m_data.z != other.m_data.z);
        }

        inline float Dot(CVector const& other) const {
            return m_data.x * other.m_data.x + m_data.y * other.m_data.y + m_data.z * other.m_data.z;
        }


        inline const CVector Cross(const CVector& other) const {
            return CVector(m_data.y * other.m_data.z - m_data.z * other.m_data.y, 
                           m_data.z * other.m_data.x - m_data.x * other.m_data.z, 
                           m_data.x * other.m_data.y - m_data.y * other.m_data.x);
        }


        inline const float Len(void) {
            return float(sqrt(this->Dot(*this)));
        }


        inline CVector& Normalize(void) {
            float l = this->Len();
            if ((l != 0.0) && (l != 1.0))
                *this /= l;
            return *this;
        }

        inline const float Min(void) const {
            return (m_data.x < m_data.y) ? (m_data.x < m_data.z) ? m_data.x : m_data.z : (m_data.y < m_data.z) ? m_data.y : m_data.z;
        }


        inline const float Max(void) const {
            return (m_data.x > m_data.y) ? (m_data.x > m_data.z) ? m_data.x : m_data.z : (m_data.y > m_data.z) ? m_data.y : m_data.z;
        }

        
        inline const CVector& Minimize(CVector& other) {
            if (m_data.x > other.m_data.x) m_data.x = other.m_data.x;
            if (m_data.y > other.m_data.y) m_data.y = other.m_data.y;
            if (m_data.z > other.m_data.z) m_data.z = other.m_data.z;
            return *this;
        }


        inline const CVector& Maximize(CVector& other) {
            if (m_data.x < other.m_data.x) m_data.x = other.m_data.x;
            if (m_data.y < other.m_data.y) m_data.y = other.m_data.y;
            if (m_data.z < other.m_data.z) m_data.z = other.m_data.z;
            return *this;
        }


        inline const bool IsValid(void) const {
            return (m_data.x == m_data.x) && (m_data.y == m_data.y) && (m_data.z == m_data.z);   // if any component is NaN, check for equality with itself should return false
        }

        static inline CVector Perp(const CVector& v0, const CVector& v1, const CVector& v2) {
            return (v1 - v0).Cross(v2 - v1);
        }

        static inline CVector Normal(CVector& v0, CVector& v1, CVector& v2) {
            return Perp(v0, v1, v2).Normalize();
        }

        static int Compare(CVector& v0, CVector& v1) {
            if (v0.m_data.x < v1.m_data.x)
                return -1;
            if (v0.m_data.x > v1.m_data.x)
                return 1;
            if (v0.m_data.y < v1.m_data.y)
                return -1;
            if (v0.m_data.y > v1.m_data.y)
                return 1;
            if (v0.m_data.z < v1.m_data.z)
                return -1;
            if (v0.m_data.z > v1.m_data.z)
                return 1;
            return 0;
        }

    };

// =================================================================================================
