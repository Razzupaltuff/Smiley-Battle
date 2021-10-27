#pragma once

// =================================================================================================
// Texture coordinate representation

class CTexCoord {
    public:
        float   m_u, m_v;

        CTexCoord(float u = 0.0f, float v = 0.0f) : m_u(u), m_v(v) {}

        CTexCoord& operator= (CTexCoord const& other) {
            m_u = other.m_u;
            m_v = other.m_v;
            return *this;
        }
 
        CTexCoord operator+ (CTexCoord const& other) {
            return CTexCoord(m_u + other.m_u, m_v + other.m_v);
        }

        CTexCoord operator- (CTexCoord const& other) {
            return CTexCoord(m_u - other.m_u, m_v - other.m_v);
        }

        CTexCoord& operator+= (CTexCoord const& other) {
            m_u += other.m_u;
            m_v += other.m_v;
            return *this;
        }

        CTexCoord& operator-= (CTexCoord const& other) {
            m_u -= other.m_u;
            m_v -= other.m_v;
            return *this;
        }

        CTexCoord& operator-() {
            m_u = -m_u;
            m_v = -m_v;
            return *this;
        }

        CTexCoord& operator* (int n) {
            m_u *= n;
            m_v *= n;
            return *this;
        }
		
};

// =================================================================================================
