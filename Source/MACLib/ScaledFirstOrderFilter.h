#pragma once

namespace APE
{

template <int64 MULTIPLY, int64 SHIFT> class CScaledFirstOrderFilter
{
public:
    
    __forceinline void Flush()
    {
        m_nLastValue = 0;
    }

    __forceinline int64 Compress(const int64 nInput)
    {
        int64 nResult = nInput - ((m_nLastValue * MULTIPLY) >> SHIFT);
        m_nLastValue = nInput;
        return nResult;
    }

    __forceinline int64 Decompress(const int64 nInput)
    {
        m_nLastValue = nInput + ((m_nLastValue * MULTIPLY) >> SHIFT);
        return m_nLastValue;
    }

protected:
    int64 m_nLastValue;
};

}
