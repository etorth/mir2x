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
#include "actormsg.hpp"

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
};

inline const char *mpkName(int type)
{
#define _add_mpk_type_case(t) case t: return #t;
    switch(type){
        _add_mpk_type_case(MPK_NONE            )
        _add_mpk_type_case(MPK_OK              )
        _add_mpk_type_case(MPK_ERROR           )
        _add_mpk_type_case(MPK_BADACTORPOD     )
        _add_mpk_type_case(MPK_BADCHANNEL      )
        _add_mpk_type_case(MPK_TIMEOUT         )
        _add_mpk_type_case(MPK_UID             )
        _add_mpk_type_case(MPK_PING            )
        _add_mpk_type_case(MPK_LOGIN           )
        _add_mpk_type_case(MPK_METRONOME       )
        _add_mpk_type_case(MPK_TRYMOVE         )
        _add_mpk_type_case(MPK_TRYSPACEMOVE    )
        _add_mpk_type_case(MPK_MOVEOK          )
        _add_mpk_type_case(MPK_SPACEMOVEOK     )
        _add_mpk_type_case(MPK_TRYLEAVE        )
        _add_mpk_type_case(MPK_LOGINOK         )
        _add_mpk_type_case(MPK_ADDRESS         )
        _add_mpk_type_case(MPK_LOGINQUERYDB    )
        _add_mpk_type_case(MPK_NETPACKAGE      )
        _add_mpk_type_case(MPK_ADDCHAROBJECT   )
        _add_mpk_type_case(MPK_BINDCHANNEL     )
        _add_mpk_type_case(MPK_ACTION          )
        _add_mpk_type_case(MPK_PULLCOINFO      )
        _add_mpk_type_case(MPK_QUERYMAPLIST    )
        _add_mpk_type_case(MPK_MAPLIST         )
        _add_mpk_type_case(MPK_MAPSWITCH       )
        _add_mpk_type_case(MPK_MAPSWITCHOK     )
        _add_mpk_type_case(MPK_TRYMAPSWITCH    )
        _add_mpk_type_case(MPK_QUERYMAPUID     )
        _add_mpk_type_case(MPK_QUERYLOCATION   )
        _add_mpk_type_case(MPK_LOCATION        )
        _add_mpk_type_case(MPK_PATHFIND        )
        _add_mpk_type_case(MPK_PATHFINDOK      )
        _add_mpk_type_case(MPK_ATTACK          )
        _add_mpk_type_case(MPK_UPDATEHP        )
        _add_mpk_type_case(MPK_DEADFADEOUT     )
        _add_mpk_type_case(MPK_QUERYCORECORD   )
        _add_mpk_type_case(MPK_QUERYCOCOUNT    )
        _add_mpk_type_case(MPK_COCOUNT         )
        _add_mpk_type_case(MPK_EXP             )
        _add_mpk_type_case(MPK_MISS            )
        _add_mpk_type_case(MPK_NEWDROPITEM     )
        _add_mpk_type_case(MPK_SHOWDROPITEM    )
        _add_mpk_type_case(MPK_NOTIFYDEAD      )
        _add_mpk_type_case(MPK_OFFLINE         )
        _add_mpk_type_case(MPK_PICKUP          )
        _add_mpk_type_case(MPK_PICKUPOK        )
        _add_mpk_type_case(MPK_REMOVEGROUNDITEM)
        _add_mpk_type_case(MPK_CORECORD        )
        _add_mpk_type_case(MPK_NOTIFYNEWCO     )
        _add_mpk_type_case(MPK_CHECKMASTER     )
        _add_mpk_type_case(MPK_QUERYMASTER     )
        _add_mpk_type_case(MPK_QUERYFINALMASTER)
        _add_mpk_type_case(MPK_QUERYFRIENDTYPE )
        _add_mpk_type_case(MPK_FRIENDTYPE      )
        _add_mpk_type_case(MPK_QUERYNAMECOLOR  )
        _add_mpk_type_case(MPK_NAMECOLOR       )
        _add_mpk_type_case(MPK_MASTERKILL      )
        _add_mpk_type_case(MPK_NPCEVENT        )
        _add_mpk_type_case(MPK_NPCXMLLAYOUT    )
        _add_mpk_type_case(MPK_NPCERROR        )
        default: return "MPK_UNKNOWN";
    }
#undef _add_mpk_type_case
}

using MessagePack = InnMessagePack<64>;
