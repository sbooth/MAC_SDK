#include "All.h"
#include "GlobalFunctions.h"
#include "NNFilter.h"
#include "Predictor.h"
#ifdef ENABLE_SSE_ASSEMBLY
    #include <emmintrin.h> // SSE 2
#endif

namespace APE
{

CNNFilter::CNNFilter(intn nOrder, intn nShift, intn nVersion, bool b32Bit, IPredictorDecompress * pDecompress)
{
    if ((nOrder <= 0) || ((nOrder % 16) != 0)) throw(1);
    m_nOrder = nOrder;
    m_nShift = nShift;
    m_nVersion = nVersion;
    m_bSSEAvailable = GetSSEAvailable(false);
    m_nBitdepth = b32Bit ? 32 : 16;
    m_pDecompress = pDecompress;

    m_rbInput16.Create(NN_WINDOW_ELEMENTS, m_nOrder);
    m_rbDeltaM16.Create(NN_WINDOW_ELEMENTS, m_nOrder);
    m_rbInput32.Create(NN_WINDOW_ELEMENTS, m_nOrder);
    m_rbDeltaM32.Create(NN_WINDOW_ELEMENTS, m_nOrder);

    m_paryM16 = (short * ) AllocateAligned(intn(sizeof(short)) * m_nOrder, 16); // align for possible SSE usage
    m_paryM32 = (int *) AllocateAligned(intn(sizeof(int)) * m_nOrder, 16); // align for possible SSE usage
}

CNNFilter::~CNNFilter()
{
    if (m_paryM16 != NULL)
    {
        FreeAligned(m_paryM16);
        m_paryM16 = NULL;
    }
    if (m_paryM32 != NULL)
    {
        FreeAligned(m_paryM32);
        m_paryM32 = NULL;
    }
}

void CNNFilter::Flush()
{
    memset(&m_paryM16[0], 0, m_nOrder * sizeof(short));
    memset(&m_paryM32[0], 0, m_nOrder * sizeof(int));
    m_rbInput16.Flush();
    m_rbDeltaM16.Flush();
    m_rbInput32.Flush();
    m_rbDeltaM32.Flush();
    m_nRunningAverage = 0;
}

int64 CNNFilter::Compress(int64 nInput)
{
    if (m_nBitdepth == 16)
    {
        // convert the input to a short and store it
        m_rbInput16[0] = GetSaturatedShortFromInt(nInput);

        // figure a dot product
        int64 nDotProduct;
        #ifdef ENABLE_SSE_ASSEMBLY
            if (this->useSSE2())
               nDotProduct = CalculateDotProductSSEx16(&m_rbInput16[-m_nOrder], &m_paryM16[0], m_nOrder);
            else
        #endif
        nDotProduct = CalculateDotProductx16(&m_rbInput16[-m_nOrder], &m_paryM16[0], m_nOrder);

        // calculate the output
        int64 nOutput = int64(nInput) - int64((int64(nDotProduct) + (int64(1) << (int64(m_nShift) - 1))) >> int64(m_nShift));

        // adapt
#ifdef ENABLE_SSE_ASSEMBLY
        if (this->useSSE2())
            AdaptSSEx16(&m_paryM16[0], &m_rbDeltaM16[-m_nOrder], nOutput, m_nOrder);
        else
#endif
            Adaptx16(&m_paryM16[0], &m_rbDeltaM16[-m_nOrder], nOutput, m_nOrder);

        int64 nTempABS = llabs(nInput);

        if (nTempABS > (m_nRunningAverage * 3))
            m_rbDeltaM16[0] = ((nInput >> 25) & 64) - 32;
        else if (nTempABS > (m_nRunningAverage * 4) / 3)
            m_rbDeltaM16[0] = ((nInput >> 26) & 32) - 16;
        else if (nTempABS > 0)
            m_rbDeltaM16[0] = ((nInput >> 27) & 16) - 8;
        else
            m_rbDeltaM16[0] = 0;

        m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

        m_rbDeltaM16[-1] >>= 1;
        m_rbDeltaM16[-2] >>= 1;
        m_rbDeltaM16[-8] >>= 1;

        // increment and roll if necessary
        m_rbInput16.IncrementSafe();
        m_rbDeltaM16.IncrementSafe();

        return nOutput;
    }
    else // m_nBitdepth == 32
    {
        // convert the input to a short and store it
        m_rbInput32[0] = GetSaturatedShortFromInt(nInput);

        // figure a dot product
        int64 nDotProduct;
        nDotProduct = CalculateDotProduct(&m_rbInput32[-m_nOrder], &m_paryM32[0], m_nOrder);

        // calculate the output
        int64 nOutput = int64(nInput) - int64((int64(nDotProduct) + (int64(1) << (int64(m_nShift) - 1))) >> int64(m_nShift));

        // adapt
#ifdef ENABLE_SSE_ASSEMBLY
        if (this->useSSE2())
            AdaptSSE(&m_paryM32[0], &m_rbDeltaM32[-m_nOrder], nOutput, m_nOrder);
        else
#endif
            Adapt(&m_paryM32[0], &m_rbDeltaM32[-m_nOrder], nOutput, m_nOrder);

        int64 nTempABS = llabs(nInput);

        if (nTempABS > (m_nRunningAverage * 3))
            m_rbDeltaM32[0] = ((nInput >> 25) & 64) - 32;
        else if (nTempABS > (m_nRunningAverage * 4) / 3)
            m_rbDeltaM32[0] = ((nInput >> 26) & 32) - 16;
        else if (nTempABS > 0)
            m_rbDeltaM32[0] = ((nInput >> 27) & 16) - 8;
        else
            m_rbDeltaM32[0] = 0;

        m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

        m_rbDeltaM32[-1] >>= 1;
        m_rbDeltaM32[-2] >>= 1;
        m_rbDeltaM32[-8] >>= 1;

        // increment and roll if necessary
        m_rbInput32.IncrementSafe();
        m_rbDeltaM32.IncrementSafe();

        return nOutput;
    }
}

int64 CNNFilter::Decompress(int64 nInput)
{
    if (m_nBitdepth == 16)
    {
        // figure a dot product
        int64 nDotProduct;
        #ifdef ENABLE_SSE_ASSEMBLY
            if (this->useSSE2())
                nDotProduct = CalculateDotProductSSEx16(&m_rbInput16[-m_nOrder], &m_paryM16[0], m_nOrder);
            else
        #endif
                nDotProduct = CalculateDotProductx16(&m_rbInput16[-m_nOrder], &m_paryM16[0], m_nOrder);

        // adapt
#ifdef ENABLE_SSE_ASSEMBLY
        if (this->useSSE2())
            AdaptSSEx16(&m_paryM16[0], &m_rbDeltaM16[-m_nOrder], nInput, m_nOrder);
        else
#endif
            Adaptx16(&m_paryM16[0], &m_rbDeltaM16[-m_nOrder], nInput, m_nOrder);

        // store the output value
        int64 nOutput;
        if (m_pDecompress && m_pDecompress->GetLegacyDecode())
            nOutput = nInput + ((int(nDotProduct) + (int(1) << (m_nShift - 1))) >> m_nShift);
        else
            nOutput = nInput + ((int64(nDotProduct) + (int64(1) << (m_nShift - 1))) >> m_nShift);

        // update the input buffer
        m_rbInput16[0] = GetSaturatedShortFromInt(nOutput);

        if (m_nVersion >= 3980)
        {
            int64 nTempABS = llabs(nOutput);

            if (nTempABS > (m_nRunningAverage * 3))
                m_rbDeltaM16[0] = ((nOutput >> 25) & 64) - 32;
            else if (nTempABS > (m_nRunningAverage * 4) / 3)
                m_rbDeltaM16[0] = ((nOutput >> 26) & 32) - 16;
            else if (nTempABS > 0)
                m_rbDeltaM16[0] = ((nOutput >> 27) & 16) - 8;
            else
                m_rbDeltaM16[0] = 0;

            m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

            m_rbDeltaM16[-1] >>= 1;
            m_rbDeltaM16[-2] >>= 1;
            m_rbDeltaM16[-8] >>= 1;
        }
        else
        {
            m_rbDeltaM16[0] = (nOutput == 0) ? 0 : ((nOutput >> 28) & 8) - 4;
            m_rbDeltaM16[-4] >>= 1;
            m_rbDeltaM16[-8] >>= 1;
        }

        // increment and roll if necessary
        m_rbInput16.IncrementSafe();
        m_rbDeltaM16.IncrementSafe();

        return nOutput;
    }
    else // m_nBitdepth == 32
    {
        // figure a dot product
        int64 nDotProduct;
        nDotProduct = CalculateDotProduct(&m_rbInput32[-m_nOrder], &m_paryM32[0], m_nOrder);

        // adapt
    #ifdef ENABLE_SSE_ASSEMBLY
        if (this->useSSE2())
            AdaptSSE(&m_paryM32[0], &m_rbDeltaM32[-m_nOrder], nInput, m_nOrder);
        else
    #endif
            Adapt(&m_paryM32[0], &m_rbDeltaM32[-m_nOrder], nInput, m_nOrder);

        // store the output value
        int64 nOutput = nInput + ((int64(nDotProduct) + (int64(1) << (m_nShift - 1))) >> m_nShift);

        // update the input buffer
        m_rbInput32[0] = GetSaturatedShortFromInt(nOutput);

        if (m_nVersion >= 3980)
        {
            int64 nTempABS = llabs(nOutput);

            if (nTempABS > (m_nRunningAverage * 3))
                m_rbDeltaM32[0] = ((nOutput >> 25) & 64) - 32;
            else if (nTempABS > (m_nRunningAverage * 4) / 3)
                m_rbDeltaM32[0] = ((nOutput >> 26) & 32) - 16;
            else if (nTempABS > 0)
                m_rbDeltaM32[0] = ((nOutput >> 27) & 16) - 8;
            else
                m_rbDeltaM32[0] = 0;

            m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

            m_rbDeltaM32[-1] >>= 1;
            m_rbDeltaM32[-2] >>= 1;
            m_rbDeltaM32[-8] >>= 1;
        }
        else
        {
            m_rbDeltaM32[0] = (nOutput == 0) ? 0 : ((nOutput >> 28) & 8) - 4;
            m_rbDeltaM32[-4] >>= 1;
            m_rbDeltaM32[-8] >>= 1;
        }

        // increment and roll if necessary
        m_rbInput32.IncrementSafe();
        m_rbDeltaM32.IncrementSafe();
    
        return nOutput;
    }
}

void CNNFilter::Adapt(int * pM, int * pAdapt, int64 nDirection, intn nOrder)
{
    nOrder >>= 4;

    if (nDirection < 0) 
    {    
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ += *pAdapt++;)
        }    
    }
    else if (nDirection > 0)
    {
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ -= *pAdapt++;)
        }
    }
}

void CNNFilter::Adaptx16(short * pM, short * pAdapt, int64 nDirection, intn nOrder)
{
    nOrder >>= 4;

    if (nDirection < 0)
    {
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ += *pAdapt++;)
        }
    }
    else if (nDirection > 0)
    {
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ -= *pAdapt++;)
        }
    }
}

int64 CNNFilter::CalculateDotProduct(int * pA, int * pB, intn nOrder)
{
    int64 nDotProduct = 0;
    nOrder >>= 4;

    while (nOrder--)
    {
        EXPAND_16_TIMES(nDotProduct += *pA++ * *pB++;)
    }
    
    return nDotProduct;
}

int64 CNNFilter::CalculateDotProductx16(short * pA, short * pB, intn nOrder)
{
    int64 nDotProduct = 0;
    nOrder >>= 4;

    while (nOrder--)
    {
        EXPAND_16_TIMES(nDotProduct += *pA++ * *pB++;)
    }

    return nDotProduct;
}

#ifdef ENABLE_SSE_ASSEMBLY

void CNNFilter::AdaptSSE(int * pM, int * pAdapt, int64 nDirection, intn nOrder)
{
    // we require that pM is aligned, allowing faster loads and stores
    ASSERT((size_t(pM) % 16) == 0);

    if (nDirection < 0) 
    {    
        for (int z = 0; z < nOrder; z += 4)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_add_epi32(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
    else if (nDirection > 0)
    {
        for (int z = 0; z < nOrder; z += 4)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_sub_epi32(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
}

void CNNFilter::AdaptSSEx16(short * pM, short * pAdapt, int64 nDirection, intn nOrder)
{
    // we require that pM is aligned, allowing faster loads and stores
    ASSERT((size_t(pM) % 16) == 0);

    if (nDirection < 0) 
    {    
        for (int z = 0; z < nOrder; z += 8)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_add_epi16(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
    else if (nDirection > 0)
    {
        for (int z = 0; z < nOrder; z += 8)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_sub_epi16(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
}

int64 CNNFilter::CalculateDotProductSSEx16(short * pA, short * pB, intn nOrder)
{
    // we require that pB is aligned, allowing faster loads
    ASSERT((size_t(pB) % 16) == 0);

    // loop
    __m128i sseSum = _mm_setzero_si128();
    for (int z = 0; z < nOrder; z += 8)
    {
        __m128i sseA = _mm_loadu_si128((__m128i*) & pA[z]);
        __m128i sseB = _mm_load_si128((__m128i*) & pB[z]);
        __m128i sseDotProduct = _mm_madd_epi16(sseA, sseB);
        sseSum = _mm_add_epi32(sseSum, sseDotProduct);
    }

    // build output
    int64 nDotProduct = 0;
#ifndef _MSC_VER
    __oword my__oword;
    my__oword.m128i = sseSum;
    nDotProduct = my__oword.m128i_i32[0] + my__oword.m128i_i32[1] + my__oword.m128i_i32[2] + my__oword.m128i_i32[3];
#else
    nDotProduct = sseSum.m128i_i32[0] + sseSum.m128i_i32[1] + sseSum.m128i_i32[2] + sseSum.m128i_i32[3];
#endif

    // TODO: SSE4 instructions might help performance of the horizontal add, for example:
    //int nDotProduct = _mm_extract_epi32(sseSum, 0) + _mm_extract_epi32(sseSum, 1) + _mm_extract_epi32(sseSum, 2) + _mm_extract_epi32(sseSum, 3);

    return nDotProduct;
}

#endif

}