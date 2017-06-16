/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *  Last Modified: 06/16/2017 14:12:32
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

template<size_t SBufSize = 64>
class InnMessagePack final
{
    private:
        int m_Type;

    private:
        uint32_t m_ID;
        uint32_t m_Respond;

    private:
        uint8_t  m_SBuf[SBufSize];
        size_t   m_SBufUsedLen;

    private:
        uint8_t *m_DBuf;
        size_t   m_DBufLen;

    public:
        // since we make sender to accept only MessageBuf
        // then here makes ID and Respond to be immutable and can only be set during initialization
        InnMessagePack(int nType = MPK_NONE,    // message type
                const uint8_t *pData = nullptr, // message buffer
                size_t nDataLen = 0,            // message buffer length
                uint32_t nID = 0,               // request id
                uint32_t nRespond = 0)          // reply id
            : m_Type(nType)
            , m_ID(nID)
            , m_Respond(nRespond)
        {
            if(pData && nDataLen){
                if(nDataLen <= SBufSize){
                    m_SBufUsedLen = nDataLen;
                    std::memcpy(m_SBuf, pData, nDataLen);

                    m_DBuf = nullptr;
                    m_DBufLen = 0;
                }else{
                    m_DBuf = new uint8_t[nDataLen];
                    m_DBufLen = nDataLen;
                    std::memcpy(m_DBuf, pData, nDataLen);

                    m_SBufUsedLen = 0;
                }
            }else{
                m_DBuf = nullptr;
                m_DBufLen = 0;

                m_SBufUsedLen = 0;
            }
        }

        InnMessagePack(const MessageBuf &rstMB, uint32_t nID = 0, uint32_t nRespond = 0)
            : InnMessagePack(rstMB.Type(), rstMB.Data(), rstMB.DataLen(), nID, nRespond)
        {}

        InnMessagePack(InnMessagePack &&rstMPK)
            : m_Type(rstMPK.Type())
            , m_ID(rstMPK.ID())
            , m_Respond(rstMPK.Respond())
        {
            // case-1: use dynamic buffer, steal the buffer
            //         after this call rstMPK will be invalid
            if(rstMPK.m_DBuf && rstMPK.m_DBufLen){
                m_DBuf = rstMPK.m_DBuf;
                m_DBufLen = rstMPK.m_DBufLen;

                m_SBufUsedLen = 0;

                rstMPK.m_DBuf = nullptr;
                rstMPK.m_DBufLen = 0;
                return;
            }

            // case-1: use static buffer, copy only
            //         but after this call I make rstMPK invalid
            if(rstMPK.m_SBufUsedLen){
                m_SBufUsedLen = rstMPK.m_SBufUsedLen;
                std::memcpy(m_SBuf, rstMPK.m_SBuf, m_SBufUsedLen);

                m_DBuf = nullptr;
                m_DBufLen = 0;

                rstMPK.m_SBufUsedLen = 0;
                return;
            }

            // case-3: empty message
            //         shouldn't happen here
            {
                m_SBufUsedLen = 0;

                m_DBuf = nullptr;
                m_DBufLen = 0;
                return;
            }
        }

        InnMessagePack(const InnMessagePack &rstMPK)
            : InnMessagePack(rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen(), rstMPK.ID(), rstMPK.Respond())
        {}

    public:
       ~InnMessagePack()
        {
            delete [] m_DBuf;
        }

    public:
       InnMessagePack &operator = (InnMessagePack stMPK)
       {
           std::swap(m_Type         , stMPK.m_Type       );
           std::swap(m_ID           , stMPK.m_ID         );
           std::swap(m_Respond      , stMPK.m_Respond    );

           std::swap(m_SBufUsedLen  , stMPK.m_SBufUsedLen);
           std::swap(m_DBuf         , stMPK.m_DBuf       );
           std::swap(m_DBufLen      , stMPK.m_DBufLen    );

           if(m_SBufUsedLen){
               std::memcpy(m_SBuf, stMPK.m_SBuf, m_SBufUsedLen);
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
            return m_SBufUsedLen ? m_SBuf : m_DBuf;
        }

        size_t DataLen() const
        {
            return m_SBufUsedLen ? m_SBufUsedLen : m_DBufLen;
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

        const char *Name() const
        {
            switch(m_Type){
                case MPK_NONE                : return "MPK_NONE";
                case MPK_OK                  : return "MPK_OK";
                case MPK_ERROR               : return "MPK_ERROR";
                case MPK_BADACTORPOD         : return "MPK_BADACTORPOD";
                case MPK_TIMEOUT             : return "MPK_TIMEOUT";
                case MPK_UID                 : return "MPK_UID";
                case MPK_PING                : return "MPK_PING";
                case MPK_LOGIN               : return "MPK_LOGIN";
                case MPK_METRONOME           : return "MPK_METRONOME";
                case MPK_TRYMOVE             : return "MPK_TRYMOVE";
                case MPK_MOVEOK              : return "MPK_MOVEOK";
                case MPK_TRYLEAVE            : return "MPK_TRYLEAVE";
                case MPK_TRYSPACEMOVE        : return "MPK_TRYSPACEMOVE";
                case MPK_LOGINOK             : return "MPK_LOGINOK";
                case MPK_ADDRESS             : return "MPK_ADDRESS";
                case MPK_LOGINQUERYDB        : return "MPK_LOGINQUERYDB";
                case MPK_NETPACKAGE          : return "MPK_NETPACKAGE";
                case MPK_ADDCHAROBJECT       : return "MPK_ADDCHAROBJECT";
                case MPK_BINDSESSION         : return "MPK_BINDSESSION";
                case MPK_ACTION              : return "MPK_ACTION";
                case MPK_QUERYMONSTERGINFO   : return "MPK_QUERYMONSTERGINFO";
                case MPK_PULLCOINFO          : return "MPK_PULLCOINFO";
                case MPK_NEWCONNECTION       : return "MPK_NEWCONNECTION";
                case MPK_QUERYMAPLIST        : return "MPK_QUERYMAPLIST";
                case MPK_MAPLIST             : return "MPK_MAPLIST";
                case MPK_MAPSWITCH           : return "MPK_MAPSWITCH";
                case MPK_MAPSWITCHOK         : return "MPK_MAPSWITCHOK";
                case MPK_TRYMAPSWITCH        : return "MPK_TRYMAPSWITCH";
                case MPK_QUERYMAPUID         : return "MPK_QUERYMAPUID";
                case MPK_QUERYLOCATION       : return "MPK_QUERYLOCATION";
                case MPK_LOCATION            : return "MPK_LOCATION";
                case MPK_PATHFIND            : return "MPK_PATHFIND";
                case MPK_PATHFINDOK          : return "MPK_PATHFINDOK";
                case MPK_ATTACK              : return "MPK_ATTACK";
                case MPK_UPDATEHP            : return "MPK_UPDATEHP";
                case MPK_DEADFADEOUT         : return "MPK_DEADFADEOUT";
                case MPK_QUERYCORECORD       : return "MPK_QUERYCORECORD";
                case MPK_QUERYCOCOUNT        : return "MPK_QUERYCOCOUNT";
                case MPK_COCOUNT             : return "MPK_COCOUNT";
                default                      : return "MPK_UNKNOWN";
            }
        }
};

using MessagePack = InnMessagePack<64>;
