/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *  Last Modified: 04/25/2016 21:47:02
 *
 *    Description: message class for actor system
 *
 *                 TODO
 *                 I was thinkink of whether I need to put some compress
 *                 mechanics inside but finally give up. Because most likely
 *                 one message created-copied-destroied by only one time, its
 *                 life cycle is this three-phased.
 *
 *                 If enable compressing, then it's
 *                      created-compressed-copied-decompressed-destroied
 *                 it's a five-phased cycle, I am doubting whether its worthwile
 *                 to do, since as mentioned, most of the message only copied by
 *                 one time and then destroyed.
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <cstring>
#include <cstdint>

enum MessagePackType: int {
    MPK_UNKNOWN = 0,
    MPK_HELLO,
    MPK_METRONOME,
    MPK_ACTIVATE,
};

typedef struct {
    uint32_t GUID;
    uint32_t UID;
    uint32_t AddTime;

    int X;
    int Y;
}AMNewPlayer;

typedef struct {
    uint32_t GUID;
    uint32_t UID;
    uint32_t AddTime;

    int X;
    int Y;
}AMLogin;

template<size_t StaticBufSize = 64>
class InnMessagePack final
{
    private:
        int         m_Type;

        size_t      m_BufLen;
        uint8_t    *m_Buf;

        size_t      m_StaticBufUsedLen;
        uint8_t     m_StaticBuf[StaticBufSize];

        uint32_t    m_Response;

    public:
        InnMessagePack(int nMsgType = MPK_UNKNOWN)
            : m_Type(nMsgType)
            , m_BufLen(0)
            , m_Buf(nullptr)
            , m_StaticBufUsedLen(0)
            , m_Response(0)
        {}

        InnMessagePack(int nMsgType, const uint8_t *pData, size_t nDataLen)
        {
            // 1. type
            m_Type = nMsgType;

            if(pData && nDataLen > 0){
                if(nDataLen <= StaticBufSize){
                    std::memcpy(m_StaticBuf, pData, nDataLen);
                    m_StaticBufUsedLen = nDataLen;
                }else{
                    m_Buf = new uint8_t[nDataLen];
                    m_BufLen = nDataLen;
                    std::memcpy(m_Buf, pData, nDataLen);
                }
            }
        }

        InnMessagePack(const InnMessagePack &rstMPK)
        {
            // 1. type
            m_Type = rstMPK.m_Type;

            // 2. static buffer
            if(rstMPK.m_StaticBufUsedLen > 0){
                std::memcpy(m_StaticBuf, rstMPK.m_StaticBuf, m_StaticBufUsedLen);
            }

            // 3. dynamic buffer
            if(rstMPK.m_Buf && rstMPK.m_BufLen > 0){
                m_Buf = new uint8_t[rstMPK.m_BufLen];
                m_BufLen = rstMPK.m_BufLen;
                std::memcpy(m_Buf, rstMPK.m_Buf, rstMPK.m_BufLen);
            }
        }

        // don't put pointer in T, this is copied by bytes
        template <typename T>
        InnMessagePack(int nMsgType, const T &rstPOD)
        {
            static_assert(std::is_pod<T>::value, "POD data type supported only");
            if(sizeof(rstPOD) > StaticBufSize){
                m_Buf = new uint8_t[sizeof(rstPOD)];
                m_BufLen = sizeof(rstPOD);

                std::memcpy(m_Buf, &rstPOD, sizeof(rstPOD));
            }else{
                m_StaticBufUsedLen = sizeof(rstPOD);
                std::memcpy(m_StaticBuf, &rstPOD, sizeof(rstPOD));
            }
        }

        ~InnMessagePack()
        {
            delete m_Buf;
        }

    public:
        // most used, for message forwarding, need optimization
        InnMessagePack &operator = (InnMessagePack stMPK)
        {
            std::swap(m_Buf, stMPK.m_Buf);

            m_Type = stMPK.m_Type;
            m_BufLen = stMPK.m_BufLen;
            m_StaticBufUsedLen = stMPK.m_StaticBufUsedLen;

            if(m_StaticBufUsedLen > 0){
                std::memcpy(m_StaticBuf, stMPK.m_StaticBuf, m_StaticBufUsedLen);
            }
            return *this;
        }

    public:
        int Type() const
        {
            return m_Type;
        }

        const uint8_t *Data() const
        {
            if(m_StaticBufUsedLen > 0){
                return m_StaticBuf;
            }

            return m_Buf;
        }

        size_t DataLen() const
        {
            if(m_StaticBufUsedLen){
                return m_StaticBufUsedLen;
            }
            return m_BufLen;
        }

        size_t Size() const
        {
            return DataLen();
        }

        uint32_t Response() const
        {
            return m_Response;
        }
};

using MessagePack = InnMessagePack<64>;
