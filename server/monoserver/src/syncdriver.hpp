/*
 * =====================================================================================
 *
 *       Filename: syncdriver.hpp
 *        Created: 04/27/2016 00:28:05
 *  Last Modified: 04/27/2016 22:28:31
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
        // define error code of return:
        //      0   : no error
        //      1   : send failed
        //      2   : send succeed but wait for response failed
        //      3   : other unknown errors
        int Send(const MessagePack &rstMPK,
                const Theron::Address &rstAddress, MessagePack *pMPK = nullptr)
        {
            MessagePack stTmpMPK;
            Theron::Address stTmpAddress;

            // 1. clean the cather
            while(true){ if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){ break; } }

            // 2. send message
            extern Theron::Framework *g_Framework;
            bool bSendRet = g_Framework->Send(rstMPK, m_Receiver.GetAddress(), rstAddress);

            // 3. if no requirement of response, return
            if(!(rstMPK.Respond() && bSendRet)){ return bSendRet ? 1 : 0; }

            // 4. wait for response
            uint32_t nWaitRet = m_Receiver.Wait(1);

            // 5. no room for copy, return
            //    now we know send is successful
            if(!(pMPK && nWaitRet)){ return (nWaitRet == 1) ? 0 : 2; }

            // 5. handle response
            //    now we already has 1 response in catcher, just copy it out
            if(m_Catcher.Pop(stTmpMPK, stTmpAddress) && stTmpAddress == rstAddress){
                // so if error occurs at the last step, we still keep pMPK unchanged
                if(pMPK){ *pMPK = stTmpMPK; }
                return 0;
            }

            // 6. mysterious errors...
            return 3;
        }
};
