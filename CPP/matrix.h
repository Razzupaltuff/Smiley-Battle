# pragma once

#include <string.h>
//#include <stdlib.h>
#include <stdint.h>
#include "vector.h"

// =================================================================================================
// Matrix math.Just the usual 3D stuff.

class CMatrix {

    public:
        CVector m_data[4];
        float   m_array[16];
        static const CMatrix IDENTITY;

        CMatrix () {
            m_data [0] = CVector (1.0f, 0.0f, 0.0f, 0.0f);
            m_data [1] = CVector (0.0f, 1.0f, 0.0f, 0.0f);
            m_data [2] = CVector (0.0f, 0.0f, 1.0f, 0.0f);
            m_data [3] = CVector (0.0f, 0.0f, 0.0f, 1.0f);
        }

        CMatrix(CVector r, CVector u, CVector f, CVector t = CVector (0.0f, 0.0f, 0.0f, 1.0f)) {
            m_data[0] = r;
            m_data[1] = u;
            m_data[2] = f;
            m_data[3] = t;
        }

        CMatrix& Compute(float sinX, float cosX, float sinY, float cosY, float sinZ, float cosZ);

        inline CVector& operator[] (const uint32_t i) {
            return m_data[i];
        }

        inline CMatrix& operator= (const CMatrix& other) {
            if (this != &other)
                for (int i = 0; i < 4; i++)
                    m_data[i] = other.m_data[i];
                //memcpy(m_data, other.m_data, sizeof(m_data));
            return *this;
        }

        inline CMatrix Identity(void) {
            return CMatrix(CVector(1.0, 0.0, 0.0, 0.0), CVector(0.0, 1.0, 0.0, 0.0), CVector(0.0, 0.0, 1.0, 0.0), CVector(0.0, 0.0, 0.0, 1.0));
        }

        inline CVector& R(void) {
            return m_data[0];
        }

        inline CVector& U(void) {
            return m_data[1];
        }

        inline CVector& F(void) {
            return m_data[2];
        }

        inline CVector& T(void) {
            return m_data[3];
        }

        CMatrix operator* (CMatrix& other);

        CMatrix& operator*= (CMatrix& other);

        float Det(void);

        CMatrix Inverse(void);

        CMatrix Transpose();

        inline CVector Rotate(const CVector& v) {
            CMatrix t = this->Transpose();
            return CVector(v.Dot(t[0]), v.Dot(t[1]), v.Dot(t[2]));
        }

        inline CVector Unrotate(const CVector v) const {
            return CVector(v.Dot(m_data[0]), v.Dot(m_data[1]), v.Dot(m_data[2]));
        }


        inline float* AsArray() {
            for (size_t i = 0; i < 4; i++)
                memcpy(m_array + 4 * i, &m_data[i].m_data, sizeof(m_data[i].m_data));
            return m_array;
        }


        inline float* Array (void) {
            return m_array;
        }
};

// =================================================================================================

