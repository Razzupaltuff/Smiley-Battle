
#include "matrix.h"


CMatrix& CMatrix::Compute(float sinX, float cosX, float sinY, float cosY, float sinZ, float cosZ) {
    m_data[0] = CVector(cosY * cosZ, -cosY * sinZ, sinY);
    m_data[1] = CVector(sinX * sinY * cosZ + cosX * sinZ, -sinX * sinY * sinZ + cosX * cosZ, -sinX * cosY);
    m_data[2] = CVector(-cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ + sinX * cosZ, cosX * cosY);
    m_data[3] = CVector(0, 0, 0, 1);
    return *this;
}



CMatrix CMatrix::operator* (CMatrix& other) {
    CMatrix m = CMatrix();
    CVector v (m_data[0].X(), m_data[1].X(), m_data[2].X());
    m[0].X() = v.Dot(other[0]);
    m[1].X() = v.Dot(other[1]);
    m[2].X() = v.Dot(other[2]);
    v = CVector(m_data[0].Y(), m_data[1].Y(), m_data[2].Y());
    m[0].Y() = v.Dot(other[0]);
    m[1].Y() = v.Dot(other[1]);
    m[2].Y() = v.Dot(other[2]);
    v = CVector(m_data[0].Z(), m_data[1].Z(), m_data[2].Z());
    m[0].Z() = v.Dot(other[0]);
    m[1].Z() = v.Dot(other[1]);
    m[2].Z() = v.Dot(other[2]);
    m[3] = CVector(0, 0, 0, 1);
    return m;
}


CMatrix& CMatrix::operator*= (CMatrix& other) {
    CVector v (m_data[0].X(), m_data[1].X(), m_data[2].X());
    m_data[0].X() = v.Dot(other[0]);
    m_data[1].X() = v.Dot(other[1]);
    m_data[2].X() = v.Dot(other[2]);
    v = CVector(m_data[0].Y(), m_data[1].Y(), m_data[2].Y());
    m_data[0].Y() = v.Dot(other[0]);
    m_data[1].Y() = v.Dot(other[1]);
    m_data[2].Y() = v.Dot(other[2]);
    v = CVector(m_data[0].Z(), m_data[1].Z(), m_data[2].Z());
    m_data[0].Z() = v.Dot(other[0]);
    m_data[1].Z() = v.Dot(other[1]);
    m_data[2].Z() = v.Dot(other[2]);
    m_data[3] = CVector(0, 0, 0, 1);
    return *this;
}


float CMatrix::Det(void) {
    return
        m_data[0].X() * (m_data[1].Y() * m_data[2].Z() - m_data[1].Z() * m_data[2].Y()) +
        m_data[0].Y() * (m_data[1].Z() * m_data[2].X() - m_data[1].X() * m_data[2].Z()) +
        m_data[0].Z() * (m_data[1].X() * m_data[2].Y() - m_data[1].Y() * m_data[2].X());
}


CMatrix CMatrix::Inverse(void) {
    float det = this->Det();
    return CMatrix(
        CVector(
            (m_data[1].Y() * m_data[2].Z() - m_data[1].Z() * m_data[2].Y()) / det,
            (m_data[0].Z() * m_data[2].Y() - m_data[0].Y() * m_data[2].Z()) / det,
            (m_data[0].Y() * m_data[1].Z() - m_data[0].Z() * m_data[1].Y()) / det),
        CVector(
            (m_data[1].Z() * m_data[2].X() - m_data[1].X() * m_data[2].Z()) / det,
            (m_data[0].X() * m_data[2].Z() - m_data[0].Z() * m_data[2].X()) / det,
            (m_data[0].Z() * m_data[1].X() - m_data[0].X() * m_data[1].Z()) / det),
        CVector(
            (m_data[1].X() * m_data[2].Y() - m_data[1].Y() * m_data[2].X()) / det,
            (m_data[0].Y() * m_data[2].X() - m_data[0].X() * m_data[2].Y()) / det,
            (m_data[0].X() * m_data[1].Y() - m_data[0].Y() * m_data[1].X()) / det),
        CVector(0, 0, 0, 1)
    );
}


CMatrix CMatrix::Transpose() {
    return CMatrix(
        CVector(m_data[0].X(), m_data[1].X(), m_data[2].X()),
        CVector(m_data[0].Y(), m_data[1].Y(), m_data[2].Y()),
        CVector(m_data[0].Z(), m_data[1].Z(), m_data[2].Z()),
        CVector(0, 0, 0, 1));
}

// =================================================================================================


