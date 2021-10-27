# pragma once

#define _USE_MATH_DEFINES // for C++
//#include <cmath>
//#include <math.h>
#include "glew.h"
#include "cstring.h"
#include "matrix.h"

// =================================================================================================
// Position and orientation (scaling, translation and rotation) handling
// Can have parent cameras. Compute world position depending on parent positions if a parent other
// than the viewer is present. This allows for child actors that move with their parent actors.

class CCamera {
    public:
        CString     m_name;
        CMatrix     m_orientation;
        bool        m_havePositions[2];
        CVector     m_positions[2];
        CVector     m_angles;
        float       m_size;
        bool        m_isViewer;
        CCamera *   m_parent;

    private:
        void ClampAngles (void);

    public:
        CCamera (CString name = CString (""), bool isViewer = false) {
            m_size = 1.0f;
            m_name = name;
            m_parent = nullptr;
            m_isViewer = isViewer;
            m_havePositions[0] =
            m_havePositions[1] = false;
        }


        CCamera& operator=(const CCamera& other);


        inline void Move (CVector v) {
            glTranslatef (v.X(), v.Y(), v.Z());
            }


        inline void Rotate (CMatrix& m) {
            glMultMatrixf (m.AsArray ());
            }


        inline void Scale (void) {
            glScalef (m_size, m_size, m_size);
            }


        inline void Setup (CVector& position, CMatrix& orientation) {
            m_positions [0] = position;
            m_orientation = orientation;
            }


        inline void SetParent (CCamera* parent) {
            m_parent = parent;
            }


        inline float GetSize(void) {
            return m_size;
        }

        inline void SetSize (float size) {
            m_size = size;
        }

        void Enable (void);

        void Disable (void);

        inline CVector& GetOrientation (void) {
            return m_angles;
        }

        inline void SetOrientation (CVector angles) {
            m_angles = angles;
            ClampAngles ();
            UpdateOrientation ();
            }


        inline bool HavePosition(int i = 0) {
            return m_havePositions[i];
        }


        inline CVector& GetPosition (int i = 0) {
            return m_positions[i];
        }


        inline void SetPosition (CVector position, int i = 0) {
            if (i == 0)
                BumpPosition ();
            m_positions [i] = position;
            m_havePositions[i] = position.IsValid();
            }


        inline void UpdatePosition (CVector offset) {
            BumpPosition ();
            m_positions [0] += offset;
            }


        inline void BumpPosition (void) {
            m_positions [1] = m_positions [0];
            m_havePositions[1] = m_havePositions[0];
            }


        inline CVector& R (void) {
            return m_orientation.R ();
        }


        inline CVector& U (void) {
            return m_orientation.U ();
        }


        inline CVector& F (void) {
            return m_orientation.F ();
        }


        inline float Rad (float a) {
            return float (a / 180.0 * M_PI);
        }


        // create an orientation (rotation) matrix from heading angles
        void UpdateAngles (CVector angles, bool reverse = false);

        void UpdateOrientation (void);

    };

// =================================================================================================
