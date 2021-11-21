#include "All.h"
#include "IO.h"
#include "APECompressCreate.h"
#include "APECompressCore.h"

#ifdef APE_SUPPORT_COMPRESS

namespace APE
{

CAPECompressCreate::CAPECompressCreate()
{
    m_nMaxFrames = 0;
    m_bTooMuchData = false;
}

CAPECompressCreate::~CAPECompressCreate()
{
}

int CAPECompressCreate::Start(CIO * pioOutput, const WAVEFORMATEX * pwfeInput, int64 nMaxAudioBytes, int nCompressionLevel, const void * pHeaderData, int64 nHeaderBytes, int32 nFlags)
{
    // verify the parameters
    if (pioOutput == NULL || pwfeInput == NULL)
        return ERROR_BAD_PARAMETER;

    // verify the wave format
    if ((pwfeInput->nChannels < 1) || (pwfeInput->nChannels > 32))
    {
        return ERROR_INPUT_FILE_UNSUPPORTED_CHANNEL_COUNT;
    }
    if ((pwfeInput->wBitsPerSample != 8) && (pwfeInput->wBitsPerSample != 16) && (pwfeInput->wBitsPerSample != 24) && (pwfeInput->wBitsPerSample != 32))
    {
        return ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH;
    }
    if ((pwfeInput->wBitsPerSample == 32) && (nCompressionLevel > MAC_COMPRESSION_LEVEL_EXTRA_HIGH))
    {
        return ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH;
    }
    if (pwfeInput->wFormatTag != WAVE_FORMAT_PCM)
    {
        return ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH;
    }

    // initialize (creates the base classes)
    m_nBlocksPerFrame = 73728;
    if (nCompressionLevel == MAC_COMPRESSION_LEVEL_EXTRA_HIGH)
        m_nBlocksPerFrame *= 4;
    else if (nCompressionLevel == MAC_COMPRESSION_LEVEL_INSANE)
        m_nBlocksPerFrame *= 16;

    m_spIO.Assign(pioOutput, false, false);
    m_spAPECompressCore.Assign(new CAPECompressCore(m_spIO, pwfeInput, m_nBlocksPerFrame, nCompressionLevel));
    
    // copy the format
    memcpy(&m_wfeInput, pwfeInput, sizeof(WAVEFORMATEX));
    
    // the compression level
    m_nCompressionLevel = nCompressionLevel;
    m_nFrameIndex = 0;
    m_nLastFrameBlocks = m_nBlocksPerFrame;
    
    // initialize the file
    uint32 nMaxAudioBlocks = uint32(nMaxAudioBytes / pwfeInput->nBlockAlign);
    intn nMaxFrames = nMaxAudioBlocks / m_nBlocksPerFrame;
    if ((nMaxAudioBlocks % m_nBlocksPerFrame) != 0) nMaxFrames++;
        
    InitializeFile(m_spIO, &m_wfeInput, nMaxFrames,
        m_nCompressionLevel, pHeaderData, nHeaderBytes, nFlags);
    
    return ERROR_SUCCESS;
}

intn CAPECompressCreate::GetFullFrameBytes()
{
    return m_nBlocksPerFrame * m_wfeInput.nBlockAlign;
}

int CAPECompressCreate::EncodeFrame(const void * pInputData, int nInputBytes)
{
    int nInputBlocks = nInputBytes / m_wfeInput.nBlockAlign;
    
    if ((nInputBlocks < m_nBlocksPerFrame) && (m_nLastFrameBlocks < m_nBlocksPerFrame))
    {
        return ERROR_UNDEFINED; // can only pass a smaller frame for the very last time
    }

    // update the seek table
    m_spAPECompressCore->GetBitArray()->AdvanceToByteBoundary();
    int nResult = SetSeekByte(m_nFrameIndex, (int) m_spIO->GetPosition() + (m_spAPECompressCore->GetBitArray()->GetCurrentBitIndex() / 8));
    if (nResult != ERROR_SUCCESS)
        return nResult;
    
    // compress
    nResult = m_spAPECompressCore->EncodeFrame(pInputData, nInputBytes);
    
    // update stats
    m_nLastFrameBlocks = nInputBlocks;
    m_nFrameIndex++;

    return nResult;
}

int CAPECompressCreate::Finish(const void * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes)
{
    // clear the bit array
    RETURN_ON_ERROR(m_spAPECompressCore->GetBitArray()->OutputBitArray(true));
    
    // finalize the file
    RETURN_ON_ERROR(FinalizeFile(m_spIO, m_nFrameIndex, m_nLastFrameBlocks, 
        pTerminatingData, nTerminatingBytes, nWAVTerminatingBytes));
    
    return ERROR_SUCCESS;
}

int CAPECompressCreate::SetSeekByte(int nFrame, uint32 nByteOffset)
{
    if (nFrame >= m_nMaxFrames)
    {
        m_bTooMuchData = true;
        return ERROR_APE_COMPRESS_TOO_MUCH_DATA;
    }
    m_spSeekTable[nFrame] = nByteOffset;
    return ERROR_SUCCESS;
}

int CAPECompressCreate::InitializeFile(CIO * pIO, const WAVEFORMATEX * pwfeInput, intn nMaxFrames, intn nCompressionLevel, const void * pHeaderData, int64 nHeaderBytes, int32 nFlags)
{
    // error check the parameters
    if (pIO == NULL || pwfeInput == NULL || nMaxFrames <= 0)
        return ERROR_BAD_PARAMETER;
    
    APE_DESCRIPTOR APEDescriptor; memset(&APEDescriptor, 0, sizeof(APEDescriptor));
    APE_HEADER APEHeader; memset(&APEHeader, 0, sizeof(APEHeader));

    // create the descriptor (only fill what we know)
    APEDescriptor.cID[0] = 'M';
    APEDescriptor.cID[1] = 'A';
    APEDescriptor.cID[2] = 'C';
    APEDescriptor.cID[3] = ' ';
    APEDescriptor.nVersion = MAC_FILE_VERSION_NUMBER;
    APEDescriptor.nPadding = 0; // set to zero even though we memset above to be clean
    
    APEDescriptor.nDescriptorBytes = sizeof(APEDescriptor);
    APEDescriptor.nHeaderBytes = sizeof(APEHeader);
    APEDescriptor.nSeekTableBytes = uint32(nMaxFrames) * uint32(sizeof(unsigned int));
    APEDescriptor.nHeaderDataBytes = (uint32) ((nHeaderBytes == CREATE_WAV_HEADER_ON_DECOMPRESSION) ? 0 : nHeaderBytes);

    // create the header (only fill what we know now)
    APEHeader.nBitsPerSample = pwfeInput->wBitsPerSample;
    APEHeader.nChannels = pwfeInput->nChannels;
    APEHeader.nSampleRate = pwfeInput->nSamplesPerSec;
    
    APEHeader.nCompressionLevel = (uint16) nCompressionLevel;
    APEHeader.nFormatFlags = uint16(nFlags) | ((nHeaderBytes == CREATE_WAV_HEADER_ON_DECOMPRESSION) ? MAC_FORMAT_FLAG_CREATE_WAV_HEADER : 0);
    
    APEHeader.nBlocksPerFrame = (uint32) m_nBlocksPerFrame;

    // write the data to the file
    unsigned int nBytesWritten = 0;
    RETURN_ON_ERROR(pIO->Write(&APEDescriptor, sizeof(APEDescriptor), &nBytesWritten))
    RETURN_ON_ERROR(pIO->Write(&APEHeader, sizeof(APEHeader), &nBytesWritten))
        
    // write an empty seek table
    m_spSeekTable.Assign(new uint32 [nMaxFrames], true);
    if (m_spSeekTable == NULL) { return ERROR_INSUFFICIENT_MEMORY; }
    ZeroMemory(m_spSeekTable, nMaxFrames * 4);
    RETURN_ON_ERROR(pIO->Write(m_spSeekTable, (unsigned int) (nMaxFrames * 4), &nBytesWritten))
    m_nMaxFrames = nMaxFrames;

    // write the WAV data
    if ((pHeaderData != NULL) && (nHeaderBytes > 0) && (nHeaderBytes != CREATE_WAV_HEADER_ON_DECOMPRESSION))
    {
        m_spAPECompressCore->GetBitArray()->GetMD5Helper().AddData(pHeaderData, (int) nHeaderBytes);
        RETURN_ON_ERROR(pIO->Write((void *) pHeaderData, (unsigned int) nHeaderBytes, &nBytesWritten))
    }

    return ERROR_SUCCESS;
}

int CAPECompressCreate::FinalizeFile(CIO * pIO, int nNumberOfFrames, int nFinalFrameBlocks, const void * pTerminatingData, int64 nTerminatingBytes, int64 nWAVTerminatingBytes)
{
    // store the tail position
    int64 nTailPosition = pIO->GetPosition();

    // append the terminating data
    unsigned int nBytesWritten = 0;
    unsigned int nBytesRead = 0;
    int64 nResult = 0;
    if ((pTerminatingData != NULL) && (nTerminatingBytes > 0))
    {
        // update the MD5 sum to include the WAV terminating bytes
        m_spAPECompressCore->GetBitArray()->GetMD5Helper().AddData(pTerminatingData, nWAVTerminatingBytes);

        // write the entire chunk to the new file
        if (pIO->Write((void *) pTerminatingData, (unsigned int) nTerminatingBytes, &nBytesWritten) != 0) { return ERROR_IO_WRITE; }
    }
    
    // go to the beginning and update the information
    pIO->SetSeekPosition(0);
    pIO->SetSeekMethod(APE_FILE_BEGIN);
    nResult = pIO->PerformSeek();
    
    // get the descriptor
    APE_DESCRIPTOR APEDescriptor;
    nResult = pIO->Read(&APEDescriptor, sizeof(APEDescriptor), &nBytesRead);
    if ((nResult != 0) || (nBytesRead != sizeof(APEDescriptor))) { return ERROR_IO_READ; }

    // get the header
    APE_HEADER APEHeader;
    nResult = pIO->Read(&APEHeader, sizeof(APEHeader), &nBytesRead);
    if (nResult != 0 || nBytesRead != sizeof(APEHeader)) { return ERROR_IO_READ; }
    
    // update the header
    APEHeader.nFinalFrameBlocks = nFinalFrameBlocks;
    APEHeader.nTotalFrames = nNumberOfFrames;
    
    // update the descriptor
    int64 nFrameDataBytes = nTailPosition - (APEDescriptor.nDescriptorBytes + APEDescriptor.nHeaderBytes + APEDescriptor.nSeekTableBytes + APEDescriptor.nHeaderDataBytes);
    APEDescriptor.nAPEFrameDataBytes = (uint32) (nFrameDataBytes & 0xFFFFFFFF);
    APEDescriptor.nAPEFrameDataBytesHigh = (uint32) (nFrameDataBytes >> 32);
    APEDescriptor.nTerminatingDataBytes = (uint32) nWAVTerminatingBytes;
    
    // update the MD5
    m_spAPECompressCore->GetBitArray()->GetMD5Helper().AddData(&APEHeader, sizeof(APEHeader));
    m_spAPECompressCore->GetBitArray()->GetMD5Helper().AddData(m_spSeekTable, int64(m_nMaxFrames * 4));
    m_spAPECompressCore->GetBitArray()->GetMD5Helper().GetResult(APEDescriptor.cFileMD5);

    // set the pointer and re-write the updated header and peak level
    pIO->SetSeekMethod(APE_FILE_BEGIN);
    pIO->SetSeekPosition(0);
    nResult = pIO->PerformSeek();
    if (pIO->Write(&APEDescriptor, sizeof(APEDescriptor), &nBytesWritten) != 0) { return ERROR_IO_WRITE; }
    if (pIO->Write(&APEHeader, sizeof(APEHeader), &nBytesWritten) != 0) { return ERROR_IO_WRITE; }
    
    // write the updated seek table
    if (pIO->Write(m_spSeekTable, (unsigned int) (m_nMaxFrames * 4), &nBytesWritten) != 0) { return ERROR_IO_WRITE; }
    
    return ERROR_SUCCESS;
}

}

#endif