#pragma once
#include <cstdint>
#include <cstddef>
#include <concepts>
#include "strf.hpp"
#include "fflerror.hpp"

namespace msgf
{
    struct MsgAttribute
    {
        const std::string name;

        //  0    :     empty
        //  1    : not empty,     fixed size,     compressed by xor
        //  2    : not empty,     fixed size, not compressed
        //  3    : not empty, not fixed size, not compressed
        const int type;

        // define message length before compression
        //  0    : empty or not fixed
        //  x    : non-empty fixed size before compression
        const size_t dataLen;

        MsgAttribute(const char *argName, int argType, size_t argLen = 0)
            : name(str_haschar(argName) ? argName : throw fflerror("invalid argument"))
            , type(argType)
            , dataLen(argLen)
        {
            // we can't use the log system here for error logging
            // since MsgAttribute will be used in both ClientMessage and ServerMessage

            switch(type){
                case 0:
                    {
                        // empty, shouldn't have any content
                        if(dataLen){
                            throw fflerror("invalid dataLen: %zu", dataLen);
                        }
                        break;
                    }
                case 1:
                    {
                        // not empty, fixed size, compressed
                        // support maxCompLen = 0b_1111111_1111111_1111111_1111111, abort if bigger than it
                        if(!(dataLen > 0 && dataLen < (1uz << 4 * 7))){
                            throw fflerror("invalid dataLen: %zu", dataLen);
                        }
                        break;
                    }
                case 2:
                    {
                        // not empty, fixed size, not compressed
                        // used for small messages that even xor compress is too much
                        if(!dataLen){
                            throw fflerror("invalid dataLen: %zu", dataLen);
                        }
                        break;
                    }
                case 3:
                    {
                        // not empty, not fixed size, not compressed
                        // dataLen should be zero to indicate it's not fixed size message
                        if(dataLen){
                            throw fflerror("invalid dataLen: %zu", dataLen);
                        }
                        break;
                    }
                default:
                    {
                        throw fflerror("invalid message type: %d", type);
                    }
            }
        }
    };

    class MsgBase
    {
        private:
            const uint8_t m_headCode;

        protected:
            MsgBase(uint8_t argHeadCode)
                : m_headCode(argHeadCode)
            {}

        public:
            bool hasResp() const
            {
                return m_headCode & 0x80;
            }

            bool useXor64() const
            {
                return type() == 1 && dataLen() >= 256;
            }

        public:
            uint8_t headCode() const
            {
                return m_headCode & 0x7f;
            }

            int type() const
            {
                return getAttribute().type;
            }

            size_t dataLen() const
            {
                return getAttribute().dataLen;
            }

            size_t maskLen() const
            {
                if(type() == 1){
                    if(useXor64()){ return (dataLen() + 63) / 64; }
                    else          { return (dataLen() +  7) /  8; }
                }
                return 0;
            }

            const std::string &name() const
            {
                return getAttribute().name;
            }

        private:
            virtual const MsgAttribute &getAttribute() const = 0;

        public:
            bool checkData(const void *data, size_t dataSize) const
            {
                switch(type()){
                    case 0:
                        {
                            return (data == nullptr) && (dataSize == 0);
                        }
                    case 1:
                    case 2:
                        {
                            return data && (dataSize == dataLen());
                        }
                    case 3:
                        {
                            if(data){
                                return dataSize > 0;
                            }
                            else{
                                return dataSize == 0;
                            }
                        }
                    default:
                        {
                            return false;
                        }
                }
            }

            bool checkDataSize(size_t maskSize, size_t bodySize) const
            {
                switch(type()){
                    case 0:
                        {
                            return (maskSize == 0) && (bodySize == 0);
                        }
                    case 1:
                        {
                            return (maskSize == maskLen()) && (bodySize <= dataLen());
                        }
                    case 2:
                        {
                            return (maskSize == 0) && (bodySize == dataLen());
                        }
                    case 3:
                        {
                            return maskSize == 0;
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
    };

    inline size_t encodeLength(uint8_t *buf, size_t bufSize, std::unsigned_integral auto length)
    {
        size_t bytes = 0;
        while(bytes < bufSize){
            const uint8_t bits = length & 0x7f;
            length >>= 7;

            if(length > 0){
                buf[bytes++] = bits | 0x80;
            }
            else{
                buf[bytes++] = bits;
                break;
            }
        }

        if(length > 0){
            throw fflerror("buffer can not hold length bits, bufSize %zu, length %zu", bufSize, length);
        }
        return bytes;
    }

    template<std::unsigned_integral T> T decodeLength(const uint8_t *buf, size_t bufSize)
    {
        static_assert(sizeof(T) >= 2); // may have issue when checking overflow if single byte

        fflassert(buf);
        fflassert(bufSize >= 1);

        T length = 0;
        for(size_t i = 0; i < bufSize; ++i){
            if(const T nextLength = (length << 7) | (buf[bufSize - 1 - i] & 0x7f); nextLength < length){
                throw fflerror("decode length overflows: %s", str_any(std::vector<uint8_t>(buf, buf + bufSize)).c_str());
            }
            else{
                length = nextLength;
            }
        }
        return length;
    }
}
