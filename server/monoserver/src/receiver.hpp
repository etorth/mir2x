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
#include <cstdint>

class Receiver
{
    private:
        uint64_t m_UID;

    private:
        std::mutex m_Lock;
        std::condition_variable m_Condition;

    private:
        std::vector<MessagePack> m_MessageList;

    public:
        Receiver();

    public:
        ~Receiver();

    private:
        void PushMessage(const MessagePack *, size_t);

    public:
        int Wait(uint32_t = 0);
};
