#pragma once

namespace APE
{

/**************************************************************************************************
IPredictorCompress - the interface for compressing (predicting) data
**************************************************************************************************/
class IPredictorCompress
{
public:
    IPredictorCompress(intn nCompressionLevel) {}
    virtual ~IPredictorCompress() {}

    virtual int64 CompressValue(int64 nA, int64 nB = 0) = 0;
    virtual int Flush() = 0;
};

/**************************************************************************************************
IPredictorDecompress - the interface for decompressing (un-predicting) data
**************************************************************************************************/
class IPredictorDecompress
{
public:
    IPredictorDecompress(intn nCompressionLevel, intn nVersion) {}
    virtual ~IPredictorDecompress() {}

    virtual int64 DecompressValue(int64 nA, int64 nB = 0) = 0;
    virtual int Flush() = 0;
    virtual void SetLegacyDecode(bool bSet) { }
    virtual bool GetLegacyDecode() { return false; }
};

}
