
#include "glew.h"
#include "camera.h"

// =================================================================================================
// Position and orientation (scaling, translation and rotation) handling
// Can have parent cameras. Compute world position depending on parent positions if a parent other
// than the viewer is present. This allows for child actors that move with their parent actors.

CCamera& CCamera::operator=(const CCamera& other) {
    if (this != &other) {
        m_orientation = other.m_orientation;
        m_positions[0] = other.m_positions[0];
        m_positions[1] = other.m_positions[1];
        m_angles = other.m_angles;
        m_size = other.m_size;
        m_isViewer = false;
    }
    return *this;
}


void CCamera::Enable (void) {
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    if (m_parent == nullptr) {
        Rotate (m_orientation);
        Move (-m_positions [0]);
    }
    else {
        Move (m_positions [0]);
        Rotate (m_orientation);
    }
    Scale ();
}


void CCamera::Disable (void) {
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    }


void CCamera::ClampAngles (void) {
    auto clamp = [](float v) { return (v < -180.0f) ? 360.0f : (v > 180.0f) ? -360.0f : 0.0f; };
    m_angles.X() += clamp(m_angles.X());
    m_angles.Y() += clamp(m_angles.Y());
    m_angles.Z() += clamp(m_angles.Z());
    /*
    if (m_angles.X() < -180.0f)
        m_angles.X() += 360.0f;
    else if (m_angles.X() > 180.0f)
        m_angles.X() -= 360.0f;
    if (m_angles.Y() < -180.0f)
        m_angles.Y() += 360.0f;
    else if (m_angles.Y() > 180.0f)
        m_angles.Y() -= 360.0f;
    if (m_angles.Z() < -180.0f)
        m_angles.Z() += 360.0f;
    else if (m_angles.Z() > 180.0f)
        m_angles.Z() -= 360.0f;
*/
}


// create an orientation (rotation) matrix from heading angles
void CCamera::UpdateAngles (CVector angles, bool reverse) {
    m_angles += angles;
    ClampAngles ();
    UpdateOrientation ();
}


void CCamera::UpdateOrientation (void) {
    float radX = Rad (m_angles.X());
    float radY = Rad (m_angles.Y());
    float radZ = Rad (m_angles.Z());
    m_orientation.Compute (sin (radX), cos (radX), sin (radY), cos (radY), sin (radZ), cos (radZ));
}


// =================================================================================================
