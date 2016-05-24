/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *  Last Modified: 05/23/2016 10:48:02
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

        // send with expection of response message
        // TODO
        // it's very easy to be blocked if we don't put a timeout functionality here
        // input argument
        //      rstMPK      :
        //      rstAddr  :
        //      pMPK        : to copy the respond message out
        //                    null if we need reponse but don't care what's the response is
        //
        // define error code of return:
        //      0   : no error
        //      1   : send failed
        //      2   : send succeed but wait for response failed
        //      3   : other unknown errors
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
            uint32_t nWaitRet = m_Receiver.Wait(1);

            if(nWaitRet != 1){ return 2; }

            // 6. handle response
            //    now we already has 1 response in catcher, just copy it out
            if(m_Catcher.Pop(stTmpMPK, stTmpAddress) && stTmpAddress == rstAddr){
                if(pMPK){ *pMPK = stTmpMPK; }
                return 0;
            }

            // 7. mysterious errors...
            return 3;
        }
};
