/*
 * =====================================================================================
 *
 *       Filename: receiver.hpp
 *        Created: 08/23/2018 04:44:33
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
#include <vector>
#include <cstdint>
#include <condition_variable>
#include "messagepack.hpp"

class Receiver
{
    private:
        friend class ActorPool;

    private:
        uint64_t m_UID;

    private:
        std::mutex m_lock;
        std::condition_variable m_condition;

    private:
        std::vector<MessagePack> m_messageList;

    public:
        Receiver();

    public:
        ~Receiver();

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    private:
        void pushMessage(MessagePack);

    public:
        size_t Wait(uint32_t = 0);

    public:
        std::vector<MessagePack> Pop();
};
