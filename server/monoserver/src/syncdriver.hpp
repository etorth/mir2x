/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *  Last Modified: 05/02/2016 00:10:32
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
        // TODO
        // it's very easy to be blocked if we don't put a timeout functionality here

        // send without waiting for response
        // also for SyncDriver we have no registed response handler
        //
        // return value:
        //      0. no error
        //      1. send failed
        int Send(MessagePack rstMPK, const Theron::Address &rstAddress)
        {
            rstMPK.ID(0);
            rstMPK.Respond(0);

            extern Theron::Framework *g_Framework;
            return g_Framework->Send(rstMPK, m_Receiver.GetAddress(), rstAddress) ? 0 : 1;
        }

        // send with expection of response message
        // input argument
        //      rstMPK      :
        //      rstAddress  :
        //      pMPK        : to copy the respond message out
        //                    null if we need reponse but don't care what's the response is
        //
        // define error code of return:
        //      0   : no error
        //      1   : send failed
        //      2   : send succeed but wait for response failed
        //      3   : other unknown errors
        int Send(MessagePack rstMPK, const Theron::Address &rstAddress, MessagePack *pMPK)
        {
            MessagePack stTmpMPK;
            Theron::Address stTmpAddress;

            // 1. clean the cather
            while(true){ if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){ break; } }

            // 2. send message
            extern Theron::Framework *g_Framework;
            bool bSendRet = g_Framework->Send(rstMPK, m_Receiver.GetAddress(), rstAddress);

            // 3. ooops send failed
            if(!bSendRet){ return 1; }

            // 4. send succeed, wait for response
            uint32_t nWaitRet = m_Receiver.Wait(1);

            if(nWaitRet != 1){ return 2; }

            // 6. handle response
            //    now we already has 1 response in catcher, just copy it out
            if(m_Catcher.Pop(stTmpMPK, stTmpAddress) && stTmpAddress == rstAddress){
                if(pMPK){ *pMPK = stTmpMPK; }
                return 0;
            }

            // 7. mysterious errors...
            return 3;
        }
};
