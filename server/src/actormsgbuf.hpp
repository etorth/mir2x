#pragma once
#include <cstdint>
#include <type_traits>
#include "actormsg.hpp"

class ActorMsgBuf final
{
    private:
        int            m_type;
        const uint8_t *m_data;
        size_t         m_size;

    public:
        // bad message
        ActorMsgBuf()
            : ActorMsgBuf(AM_NONE)
        {}

        // message descriptor without body
        ActorMsgBuf(int argType)
            : ActorMsgBuf(argType, nullptr, 0)
        {}

        // message descriptor with body
        // if we provide body, we need to supply both pointer and length
        ActorMsgBuf(int argType, const uint8_t *argData, size_t argSize)
            : m_type(argType)
            , m_data(argData)
            , m_size(argSize)
        {}

        ActorMsgBuf(int argType, const std::string &argBuf)
            : ActorMsgBuf(argType, (const uint8_t *)(argBuf.data()), argBuf.size())
        {}

        // actually you can put a pointer here
        // which makes a ActorMsgPack contain a pointer value
        //
        // since we defined ActorMsgBuf(int, const uint8_t *, size_t)
        // this won't cause ambiguity
        template<typename T> ActorMsgBuf(int argType, const T &t)
            : ActorMsgBuf(argType, (const uint8_t *)(&t), sizeof(t))
        {
            static_assert(std::is_trivially_copyable_v<T>);
        }

    public:
        operator bool () const
        {
            return type() != AM_NONE;
        }

    public:
        int type() const
        {
            return m_type;
        }

        const uint8_t *data() const
        {
            return m_data;
        }

        size_t size() const
        {
            return m_size;
        }
};
