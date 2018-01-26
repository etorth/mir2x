/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *    Description: class which behaves as:
 *                      ``send-wait-receive-action-.....-send-wait-receive-action..."
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
#include <Theron/Theron.h>
#include "messagepack.hpp"

class SyncDriver
{
    protected:
        // syncdriver doesn't have a respond handler map to handle the response
        // alternatively it does a synchronized way to wait for the response
        // but this could introduce a bug as following:
        //
        // 1. A -> B and not expecting response
        // 2. A -> B and not expecting response R2
        // 3. A is waiting for R2, but B responds with R1 for the first step
        //
        // this breaks internal logic for A
        // then we use m_ValidID to make sure it's responding as expected
        uint32_t m_ValidID;

    protected:
        Theron::Receiver m_Receiver;
        Theron::Catcher<MessagePack> m_Catcher;

    public:
        SyncDriver()
            : m_ValidID(1)
            , m_Receiver()
            , m_Catcher()
        {
            m_Receiver.RegisterHandler(&m_Catcher, &Theron::Catcher<MessagePack>::Push);
        }

        virtual ~SyncDriver() = default;

    public:
        int Forward(const MessageBuf &, const Theron::Address &, uint32_t);
        int Forward(const MessageBuf &, const Theron::Address &, uint32_t, MessagePack *);

    public:
        int Forward(const MessageBuf &rstMB, const Theron::Address &rstAddress)
        {
            return Forward(rstMB, rstAddress, (uint32_t)(0));
        }

        int Forward(const MessageBuf &rstMB, const Theron::Address &rstAddress, MessagePack *pMPK)
        {
            return Forward(rstMB, rstAddress, (uint32_t)(0), pMPK);
        }
};
