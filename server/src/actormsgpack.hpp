#pragma once
#include <cstring>
#include <cstdint>
#include <utility>
#include <type_traits>
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "actormsg.hpp"
#include "actormsgbuf.hpp"

template<size_t SBUFSIZE = 64> class InnActorMsgPack final
{
    private:
        int m_type;

    private:
        uint64_t m_from;

    private:
        uint64_t m_seqID;
        uint64_t m_respID;

    private:
        uint8_t m_sbuf[SBUFSIZE];
        size_t  m_sbufSize;

    private:
        uint8_t *m_dbuf;
        size_t   m_dbufSize;

    public:
        InnActorMsgPack(int argType = AM_NONE, const uint8_t *argData = nullptr, size_t argSize = 0, uint64_t argFrom = 0, uint64_t argSeqID = 0, uint64_t argRespID = 0)
            : m_type(argType)
            , m_from(argFrom)
            , m_seqID(argSeqID)
            , m_respID(argRespID)
        {
            if(argData && argSize){
                if(argSize <= SBUFSIZE){
                    m_sbufSize = argSize;
                    std::memcpy(m_sbuf, argData, argSize);
                    m_dbuf = nullptr;
                    m_dbufSize = 0;
                }
                else{
                    m_dbuf = new uint8_t[argSize];
                    m_dbufSize = argSize;
                    std::memcpy(m_dbuf, argData, argSize);
                    m_sbufSize = 0;
                }
            }
            else{
                m_dbuf = nullptr;
                m_dbufSize = 0;
                m_sbufSize = 0;
            }
        }

        InnActorMsgPack(const ActorMsgBuf &mbuf, uint64_t argFrom = 0, uint64_t argSeqID = 0, uint64_t argRespID = 0)
            : InnActorMsgPack(mbuf.type(), mbuf.data(), mbuf.size(), argFrom, argSeqID, argRespID)
        {}

        InnActorMsgPack(const InnActorMsgPack &mpk)
            : InnActorMsgPack(mpk.type(), mpk.data(), mpk.size(), mpk.from(), mpk.seqID(), mpk.respID())
        {}

        InnActorMsgPack(InnActorMsgPack &&mpk)
            : m_type(mpk.type())
            , m_from(mpk.from())
            , m_seqID(mpk.seqID())
            , m_respID(mpk.respID())
        {
            // invalidate the r-ref message by
            // 1. remove its from/id/resp
            // 2. remove its buffer

            mpk.m_type   = AM_NONE;
            mpk.m_from   = 0;
            mpk.m_seqID  = 0;
            mpk.m_respID = 0;

            // case-1: use dynamic buffer, steal the buffer
            //         after this call mpk should be destructed immediately
            if(mpk.m_dbuf && mpk.m_dbufSize){
                m_dbuf = mpk.m_dbuf;
                m_dbufSize = mpk.m_dbufSize;
                m_sbufSize = 0;
                mpk.m_dbuf = nullptr;
                mpk.m_dbufSize = 0;
                return;
            }

            // case-2: use static buffer, copy only
            //         but after this call invalidate mpk by delete its data
            if(mpk.m_sbufSize){
                m_sbufSize = mpk.m_sbufSize;
                std::memcpy(m_sbuf, mpk.m_sbuf, m_sbufSize);
                m_dbuf = nullptr;
                m_dbufSize = 0;
                mpk.m_sbufSize = 0;
                return;
            }

            // case-3: empty message
            //         shouldn't happen here
            {
                m_sbufSize = 0;
                m_dbuf = nullptr;
                m_dbufSize = 0;
                return;
            }
        }

    public:
        ~InnActorMsgPack()
        {
            delete [] m_dbuf;
        }

    public:
        InnActorMsgPack & operator = (InnActorMsgPack mpk)
        {
            std::swap(m_type  , mpk.m_type  );
            std::swap(m_from  , mpk.m_from  );
            std::swap(m_seqID , mpk.m_seqID );
            std::swap(m_respID, mpk.m_respID);

            std::swap(m_sbufSize, mpk.m_sbufSize);
            std::swap(m_dbuf    , mpk.m_dbuf    );
            std::swap(m_dbufSize, mpk.m_dbufSize);

            if(m_sbufSize){
                std::memcpy(m_sbuf, mpk.m_sbuf, m_sbufSize);
            }
            return *this;
        }

        operator bool () const
        {
            return type() != AM_NONE;
        }

    public:
       template<typename T> T conv() const
       {
           static_assert(std::is_trivially_copyable_v<T>);
           fflassert(sizeof(T) == size());

           T t;
           std::memcpy(&t, data(), sizeof(t));
           return t;
       }

       template<typename T> T deserialize() const
       {
           return cerealf::deserialize<T>(data(), size());
       }

    public:
       int type() const
       {
           return m_type;
       }

       const uint8_t *data() const
       {
           return m_sbufSize ? m_sbuf : m_dbuf;
       }

       size_t size() const
       {
           return m_sbufSize ? m_sbufSize : m_dbufSize;
       }

       uint64_t from() const
       {
           return m_from;
       }

       std::pair<uint64_t, uint64_t> fromAddr() const
       {
           return {m_from, m_seqID};
       }

       uint64_t seqID() const
       {
           return m_seqID;
       }

       uint64_t respID() const
       {
           return m_respID;
       }
};

inline const char *mpkName(int type)
{
#define _add_mpk_type_case(t) case t: return #t;
    switch(type){
        _add_mpk_type_case(AM_NONE)
        _add_mpk_type_case(AM_OK)
        _add_mpk_type_case(AM_ERROR)
        _add_mpk_type_case(AM_BADACTORPOD)
        _add_mpk_type_case(AM_BADCHANNEL)
        _add_mpk_type_case(AM_SDBUFFER)
        _add_mpk_type_case(AM_REMOTECALL)
        _add_mpk_type_case(AM_TIMEOUT)
        _add_mpk_type_case(AM_UID)
        _add_mpk_type_case(AM_UIDLIST)
        _add_mpk_type_case(AM_PING)
        _add_mpk_type_case(AM_LOGIN)
        _add_mpk_type_case(AM_METRONOME)
        _add_mpk_type_case(AM_TRYJUMP)
        _add_mpk_type_case(AM_ALLOWJUMP)
        _add_mpk_type_case(AM_REJECTJUMP)
        _add_mpk_type_case(AM_JUMPOK)
        _add_mpk_type_case(AM_JUMPERROR)
        _add_mpk_type_case(AM_TRYMOVE)
        _add_mpk_type_case(AM_ALLOWMOVE)
        _add_mpk_type_case(AM_REJECTMOVE)
        _add_mpk_type_case(AM_MOVEOK)
        _add_mpk_type_case(AM_MOVEERROR)
        _add_mpk_type_case(AM_TRYSPACEMOVE)
        _add_mpk_type_case(AM_ALLOWSPACEMOVE)
        _add_mpk_type_case(AM_REJECTSPACEMOVE)
        _add_mpk_type_case(AM_SPACEMOVEOK)
        _add_mpk_type_case(AM_SPACEMOVEERROR)
        _add_mpk_type_case(AM_TRYLEAVE)
        _add_mpk_type_case(AM_ALLOWLEAVE)
        _add_mpk_type_case(AM_REJECTLEAVE)
        _add_mpk_type_case(AM_LEAVEOK)
        _add_mpk_type_case(AM_LEAVEERROR)
        _add_mpk_type_case(AM_FINISHLEAVE)
        _add_mpk_type_case(AM_LOGINOK)
        _add_mpk_type_case(AM_LOGINQUERYDB)
        _add_mpk_type_case(AM_SENDPACKAGE)
        _add_mpk_type_case(AM_RECVPACKAGE)
        _add_mpk_type_case(AM_ADDCO)
        _add_mpk_type_case(AM_BINDCHANNEL)
        _add_mpk_type_case(AM_ACTION)
        _add_mpk_type_case(AM_QUERYMAPLIST)
        _add_mpk_type_case(AM_MAPLIST)
        _add_mpk_type_case(AM_MAPSWITCHTRIGGER)
        _add_mpk_type_case(AM_TRYMAPSWITCH)
        _add_mpk_type_case(AM_ALLOWMAPSWITCH)
        _add_mpk_type_case(AM_REJECTMAPSWITCH)
        _add_mpk_type_case(AM_MAPSWITCHOK)
        _add_mpk_type_case(AM_MAPSWITCHERROR)
        _add_mpk_type_case(AM_LOADMAP)
        _add_mpk_type_case(AM_LOADMAPOK)
        _add_mpk_type_case(AM_LOADMAPERROR)
        _add_mpk_type_case(AM_QUERYLOCATION)
        _add_mpk_type_case(AM_QUERYSELLITEMLIST)
        _add_mpk_type_case(AM_LOCATION)
        _add_mpk_type_case(AM_PATHFIND)
        _add_mpk_type_case(AM_PATHFINDOK)
        _add_mpk_type_case(AM_ATTACK)
        _add_mpk_type_case(AM_UPDATEHP)
        _add_mpk_type_case(AM_DEADFADEOUT)
        _add_mpk_type_case(AM_QUERYUIDBUFF)
        _add_mpk_type_case(AM_QUERYCORECORD)
        _add_mpk_type_case(AM_QUERYCOCOUNT)
        _add_mpk_type_case(AM_QUERYPLAYERWLDESP)
        _add_mpk_type_case(AM_COCOUNT)
        _add_mpk_type_case(AM_ADDBUFF)
        _add_mpk_type_case(AM_REMOVEBUFF)
        _add_mpk_type_case(AM_EXP)
        _add_mpk_type_case(AM_MISS)
        _add_mpk_type_case(AM_HEAL)
        _add_mpk_type_case(AM_QUERYHEALTH)
        _add_mpk_type_case(AM_HEALTH)
        _add_mpk_type_case(AM_DROPITEM)
        _add_mpk_type_case(AM_SHOWDROPITEM)
        _add_mpk_type_case(AM_NOTIFYDEAD)
        _add_mpk_type_case(AM_OFFLINE)
        _add_mpk_type_case(AM_PICKUP)
        _add_mpk_type_case(AM_PICKUPITEMLIST)
        _add_mpk_type_case(AM_REMOVEGROUNDITEM)
        _add_mpk_type_case(AM_CORECORD)
        _add_mpk_type_case(AM_NOTIFYNEWCO)
        _add_mpk_type_case(AM_CHECKMASTER)
        _add_mpk_type_case(AM_CHECKMASTEROK)
        _add_mpk_type_case(AM_QUERYMASTER)
        _add_mpk_type_case(AM_QUERYFINALMASTER)
        _add_mpk_type_case(AM_QUERYFRIENDTYPE)
        _add_mpk_type_case(AM_FRIENDTYPE)
        _add_mpk_type_case(AM_CASTFIREWALL)
        _add_mpk_type_case(AM_STRIKEFIXEDLOCDAMAGE)
        _add_mpk_type_case(AM_QUERYNAMECOLOR)
        _add_mpk_type_case(AM_NAMECOLOR)
        _add_mpk_type_case(AM_MASTERKILL)
        _add_mpk_type_case(AM_MASTERHITTED)
        _add_mpk_type_case(AM_NPCEVENT)
        _add_mpk_type_case(AM_NPCERROR)
        _add_mpk_type_case(AM_BUY)
        _add_mpk_type_case(AM_BUYCOST)
        _add_mpk_type_case(AM_BUYERROR)
        _add_mpk_type_case(AM_MODIFYQUESTTRIGGERTYPE)
        _add_mpk_type_case(AM_QUERYQUESTUID)
        _add_mpk_type_case(AM_QUERYQUESTUIDLIST)
        _add_mpk_type_case(AM_QUERYQUESTTRIGGERLIST)
        _add_mpk_type_case(AM_RUNQUESTTRIGGER)
        _add_mpk_type_case(AM_SENDNOTIFY)
        _add_mpk_type_case(AM_REGISTERQUEST)
        default: return "AM_UNKNOWN";
    }
#undef _add_mpk_type_case
}

using ActorMsgPack = InnActorMsgPack<64>;
