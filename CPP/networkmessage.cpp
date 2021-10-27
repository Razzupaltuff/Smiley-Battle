#include "networkmessage.h"

// =================================================================================================
// network data and address

bool CMessage::IsValid(int valueCount) {
    /*
        check a message for a match with the requested keyword
        deconstruct message (Split payload it into separate values)
        check message correctness (value count)

    Parameters:
    -----------
        keyword:
            the keyword to check the message for
        valueCount:
            number of application parameters that the message payload should contain.
            > 0: specifies the exact required number of parameters
            < 0: specifies the required minimum number of parameters
            == 0: don't check parameter count
    */
    m_values = m_payload.Split('#');
    CString keyword = m_values[0];
    m_values = m_values[1].Split(';');
    if (!m_values[0].Empty())
        m_numValues = m_values.Length();
    else {
        m_values = CList<CString>();
        m_numValues = 0;
    }
    if (valueCount == 0) {
        m_result = 1;
        return true;
    }
    if (valueCount > 0) {
        if (m_numValues == valueCount) {
            m_result = 1;
            return true;
        }
    }
    else if (valueCount < 0) {
        if (m_numValues >= -valueCount) {
            m_result = 1;
            return true;
        }
    }
    fprintf(stderr, "message %s has wrong number of values (expected %d, found %zd)", keyword.Buffer (), valueCount, m_numValues);
    m_result = -1;
    return false;
}

// =================================================================================================
