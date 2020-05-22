/*
 * =====================================================================================
 *
 *       Filename: messagepack.hpp
 *        Created: 04/20/2016 21:57:08
 *    Description:
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
#include "fflerror.hpp"
#include "messagebuf.hpp"
#include "actormessage.hpp"

template<size_t StaticBufferLength = 64> class InnMessagePack final
{
    private:
        int m_type;

    private:
        uint64_t m_from;

    private:
        uint32_t m_ID;
        uint32_t m_respond;

    private:
        uint8_t  m_SBuf[StaticBufferLength];
        size_t   m_SBufLen;

    private:
        uint8_t *m_DBuf;
        size_t   m_DBufLen;

    public:
        InnMessagePack(int nType = MPK_NONE, const uint8_t *pData = nullptr, size_t nDataLen = 0, uint64_t nFrom = 0, uint32_t nID = 0, uint32_t nRespond = 0)
            : m_type(nType)
            , m_from(nFrom)
            , m_ID(nID)
            , m_respond(nRespond)
        {
            if(pData && nDataLen){
                if(nDataLen <= StaticBufferLength){
                    m_SBufLen = nDataLen;
                    std::memcpy(m_SBuf, pData, nDataLen);

                    m_DBuf = nullptr;
                    m_DBufLen = 0;
                }else{
                    m_DBuf = new uint8_t[nDataLen];
                    m_DBufLen = nDataLen;
                    std::memcpy(m_DBuf, pData, nDataLen);

                    m_SBufLen = 0;
                }
            }else{
                m_DBuf = nullptr;
                m_DBufLen = 0;

                m_SBufLen = 0;
            }
        }

        InnMessagePack(const MessageBuf &rstMB, uint64_t nFrom = 0, uint32_t nID = 0, uint32_t nRespond = 0)
            : InnMessagePack(rstMB.Type(), rstMB.Data(), rstMB.DataLen(), nFrom, nID, nRespond)
        {}

        InnMessagePack(const InnMessagePack &rstMPK)
            : InnMessagePack(rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen(), rstMPK.from(), rstMPK.ID(), rstMPK.Respond())
        {}

        InnMessagePack(InnMessagePack &&rstMPK)
            : m_type(rstMPK.Type())
            , m_from(rstMPK.from())
            , m_ID(rstMPK.ID())
            , m_respond(rstMPK.Respond())
        {
            // invalidate the r-ref message by
            // 1. remove its from/id/resp
            // 2. remove its buffer

            rstMPK.m_type    = MPK_NONE;
            rstMPK.m_from    = 0;
            rstMPK.m_ID      = 0;
            rstMPK.m_respond = 0;

            // case-1: use dynamic buffer, steal the buffer
            //         after this call rstMPK should be destructed immediately
            if(rstMPK.m_DBuf && rstMPK.m_DBufLen){
                m_DBuf = rstMPK.m_DBuf;
                m_DBufLen = rstMPK.m_DBufLen;

                m_SBufLen = 0;

                rstMPK.m_DBuf = nullptr;
                rstMPK.m_DBufLen = 0;
                return;
            }

            // case-2: use static buffer, copy only
            //         but after this call invalidate rstMPK by delete its data
            if(rstMPK.m_SBufLen){
                m_SBufLen = rstMPK.m_SBufLen;
                std::memcpy(m_SBuf, rstMPK.m_SBuf, m_SBufLen);

                m_DBuf = nullptr;
                m_DBufLen = 0;

                rstMPK.m_SBufLen = 0;
                return;
            }

            // case-3: empty message
            //         shouldn't happen here
            {
                m_SBufLen = 0;

                m_DBuf = nullptr;
                m_DBufLen = 0;
                return;
            }
        }

    public:
        ~InnMessagePack()
        {
            delete [] m_DBuf;
        }

    public:
       InnMessagePack & operator = (InnMessagePack stMPK)
       {
           std::swap(m_type   , stMPK.m_type   );
           std::swap(m_from   , stMPK.m_from   );
           std::swap(m_ID     , stMPK.m_ID     );
           std::swap(m_respond, stMPK.m_respond);

           std::swap(m_SBufLen, stMPK.m_SBufLen);
           std::swap(m_DBuf   , stMPK.m_DBuf   );
           std::swap(m_DBufLen, stMPK.m_DBufLen);

           if(m_SBufLen){
               std::memcpy(m_SBuf, stMPK.m_SBuf, m_SBufLen);
           }

           return *this;
       }

       operator bool () const
       {
           return Type() != MPK_NONE;
       }

    public:
       template<typename T> void assign(T &t) const
       {
           static_assert(std::is_trivially_copyable_v<T>);
           if(sizeof(T) != DataLen()){
               throw fflerror("size mismatch, using wrong POD type");
           }
           std::memcpy(&t, Data(), sizeof(t));
       }

       template<typename T> T conv() const
       {
           T t;
           assign<T>(t);
           return t;
       }

    public:
        int Type() const
        {
            return m_type;
        }

        const uint8_t *Data() const
        {
            return m_SBufLen ? m_SBuf : m_DBuf;
        }

        size_t DataLen() const
        {
            return m_SBufLen ? m_SBufLen : m_DBufLen;
        }

        size_t Size() const
        {
            return DataLen();
        }

        uint64_t from() const
        {
            return m_from;
        }

        uint32_t ID() const
        {
            return m_ID;
        }

        uint32_t Respond() const
        {
            return m_respond;
        }

        const char *Name() const
        {
            switch(m_type){
                case MPK_NONE                : return "MPK_NONE";
                case MPK_OK                  : return "MPK_OK";
                case MPK_ERROR               : return "MPK_ERROR";
                case MPK_BADACTORPOD         : return "MPK_BADACTORPOD";
                case MPK_BADCHANNEL          : return "MPK_BADCHANNEL";
                case MPK_TIMEOUT             : return "MPK_TIMEOUT";
                case MPK_UID                 : return "MPK_UID";
                case MPK_PING                : return "MPK_PING";
                case MPK_LOGIN               : return "MPK_LOGIN";
                case MPK_METRONOME           : return "MPK_METRONOME";
                case MPK_TRYMOVE             : return "MPK_TRYMOVE";
                case MPK_TRYSPACEMOVE        : return "MPK_TRYSPACEMOVE";
                case MPK_MOVEOK              : return "MPK_MOVEOK";
                case MPK_SPACEMOVEOK         : return "MPK_SPACEMOVEOK";
                case MPK_TRYLEAVE            : return "MPK_TRYLEAVE";
                case MPK_LOGINOK             : return "MPK_LOGINOK";
                case MPK_ADDRESS             : return "MPK_ADDRESS";
                case MPK_LOGINQUERYDB        : return "MPK_LOGINQUERYDB";
                case MPK_NETPACKAGE          : return "MPK_NETPACKAGE";
                case MPK_ADDCHAROBJECT       : return "MPK_ADDCHAROBJECT";
                case MPK_BINDCHANNEL         : return "MPK_BINDCHANNEL";
                case MPK_ACTION              : return "MPK_ACTION";
                case MPK_PULLCOINFO          : return "MPK_PULLCOINFO";
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
                case MPK_QUERYRECTUIDLIST    : return "MPK_QUERYRECTUIDLIST";
                case MPK_UIDLIST             : return "MPK_UIDLIST";
                case MPK_EXP                 : return "MPK_EXP";
                case MPK_MISS                : return "MPK_MISS";
                case MPK_NEWDROPITEM         : return "MPK_NEWDROPITEM";
                case MPK_SHOWDROPITEM        : return "MPK_SHOWDROPITEM";
                case MPK_NOTIFYDEAD          : return "MPK_NOTIFYDEAD";
                case MPK_OFFLINE             : return "MPK_OFFLINE";
                case MPK_PICKUP              : return "MPK_PICKUP";
                case MPK_PICKUPOK            : return "MPK_PICKUPOK";
                case MPK_REMOVEGROUNDITEM    : return "MPK_REMOVEGROUNDITEM";
                case MPK_CORECORD            : return "MPK_CORECORD";
                case MPK_NOTIFYNEWCO         : return "MPK_NOTIFYNEWCO";
                case MPK_CHECKMASTER         : return "MPK_CHECKMASTER";
                case MPK_QUERYMASTER         : return "MPK_QUERYMASTER";
                case MPK_QUERYFINALMASTER    : return "MPK_QUERYFINALMASTER";
                case MPK_QUERYFRIENDTYPE     : return "MPK_QUERYFRIENDTYPE";
                case MPK_FRIENDTYPE          : return "MPK_FRIENDTYPE";
                case MPK_QUERYNAMECOLOR      : return "MPK_QUERYNAMECOLOR";
                case MPK_NAMECOLOR           : return "MPK_NAMECOLOR";
                case MPK_MASTERKILL          : return "MPK_MASTERKILL";
                case MPK_NPCEVENT            : return "MPK_NPCEVENT";
                case MPK_NPCXMLLAYOUT        : return "MPK_NPCXMLLAYOUT";
                case MPK_NPCERROR            : return "MPK_NPCERROR";
                default                      : return "MPK_UNKNOWN";
            }
        }
};

using MessagePack = InnMessagePack<64>;
