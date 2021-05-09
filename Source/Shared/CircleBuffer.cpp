#include "All.h"
#include "CircleBuffer.h"
#include "CRC.h"

namespace APE
{

CCircleBuffer::CCircleBuffer()
{
    m_pBuffer = NULL;
    m_nTotal = 0;
    m_nHead = 0;
    m_nTail = 0;
    m_nEndCap = 0;
    m_nMaxDirectWriteBytes = 0;
}

CCircleBuffer::~CCircleBuffer()
{
    SAFE_ARRAY_DELETE(m_pBuffer)
}

void CCircleBuffer::CreateBuffer(int64 nBytes, int64 nMaxDirectWriteBytes)
{
    SAFE_ARRAY_DELETE(m_pBuffer)
    
    m_nMaxDirectWriteBytes = nMaxDirectWriteBytes;
    m_nTotal = nBytes + 1 + nMaxDirectWriteBytes;
    m_pBuffer = new unsigned char [(unsigned int) m_nTotal];
    m_nHead = 0;
    m_nTail = 0;
    m_nEndCap = m_nTotal;
}

int64 CCircleBuffer::MaxAdd()
{
    int64 nMaxAdd = (m_nTail >= m_nHead) ? (m_nTotal - 1 - m_nMaxDirectWriteBytes) - (m_nTail - m_nHead) : m_nHead - m_nTail - 1;
    return nMaxAdd;
}

int64 CCircleBuffer::MaxGet()
{
    return (m_nTail >= m_nHead) ? m_nTail - m_nHead : (m_nEndCap - m_nHead) + m_nTail;
}

uint32 CCircleBuffer::UpdateCRC(uint32 nCRC, int64 nBytes)
{
    int64 nFrontBytes = ape_min(m_nTail, nBytes);
    int64 nHeadBytes = nBytes - nFrontBytes;

    if (nHeadBytes > 0)
        nCRC = CRC_update(nCRC, &m_pBuffer[m_nEndCap - nHeadBytes], nHeadBytes);

    nCRC = CRC_update(nCRC, &m_pBuffer[m_nTail - nFrontBytes], nFrontBytes);

    return nCRC;
}

int64 CCircleBuffer::Get(unsigned char * pBuffer, int64 nBytes)
{
    int64 nTotalGetBytes = 0;

    if (pBuffer != NULL && nBytes > 0)
    {
        int64 nHeadBytes = ape_min(m_nEndCap - m_nHead, nBytes);
        int64 nFrontBytes = nBytes - nHeadBytes;

        memcpy(&pBuffer[0], &m_pBuffer[m_nHead], (size_t) nHeadBytes);
        nTotalGetBytes = nHeadBytes;

        if (nFrontBytes > 0)
        {
            memcpy(&pBuffer[nHeadBytes], &m_pBuffer[0], (size_t) nFrontBytes);
            nTotalGetBytes += nFrontBytes;
        }

        RemoveHead(nBytes);
    }

    return nTotalGetBytes;
}

void CCircleBuffer::Empty()
{
    m_nHead = 0;
    m_nTail = 0;
    m_nEndCap = m_nTotal;
}

int64 CCircleBuffer::RemoveHead(int64 nBytes)
{
    nBytes = ape_min(MaxGet(), nBytes);
    m_nHead += nBytes;
    if (m_nHead >= m_nEndCap)
        m_nHead -= m_nEndCap;
    return nBytes;
}

int64 CCircleBuffer::RemoveTail(int64 nBytes)
{
    nBytes = ape_min(MaxGet(), nBytes);
    m_nTail -= nBytes;
    if (m_nTail < 0)
        m_nTail += m_nEndCap;
    return nBytes;
}

}