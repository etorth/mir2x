/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *  Last Modified: 04/29/2016 00:43:05
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
#include <utility>
#include <type_traits>

enum MessagePackType: int {
    MPK_UNKNOWN = 0,
    MPK_OK,
    MPK_ERROR,
    MPK_PING,
    MPK_LOGIN,
    MPK_REFUSE,
    MPK_MOVE,
    MPK_HELLO,
    MPK_ACTIVATE,
    MPK_METRONOME,
    MPK_ADDMONSTER,
    MPK_NEWPLAYER,
    MPK_NEWCONNECTION,
    MPK_PLAYERPHATOM,
    MPK_QUERYLOCATION,
    MPK_TRYMOVE,
    MPK_MOVEOK,
    MPK_COMMITMOVE,
    MPK_LOCATIION,
    MPK_MASTERPERSONA,
    MPK_INITREGIONMONITOR,
    MPK_READY,
    MPK_NEIGHBOR,
};

typedef struct {
    int  X;
    int  Y;
}AMRequestMove;

typedef struct {
    int  X;
    int  Y;
    int  W;
    int  H;
}AMRegion;

typedef struct {
    uint32_t MonsterIndex;
    uint32_t MapID;

    bool Strict;
    int  X;
    int  Y;
}AMMasterPersona;

typedef struct {
    uint32_t MonsterIndex;
    uint32_t MapID;

    bool Strict;
    int  X;
    int  Y;
}AMAddMonster;

typedef struct {
    void *Data;
}AMNewPlayer;

typedef struct {
    uint32_t GUID;
    uint32_t UID;
    uint32_t SID;
    uint32_t AddTime;
    uint32_t MapID;
    uint64_t Key;

    int X;
    int Y;
}AMLogin;

typedef struct {
    uint32_t GUID;
}AMPlayerPhantom;

typedef struct {
    int X;
    int Y;
    int OldX;
    int OldY;
}AMTryMove;

typedef struct {
    int X;
    int Y;
    int OldX;
    int OldY;
}AMMoveOK;

typedef struct {
    int X;
    int Y;
    int OldX;
    int OldY;
}AMCommitMove;

typedef struct {
    int X;
    int Y;
    int OldX;
    int OldY;
}AMLocation;

template<size_t StaticBufSize = 64>
class InnMessagePack final
{
    private:
        int         m_Type;

        size_t      m_BufLen;
        uint8_t    *m_Buf;

        size_t      m_StaticBufUsedLen;
        uint8_t     m_StaticBuf[StaticBufSize];

        uint32_t    m_Respond;

    public:
        InnMessagePack(int nMsgType = MPK_UNKNOWN)
            : m_Type(nMsgType)
            , m_BufLen(0)
            , m_Buf(nullptr)
            , m_StaticBufUsedLen(0)
            , m_Respond(0)
        {}

        InnMessagePack(int nMsgType,
                const uint8_t *pData, size_t nDataLen, uint32_t nRespond = 0)
        {
            // 1. type
            m_Type = nMsgType;

            // 2. response requirement
            m_Respond = nRespond;

            // 3. message body
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

            // 2. response requirement
            m_Respond = rstMPK.m_Respond;

            // 3. static buffer
            if(rstMPK.m_StaticBufUsedLen > 0){
                std::memcpy(m_StaticBuf, rstMPK.m_StaticBuf, m_StaticBufUsedLen);
            }

            // 4. dynamic buffer
            if(rstMPK.m_Buf && rstMPK.m_BufLen > 0){
                m_Buf = new uint8_t[rstMPK.m_BufLen];
                m_BufLen = rstMPK.m_BufLen;
                std::memcpy(m_Buf, rstMPK.m_Buf, rstMPK.m_BufLen);
            }
        }

        // don't put pointer in T, this is copied by bytes
        template <typename T>
        InnMessagePack(int nMsgType, const T &rstPOD, uint32_t nRespond = 0)
        {
            static_assert(std::is_pod<T>::value, "POD data type supported only");

            m_Type = nMsgType;
            m_Respond = nRespond;

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
            m_Respond = stMPK.m_Respond;
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

        uint32_t Respond() const
        {
            return m_Respond;
        }

        void Respond(uint32_t nRespond)
        {
            m_Respond = nRespond;
        }
};

using MessagePack = InnMessagePack<64>;
