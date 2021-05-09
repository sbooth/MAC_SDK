#pragma once

namespace APE
{

class CCircleBuffer  
{
public:
    // construction / destruction
    CCircleBuffer();
    virtual ~CCircleBuffer();

    // create the buffer
    void CreateBuffer(int64 nBytes, int64 nMaxDirectWriteBytes);

    // query
    int64 MaxAdd();
    int64 MaxGet();

    // direct writing
    __forceinline unsigned char * GetDirectWritePointer()
    {
        // return a pointer to the tail -- note that it will always be safe to write
        // at least m_nMaxDirectWriteBytes since we use an end cap region
        return &m_pBuffer[m_nTail];
    }

    __forceinline void UpdateAfterDirectWrite(int64 nBytes)
    {
        // update the tail
        m_nTail += nBytes;

        // if the tail enters the "end cap" area, set the end cap and loop around
        if (m_nTail >= (m_nTotal - m_nMaxDirectWriteBytes))
        {
            m_nEndCap = m_nTail;
            m_nTail = 0;
        }
    }

    // update CRC for last nBytes bytes
    uint32 UpdateCRC(uint32 nCRC, int64 nBytes);

    // get data
    int64 Get(unsigned char * pBuffer, int64 nBytes);

    // remove / empty
    void Empty();
    int64 RemoveHead(int64 nBytes);
    int64 RemoveTail(int64 nBytes);

private:
    int64 m_nTotal;
    int64 m_nMaxDirectWriteBytes;
    int64 m_nEndCap;
    int64 m_nHead;
    int64 m_nTail;
    unsigned char * m_pBuffer;
};

}
