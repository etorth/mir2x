/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *  Last Modified: 05/27/2016 18:43:02
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
        // send without waiting for response
        // also for SyncDriver we have no registed response handler
        //
        // return value:
        //      0. no error
        //      1. send failed
        int Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr)
        {
            extern Theron::Framework *g_Framework;
            return g_Framework->Send<MessagePack>(
                    {rstMB, 0, 0}, m_Receiver.GetAddress(), rstAddr) ? 0 : 1;
        }

        // send with expection of response message. this function firstly clear all cached
        // message in the receiver, then send 1 and wait to the response, TODO there could
        // be a potential bug that A send B a message without waiting for response, then A
        // send B a message without waiting for response, however, B reply A incorrectly 
        // for the first sending, and this response was caught during the waiting in the 
        // second sending, then it's bug
        //
        // so if we sending a message to a actor without expection of a repsonse, we use
        // SyncDriver().Forward(MPK, Addr); this would never cause problem since everytime
        // the receiver's address is different (not sure, it's randomly generated)
        //
        // TODO
        // it's very easy to be blocked if we don't put a timeout functionality here
        //
        // input argument
        //      rstMPK      :
        //      rstAddr     :
        //      pMPK        : to copy the respond message out
        //                    null if we need reponse but don't care what's the response is
        //
        // define error code of return:
        //      0   : no error
        //      1   : send failed
        //      2   : send succeed but wait for response failed
        //      3   : fail to pop the received message
        //      4   : mysterious error, can rarely happen
        int Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, MessagePack *pMPK)
        {
            MessagePack stTmpMPK;
            Theron::Address stTmpAddress;

            // 1. clean the catcher
            while(true){ if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){ break; } }

            // 2. send message
            extern Theron::Framework *g_Framework;
            bool bSendRet = g_Framework->Send<MessagePack>(
                    {rstMB, 1, 0}, m_Receiver.GetAddress(), rstAddr);

            // 3. ooops send failed
            if(!bSendRet){ return 1; }

            // 4. send succeed, wait for response
            while(true){
                uint32_t nWaitRet = m_Receiver.Wait(1);

                if(nWaitRet != 1){ return 2; }

                // handle response
                // now we already has >= 1 response in catcher
                if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){ return 3; }

                // since the syncdriver may send / receive multiple messages
                // and we got a message from other than rstAddr occasioinally
                if(stTmpAddress != rstAddr){ continue; }

                // ok finally we get what we want
                if(pMPK){ *pMPK = stTmpMPK; }

                return 0;
            }

            // 5. mysterious errors...
            return 4;
        }
};
