#include "totype.hpp"
#include "zcompf.hpp"

size_t zcompf::xorCountMask(const uint8_t *buf, size_t bufLen)
{
    if(bufLen > 0){
        fflassert(buf);
        return popcnt(buf, bufLen);
    }
    return 0;
}

size_t zcompf::xorCountData(const uint8_t *buf, size_t bufLen)
{
    size_t countBytes = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(buf[i]){
            countBytes++;
        }
    }
    return countBytes;
}

size_t zcompf::xorEncode(uint8_t *dst, const uint8_t *buf, size_t bufLen)
{
    fflassert(dst);
    fflassert(buf);
    fflassert(bufLen);

    const auto maskLen = (bufLen + 7) / 8;
    const auto maskPtr = dst;
    const auto compPtr = dst + maskLen;
    std::memset(maskPtr, 0, maskLen);

    size_t encodeBytes = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(buf[i]){
            maskPtr[i / 8] |= (0x01 << (i % 8));
            compPtr[encodeBytes++] = buf[i];
        }
    }
    return encodeBytes;
}

size_t zcompf::xorDecode(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
{
    fflassert(dst);
    fflassert(bufLen);

    fflassert(maskPtr);
    fflassert(compPtr);

    size_t decodeBytes = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(maskPtr[i / 8] & (0x01 << (i % 8))){
            dst[i] = compPtr[decodeBytes++];
        }
        else{
            dst[i] = 0;
        }
    }
    return decodeBytes;
}

size_t zcompf::xorCountData64(const uint8_t *buf, size_t bufLen)
{
    size_t countBytes = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(const auto procBytes = std::min<size_t>(8, bufLen - i); as_u64(buf + i, procBytes)){
            countBytes += procBytes;
        }
    }
    return countBytes;
}

size_t zcompf::xorEncode64(uint8_t *dst, const uint8_t *buf, size_t bufLen)
{
    fflassert(dst);
    fflassert(buf);
    fflassert(bufLen);

    const auto maskLen = (bufLen + 63) / 64;
    const auto maskPtr = dst;
    const auto compPtr = dst + maskLen;
    std::memset(maskPtr, 0, maskLen);

    size_t encodeBytes = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(const size_t procBytes = std::min<size_t>(8, bufLen - i); as_u64(buf + i, procBytes)){
            maskPtr[i / 64] |= (0x01 << ((i % 64) / 8));
            std::memcpy(compPtr + encodeBytes, buf + i, procBytes);
            encodeBytes += procBytes;
        }
    }
    return encodeBytes;
}

size_t zcompf::xorDecode64(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
{
    fflassert(dst);
    fflassert(bufLen);

    fflassert(maskPtr);
    fflassert(compPtr);

    size_t decodeBytes = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(const size_t procBytes = std::min<size_t>(8, bufLen - i); maskPtr[i / 64] & (0x01 << ((i % 64) / 8))){
            std::memcpy(dst + i, compPtr + decodeBytes, procBytes);
            decodeBytes += procBytes;
        }
        else{
            std::memset(dst + i, 0, procBytes);
        }
    }
    return decodeBytes;
}
