/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *  Last Modified: 05/08/2016 18:20:58
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

#include "messagebuf.hpp"
#include "actormessage.hpp"

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
        // TODO & TBD
        // since we make sender to accept only MessageBuf
        // then we make ID and Respond to be immutable and can only be set when init\ing
        InnMessagePack(int nType = MPK_UNKNOWN, // message type
                const uint8_t *pData = nullptr, // message buffer
                size_t nDataLen = 0,            // message buffer length
                uint32_t nID = 0,               // request id
                uint32_t nRespond = 0)          // reply id
            : m_Type(nType)
            , m_ID(nID)
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

        InnMessagePack(const MessageBuf &rstMB,
                uint32_t nID = 0, uint32_t nRespond = 0)
            : InnMessagePack(rstMB.Type(), rstMB.Data(), rstMB.DataLen(), nID, nRespond)
        {}

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
            : InnMessagePack(rstMPK.Type(),
                    rstMPK.Data(), rstMPK.DataLen(), rstMPK.ID(), rstMPK.Respond())
        {}


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

        const char *Name()
        {
            switch(m_Type){
                case MPK_UNKNOWN:                      return "MPK_UNKNOWN";
                case MPK_HI:                           return "MPK_HI";
                case MPK_OK:                           return "MPK_OK";
                case MPK_ERROR:                        return "MPK_ERROR";
                case MPK_PING:                         return "MPK_PING";
                case MPK_LOGIN:                        return "MPK_LOGIN";
                case MPK_REFUSE:                       return "MPK_REFUSE";
                case MPK_MOVE:                         return "MPK_MOVE";
                case MPK_HELLO:                        return "MPK_HELLO";
                case MPK_ACTIVATE:                     return "MPK_ACTIVATE";
                case MPK_METRONOME:                    return "MPK_METRONOME";
                case MPK_ADDMONSTER:                   return "MPK_ADDMONSTER";
                case MPK_NEWPLAYER:                    return "MPK_NEWPLAYER";
                case MPK_NEWCONNECTION:                return "MPK_NEWCONNECTION";
                case MPK_PLAYERPHATOM:                 return "MPK_PLAYERPHATOM";
                case MPK_QUERYLOCATION:                return "MPK_QUERYLOCATION";
                case MPK_TRYMOVE:                      return "MPK_TRYMOVE";
                case MPK_TRYSPACEMOVE:                 return "MPK_TRYSPACEMOVE";
                case MPK_MOVEOK:                       return "MPK_MOVEOK";
                case MPK_COMMITMOVE:                   return "MPK_COMMITMOVE";
                case MPK_LOCATIION:                    return "MPK_LOCATIION";
                case MPK_MASTERPERSONA:                return "MPK_MASTERPERSONA";
                case MPK_INITREGIONMONITOR:            return "MPK_INITREGIONMONITOR";
                case MPK_MAPID:                        return "MPK_MAPID";
                case MPK_READY:                        return "MPK_READY";
                case MPK_LEAVE:                        return "MPK_LEAVE";
                case MPK_REGIONMONITORREADY:           return "MPK_REGIONMONITORREADY";
                case MPK_NEIGHBOR:                     return "MPK_NEIGHBOR";
                case MPK_NEWMONSTER:                   return "MPK_NEWMONSTER";
                case MPK_LOGINOK:                      return "MPK_LOGINOK";
                case MPK_FORWARDCM:                    return "MPK_FORWARDCM";
                case MPK_CHECKCOVER:                   return "MPK_CHECKCOVER";
                case MPK_QUERYRMADDRESS:               return "MPK_QUERYRMADDRESS";
                case MPK_ADDRESS:                      return "MPK_ADDRESS";
                default:                               return "MPK_????";
            }
            return "MPK_????";
        }

        const char *Name() const
        {
            switch(m_Type){
                case MPK_UNKNOWN:                      return "MPK_UNKNOWN";
                case MPK_HI:                           return "MPK_HI";
                case MPK_OK:                           return "MPK_OK";
                case MPK_ERROR:                        return "MPK_ERROR";
                case MPK_PING:                         return "MPK_PING";
                case MPK_LOGIN:                        return "MPK_LOGIN";
                case MPK_REFUSE:                       return "MPK_REFUSE";
                case MPK_MOVE:                         return "MPK_MOVE";
                case MPK_HELLO:                        return "MPK_HELLO";
                case MPK_ACTIVATE:                     return "MPK_ACTIVATE";
                case MPK_METRONOME:                    return "MPK_METRONOME";
                case MPK_ADDMONSTER:                   return "MPK_ADDMONSTER";
                case MPK_NEWPLAYER:                    return "MPK_NEWPLAYER";
                case MPK_NEWCONNECTION:                return "MPK_NEWCONNECTION";
                case MPK_PLAYERPHATOM:                 return "MPK_PLAYERPHATOM";
                case MPK_QUERYLOCATION:                return "MPK_QUERYLOCATION";
                case MPK_TRYMOVE:                      return "MPK_TRYMOVE";
                case MPK_TRYSPACEMOVE:                 return "MPK_TRYSPACEMOVE";
                case MPK_MOVEOK:                       return "MPK_MOVEOK";
                case MPK_COMMITMOVE:                   return "MPK_COMMITMOVE";
                case MPK_LOCATIION:                    return "MPK_LOCATIION";
                case MPK_MASTERPERSONA:                return "MPK_MASTERPERSONA";
                case MPK_INITREGIONMONITOR:            return "MPK_INITREGIONMONITOR";
                case MPK_MAPID:                        return "MPK_MAPID";
                case MPK_READY:                        return "MPK_READY";
                case MPK_LEAVE:                        return "MPK_LEAVE";
                case MPK_REGIONMONITORREADY:           return "MPK_REGIONMONITORREADY";
                case MPK_NEIGHBOR:                     return "MPK_NEIGHBOR";
                case MPK_NEWMONSTER:                   return "MPK_NEWMONSTER";
                case MPK_LOGINOK:                      return "MPK_LOGINOK";
                case MPK_FORWARDCM:                    return "MPK_FORWARDCM";
                case MPK_CHECKCOVER:                   return "MPK_CHECKCOVER";
                case MPK_QUERYRMADDRESS:               return "MPK_QUERYRMADDRESS";
                case MPK_ADDRESS:                      return "MPK_ADDRESS";
                default:                               return "MPK_????";
            }
            return "MPK_????";
        }
};

using MessagePack = InnMessagePack<64>;
