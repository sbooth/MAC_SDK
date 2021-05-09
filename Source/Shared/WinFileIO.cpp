#include "All.h"

#ifdef IO_USE_WIN_FILE_IO

#include "WinFileIO.h"
#include <windows.h>
#include "CharacterHelper.h"

namespace APE
{

CWinFileIO::CWinFileIO()
{
    m_hFile = INVALID_HANDLE_VALUE;
    memset(m_cFileName, 0, sizeof(m_cFileName));
    m_bReadOnly = false;
}

CWinFileIO::~CWinFileIO()
{
    Close();
}

int CWinFileIO::Open(const wchar_t * pName, bool bOpenReadOnly)
{
    Close();

    if (wcslen(pName) >= MAX_PATH)
        return ERROR_UNDEFINED;

    #ifdef _UNICODE
        CSmartPtr<wchar_t> spName((wchar_t *) pName, true, false);    
    #else
        CSmartPtr<char> spName(GetANSIFromUTF16(pName), true);
    #endif

    // open (read / write)
    if (!bOpenReadOnly)
        m_hFile = ::CreateFile(spName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) 
    {
        // open (read-only)
        m_hFile = ::CreateFile(spName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_hFile == INVALID_HANDLE_VALUE) 
        {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_SHARING_VIOLATION)
            {
                // file in use
                return ERROR_OPENING_FILE_IN_USE;
            }
            else
            {
                // dwError of 2 means not found
                return ERROR_INVALID_INPUT_FILE;
            }
        }
        else 
        {
            m_bReadOnly = true;
        }
    }
    else
    {
        m_bReadOnly = false;
    }
    
    wcscpy_s(m_cFileName, MAX_PATH, pName);

    return ERROR_SUCCESS;
}

int CWinFileIO::Close()
{
    SAFE_FILE_CLOSE(m_hFile);

    return ERROR_SUCCESS;
}
    
int CWinFileIO::Read(void * pBuffer, unsigned int nBytesToRead, unsigned int * pBytesRead)
{
    unsigned int nTotalBytesRead = 0;
    int nBytesLeft = nBytesToRead;
    bool bRetVal = true;
    unsigned char * pucBuffer = (unsigned char *) pBuffer;

    *pBytesRead = 1;
    while ((nBytesLeft > 0) && (*pBytesRead > 0) && bRetVal)
    {
        bRetVal = ::ReadFile(m_hFile, &pucBuffer[nBytesToRead - nBytesLeft], nBytesLeft, (unsigned long *) pBytesRead, NULL) ? true : false;
        if (bRetVal && (*pBytesRead <= 0))
            bRetVal = false;
        if (bRetVal)
        {
            nBytesLeft -= *pBytesRead;
            nTotalBytesRead += *pBytesRead;
        }
    }
    
    *pBytesRead = nTotalBytesRead;
    
    return bRetVal ? ERROR_SUCCESS : ERROR_IO_READ;
}

int CWinFileIO::Write(const void * pBuffer, unsigned int nBytesToWrite, unsigned int * pBytesWritten)
{
    bool bRetVal = WriteFile(m_hFile, pBuffer, nBytesToWrite, (unsigned long *) pBytesWritten, NULL) ? true : false;

    if ((bRetVal == 0) || (*pBytesWritten != nBytesToWrite))
        return ERROR_IO_WRITE;
    else
        return ERROR_SUCCESS;
}

int64 CWinFileIO::PerformSeek()
{
    DWORD dwMoveMethod = 0;
    if (m_nSeekMethod == APE_FILE_BEGIN)
        dwMoveMethod = FILE_BEGIN;
    else if (m_nSeekMethod == APE_FILE_END)
        dwMoveMethod = FILE_END;
    else if (m_nSeekMethod == APE_FILE_CURRENT)
        dwMoveMethod = FILE_CURRENT;

    LONG Low = (m_nSeekPosition & 0xFFFFFFFF);
    LONG High = (m_nSeekPosition >> 32);

    SetFilePointer(m_hFile, Low, &High, dwMoveMethod);
    return ERROR_SUCCESS;
}

int CWinFileIO::SetEOF()
{
    return SetEndOfFile(m_hFile) ? ERROR_SUCCESS : ERROR_UNDEFINED;
}

int64 CWinFileIO::GetPosition()
{
    LONG nPositionHigh = 0;
    DWORD dwPositionLow = SetFilePointer(m_hFile, 0, &nPositionHigh, FILE_CURRENT);
    int64 nPosition = int64(dwPositionLow) + (int64(nPositionHigh) << 32);
    return nPosition;
}

int64 CWinFileIO::GetSize()
{
    DWORD dwFileSizeHigh = 0;
    DWORD dwFileSizeLow = GetFileSize(m_hFile, &dwFileSizeHigh);
    return int64(dwFileSizeLow) + (int64(dwFileSizeHigh) << 32);
}

int CWinFileIO::GetName(wchar_t * pBuffer)
{
    wcscpy_s(pBuffer, MAX_PATH, m_cFileName);
    return ERROR_SUCCESS;
}

int CWinFileIO::Create(const wchar_t * pName)
{
    Close();

    if (wcslen(pName) >= MAX_PATH)
        return ERROR_UNDEFINED;

    #ifdef _UNICODE
        CSmartPtr<wchar_t> spName((wchar_t *) pName, true, false);    
    #else
        CSmartPtr<char> spName(GetANSIFromUTF16(pName), true);
    #endif

    m_hFile = CreateFile(spName, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) 
        return ERROR_IO_WRITE;

    m_bReadOnly = false;
    
    wcscpy_s(m_cFileName, MAX_PATH, pName);

    return ERROR_SUCCESS;
}

int CWinFileIO::Delete()
{
    Close();

    #ifdef _UNICODE
        CSmartPtr<wchar_t> spName(m_cFileName, true, false);    
    #else
        CSmartPtr<char> spName(GetANSIFromUTF16(m_cFileName), true);
    #endif

    return DeleteFile(spName) ? ERROR_SUCCESS : ERROR_UNDEFINED;
}

}

#endif // #ifdef IO_USE_WIN_FILE_IO
