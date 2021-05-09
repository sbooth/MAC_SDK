#pragma once

#include "Predictor.h"
#include "RollBuffer.h"
#include "NNFilter.h"
#include "ScaledFirstOrderFilter.h"

namespace APE
{

/**************************************************************************************************
Functions to create the interfaces
**************************************************************************************************/
IPredictorCompress * __stdcall CreateIPredictorCompress();
IPredictorDecompress * __stdcall CreateIPredictorDecompress();

#define WINDOW_BLOCKS           512

#define BUFFER_COUNT            1
#define HISTORY_ELEMENTS        8
#define M_COUNT                 8

class CPredictorCompressNormal : public IPredictorCompress
{
public:
    CPredictorCompressNormal(intn nCompressionLevel, intn nBitsPerSample);
    virtual ~CPredictorCompressNormal();

    int64 CompressValue(int64 nA, int64 nB = 0);
    int Flush();

protected:
    // buffer information
    CRollBufferFast<int64, WINDOW_BLOCKS, 10> m_rbPrediction;
    CRollBufferFast<int64, WINDOW_BLOCKS, 9> m_rbAdapt;

    CScaledFirstOrderFilter<31, 5> m_Stage1FilterA;
    CScaledFirstOrderFilter<31, 5> m_Stage1FilterB;

    // adaption
    int64 m_aryM[9];
    
    // other
    int m_nCurrentIndex;
    CNNFilter * m_pNNFilter;
    CNNFilter * m_pNNFilter1;
    CNNFilter * m_pNNFilter2;
};

class CPredictorDecompressNormal3930to3950 : public IPredictorDecompress
{
public:
    CPredictorDecompressNormal3930to3950(intn nCompressionLevel, intn nVersion);
    virtual ~CPredictorDecompressNormal3930to3950();

    int64 DecompressValue(int64 nInput, int64);
    int Flush();
    
protected:
    // buffer information
    int64 * m_pBuffer[BUFFER_COUNT];

    // adaption
    int64 m_aryM[M_COUNT];
    
    // buffer pointers
    int64 * m_pInputBuffer;

    // other
    int m_nCurrentIndex;
    int64 m_nLastValue;
    CNNFilter * m_pNNFilter;
    CNNFilter * m_pNNFilter1;
};

class CPredictorDecompress3950toCurrent : public IPredictorDecompress
{
public:
    CPredictorDecompress3950toCurrent(intn nCompressionLevel, intn nVersion, intn nBitsPerSample);
    virtual ~CPredictorDecompress3950toCurrent();

    int64 DecompressValue(int64 nA, int64 nB = 0);
    int Flush();
    void SetLegacyDecode(bool bSet) { m_bLegacyDecode = bSet; }
    bool GetLegacyDecode() { return m_bLegacyDecode; }

protected:
    // adaption
    int64 m_aryMA[M_COUNT];
    int64 m_aryMB[M_COUNT];
    
    // buffer pointers
    CRollBufferFast<int64, WINDOW_BLOCKS, 8> m_rbPredictionA;
    CRollBufferFast<int64, WINDOW_BLOCKS, 8> m_rbPredictionB;

    CRollBufferFast<int64, WINDOW_BLOCKS, 8> m_rbAdaptA;
    CRollBufferFast<int64, WINDOW_BLOCKS, 8> m_rbAdaptB;

    CScaledFirstOrderFilter<31, 5> m_Stage1FilterA;
    CScaledFirstOrderFilter<31, 5> m_Stage1FilterB;

    // other
    intn m_nCurrentIndex;
    int64 m_nLastValueA;
    intn m_nVersion;
    CNNFilter * m_pNNFilter;
    CNNFilter * m_pNNFilter1;
    CNNFilter * m_pNNFilter2;
    bool m_bLegacyDecode;
};

}
