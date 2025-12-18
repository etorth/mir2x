#pragma once
#include <cstring>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <optional>
#include "cerealf.hpp"
#include "uidf.hpp"
#include "strf.hpp"
#include "fflerror.hpp"
#include "actormsg.hpp"
#include "actormsgbuf.hpp"
#include "totype.hpp"

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

    public:
       ActorMsgBuf msgBuf() const
       {
           return ActorMsgBuf(type(), data(), size());
       }

    public:
       template<class Archive> void save(Archive & ar) const
       {
           ar(m_type, m_from, m_seqID, m_respID);
           ar(size());
           ar(cereal::binary_data(data(), size()));
       }

       template<class Archive> void load(Archive & ar)
       {
           ar(m_type, m_from, m_seqID, m_respID);

           size_t size = 0;
           ar(size);

           if(size <= SBUFSIZE){
               m_sbufSize = size;
               m_dbufSize = 0;
               ar(cereal::binary_data(m_sbuf, size));
           }
           else{
               m_sbufSize = 0;
               m_dbufSize = size;
               m_dbuf = new uint8_t[size];
               ar(cereal::binary_data(m_dbuf, size));
           }
       }

    public:
       std::string str(std::optional<uint64_t> toOpt = std::nullopt) const
       {
           return str_printf("{type:%s, from:%s, %sseqID:%llu, respID:%llu, size:%llu}",
                   mpkName(type()),
                   to_cstr(uidf::getUIDString(from())),
                   toOpt.has_value() ? str_printf("to:%s, ", to_cstr(uidf::getUIDString(toOpt.value()))).c_str() : "",
                   to_llu(seqID()),
                   to_llu(respID()),
                   to_llu(size()));
       }
};

using ActorMsgPack = InnActorMsgPack<64>;
