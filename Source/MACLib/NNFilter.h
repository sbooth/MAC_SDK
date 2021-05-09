#pragma once

#ifdef ENABLE_SSE_ASSEMBLY
    #include <emmintrin.h> // SSE 2
#endif

namespace APE
{

#include "RollBuffer.h"
#define NN_WINDOW_ELEMENTS    512
class IPredictorDecompress;

class CNNFilter
{
public:
    CNNFilter(intn nOrder, intn nShift, intn nVersion, bool b32Bit, IPredictorDecompress * pDecompress);
    ~CNNFilter();

    int64 Compress(int64 nInput);
    int64 Decompress(int64 nInput);
    void Flush();

private:
    int m_nBitdepth;
    intn m_nOrder;
    intn m_nShift;
    intn m_nVersion;
    int64 m_nRunningAverage;
    APE::CRollBuffer<short> m_rbInput16;
    APE::CRollBuffer<short> m_rbDeltaM16;
    APE::CRollBuffer<int> m_rbInput32;
    APE::CRollBuffer<int> m_rbDeltaM32;
    short * m_paryM16;
    int * m_paryM32;
    bool m_bSSEAvailable; 
    IPredictorDecompress * m_pDecompress;
    bool useSSE2() const { return m_bSSEAvailable; }
    
    __forceinline short GetSaturatedShortFromInt(int64 nValue) const
    {
        short sValue;
        if (short(nValue) == nValue)
            sValue = short(nValue);
        else
            sValue = (nValue >> 63) ^ 0x7FFF;
        return sValue;
    }

    __forceinline void Adapt(int * pM, int * pAdapt, int64 nDirection, intn nOrder);
    __forceinline void Adaptx16(short * pM, short * pAdapt, int64 nDirection, intn nOrder);
    __forceinline int64 CalculateDotProduct(int * pA, int * pB, intn nOrder);
    __forceinline int64 CalculateDotProductx16(short * pA, short * pB, intn nOrder);
    
    #ifdef ENABLE_SSE_ASSEMBLY
        __forceinline void AdaptSSE(int * pM, int * pAdapt, int64 nDirection, intn nOrder);
        __forceinline void AdaptSSEx16(short * pM, short * pAdapt, int64 nDirection, intn nOrder);
        __forceinline int64 CalculateDotProductSSEx16(short * pA, short * pB, intn nOrder);
    #endif
    
    #if !defined(_MSC_VER) && defined(ENABLE_SSE_ASSEMBLY)
        typedef union __attribute__ ((aligned (16))) __oword {
            __m128i m128i;
            int8_t m128i_i8[16];
            int16_t m128i_i16[8];
            int32_t m128i_i32[4];
            int64_t m128i_i64[2];
            uint8_t m128i_u8[16];
            uint16_t m128i_u16[8];
            uint32_t m128i_u32[4];
            uint64_t m128i_u64[2];
        } __oword;
    #endif
};

}
