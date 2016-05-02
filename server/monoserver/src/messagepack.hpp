/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *  Last Modified: 05/02/2016 00:09:58
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
 *                 TODO
 *                 MessagePack is the conveyor of messaging between actors. It needs
 *                 to support message indexing. I.E.
 *
 *                 1. A send B a message, and expecting a reply
 *                 2. B get the message, consumed it, and send A a reply as expected
 *                 3. B also waiting for A to inform him that the reply is received
 *
 *                 This is the most case, then a MessagePack needs two indexing, one
 *                 to inform the sender: this is the reply you are expecting, and 
 *                 another for the receiver itself: I am expecting a reply with this
 *                 index back.
 *
 *                 so we need to put two indices in the MessagePack.
 *
 *                 TODO
 *                 For an actor it will receive two kinds of messages:
 *                 1. no expected, any other legal actor can send it message without
 *                    previously appointed
 *                 2. exptected replys from those actors it previously communicated
 *
 *                + --------------------------------------------------------------------
 *                |  From sender's view:
 *                |
 *                |  MessagePack rstMPK
 *                |  if(this message is to reply a previous message with index n){
 *                |      rstMPK.Respond(n);
 *                |  }
 *                |
 *                |  if(I exptect a reply for this message to send){
 *                |      // just put an non-zero value to mark it
 *                |      // internal logic in ActorPod will assign a proper index
 *                |      rstMPK.ID(1);
 *                |  }
 *                |
 *                |  Send(rstMPK, rstAddress);
 *                |
 *                + --------------------------------------------------------------------
 *                |  From receiver's view:
 *                |  
 *                |  get a pending message rstMPK;
 *                |  
 *                |  if(rstMPK.Respond()){
 *                |      // this message is for replying a previously sent message
 *                |      // find and execute the registed handler
 *                |      m_CallBack.find(rstMPK.Respond()).second();
 *                |  }
 *                |  
 *                |  MessagePack rstReplyMPK;
 *                |  // properly inited when handling the received message
 *                |  
 *                |  if(rstMPK.ID()){
 *                |      // this message is expected for reply
 *                |      rstReplyMPK.Respond(rstMPK.ID());
 *                |      Send(rstReplyMPK, rstReplyMPK);
 *                |  }
 *                |  
 *                +--------------------------------------------------------------------
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
    MPK_NEWMONSTOR,
    MPK_LOGINOK,
    MPK_FORWARDCM,
};

typedef struct {
    uint32_t GUID;
    uint32_t UID;
    uint32_t AddTime;
    int X;
    int Y;
    void *Data;
}AMNewMonster;

typedef struct {
    int  X;
    int  Y;
}AMMonitorReady;

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
    uint32_t UID;
    uint32_t AddTime;

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


// define the actor message conveyor
template<size_t StaticBufSize = 64>
class InnMessagePack final
{
    private:
        int         m_Type;

        size_t      m_BufLen;
        uint8_t    *m_Buf;

        size_t      m_StaticBufUsedLen;
        uint8_t     m_StaticBuf[StaticBufSize];

        uint32_t    m_ID;
        uint32_t    m_Respond;

    public:
        InnMessagePack(int nMsgType = MPK_UNKNOWN)
            : m_Type(nMsgType)
            , m_BufLen(0)
            , m_Buf(nullptr)
            , m_StaticBufUsedLen(0)
            , m_ID(0)
            , m_Respond(0)
        {}

        // TODO, here's the reason for many bugs
        // think about how to create a class with many ctor's
        InnMessagePack(int nMsgType,    // message type
                const uint8_t *pData,   // 
                size_t nDataLen,        // 
                uint32_t nRespond = 0)  // this message is to reply a previously received one
            : m_Type(nMsgType)
            , m_ID(0)
            , m_Respond(nRespond)
        {
            if(pData && nDataLen > 0){
                if(nDataLen <= StaticBufSize){
                    m_StaticBufUsedLen = nDataLen;
                    std::memcpy(m_StaticBuf, pData, nDataLen);

                    m_Buf = nullptr;
                    m_BufLen = 0;
                }else{
                    m_Buf = new uint8_t[nDataLen];
                    m_BufLen = nDataLen;
                    std::memcpy(m_Buf, pData, nDataLen);

                    m_StaticBufUsedLen = 0;
                }
            }else{
                m_BufLen = 0;
                m_Buf = nullptr;
                m_StaticBufUsedLen = 0;
            }
        }

        // don't put pointer in T, this is copied by bytes
        template <typename T>
        InnMessagePack(int nMsgType, const T &rstPOD, uint32_t nRespond = 0)
            : InnMessagePack(nMsgType, (const uint8_t *)&rstPOD, sizeof(rstPOD), nRespond)
        {
            static_assert(std::is_pod<T>::value, "POD data type supported only");
        }

        InnMessagePack(InnMessagePack &&rstMPK)
            : m_Type(rstMPK.Type())
            , m_ID(rstMPK.ID())
            , m_Respond(rstMPK.Respond())
        {
            if(rstMPK.m_Buf && rstMPK.m_BufLen > 0){
                // using dynamic buffer
                m_StaticBufUsedLen = 0;

                m_Buf = rstMPK.m_Buf;
                m_BufLen = rstMPK.m_BufLen;

                rstMPK.m_Buf = nullptr;
                rstMPK.m_BufLen = 0;

                return;
            }

            if(rstMPK.m_StaticBufUsedLen > 0){
                m_Buf = nullptr;
                m_BufLen = 0;

                m_StaticBufUsedLen = rstMPK.m_StaticBufUsedLen;
                std::memcpy(m_StaticBuf, rstMPK.m_StaticBuf, m_StaticBufUsedLen);

                // ...
                rstMPK.m_StaticBufUsedLen = 0;
                return;
            }

            m_StaticBufUsedLen = 0;
            m_Buf = nullptr;
            m_BufLen = 0;
        }

        InnMessagePack(const InnMessagePack &rstMPK)
            : InnMessagePack(rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen(), rstMPK.Respond())
        {
            m_ID = rstMPK.ID();
        }


        ~InnMessagePack()
        {
            delete m_Buf;
        }

    public:
        // most used, for message forwarding, need optimization
        InnMessagePack &operator = (InnMessagePack stMPK)
        {

            std::swap(m_Type, stMPK.m_Type);
            std::swap(m_ID, stMPK.m_ID);
            std::swap(m_Respond, stMPK.m_Respond);

            std::swap(m_Buf, stMPK.m_Buf);
            std::swap(m_BufLen, stMPK.m_BufLen);
            std::swap(m_StaticBufUsedLen, stMPK.m_StaticBufUsedLen);

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
            if(m_StaticBufUsedLen > 0){
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

        uint32_t ID() const
        {
            return m_ID;
        }

    protected:
        // this can only be invocated by ActorPod
        friend class ActorPod;
        friend class SyncDriver;

        void Respond(uint32_t nRespond)
        {
            m_Respond = nRespond;
        }


        void ID(uint32_t nID)
        {
            m_ID = nID;
        }
};

using MessagePack = InnMessagePack<64>;
