/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *  Last Modified: 06/09/2016 17:35:14
 *
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
#include <Theron/Theron.h>

#include "messagepack.hpp"

class SyncDriver
{
    protected:
        Theron::Receiver m_Receiver;
        Theron::Catcher<MessagePack> m_Catcher;

    public:
        SyncDriver()
        {
            m_Receiver.RegisterHandler(&m_Catcher, &Theron::Catcher<MessagePack>::Push);
        }

        virtual ~SyncDriver() = default;

    public:
        int Forward(const MessageBuf &, const Theron::Address &);
        int Forward(const MessageBuf &, const Theron::Address &, MessagePack *);
};
