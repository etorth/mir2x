/*
 * =====================================================================================
 *
 *       Filename: syncdriver.cpp
 *        Created: 06/09/2016 17:32:50
 *  Last Modified: 06/13/2017 22:48:24
 *
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

#include <cinttypes>
#include "serverenv.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"

// send without waiting for response
// also for SyncDriver we have no registed response handler
//
// return value:
//      0. no error
//      1. send failed
int SyncDriver::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->MIR2X_DEBUG_PRINT_AM_FORWARD){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "(Driver: 0X%0*" PRIXPTR ", Name: SyncDriver, UID: NA) -> (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nRespond);
    }
    extern Theron::Framework *g_Framework;
    return g_Framework->Send<MessagePack>({rstMB, 0, nRespond}, m_Receiver.GetAddress(), rstAddr) ? 0 : 1;
}

// send with expection of response message, this function firstly clear all cached
// message in the receiver, then send it and wait to the response.
//
// TODO it's very easy to get blocked if we don't put timeout support here. we can
// use Theron::Receiver::Comsume() instead of Theron::Receiver::Wait() to support
// the timeout functionality
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
int SyncDriver::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond, MessagePack *pMPK)
{
    MessagePack stTmpMPK;
    Theron::Address stTmpAddress;

    // 1. clean the catcher
    //    this clean all cached messages in the receiver
    //    or use m_Receiver.Reset()
    while(true){
        if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){
            break;
        }
    }

    // to avoid the overflow
    // but never check the uniqueness when wrap back
    m_ValidID = (m_ValidID + 1) ? (m_ValidID + 1) : 1;
    auto nCurrID = m_ValidID;

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->MIR2X_DEBUG_PRINT_AM_FORWARD){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "(Driver: 0X%0*" PRIXPTR ", Name: SyncDriver, UID: NA) -> (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nCurrID, nRespond);
    }

    // 2. send message
    extern Theron::Framework *g_Framework;
    if(!g_Framework->Send<MessagePack>({rstMB, nCurrID, nRespond}, m_Receiver.GetAddress(), rstAddr)){
        // 3. ooops send failed
        //    won't print any warning message since we take this as ``normal"
        return 1;
    }

    // 4. send succeed, wait for response
    while(true){
        if(m_Receiver.Wait(1) != 1){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "(SyncDriver 0X%0*" PRIXPTR "::Wait(1) failed", (int)(sizeof(this) * 2), (uintptr_t)(this));
            return 2;
        }

        // handle response
        // now we already has >= 1 response in catcher
        if(!m_Catcher.Pop(stTmpMPK, stTmpAddress)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "(SyncDriver 0X%0*" PRIXPTR "::Pop() failed", (int)(sizeof(this) * 2), (uintptr_t)(this));
            return 3;
        }

        // since the syncdriver may send / receive multiple messages
        // here it could mess up with other actor's response or the receiver's last response
        //      1. check stTmpAddress == rstAddr
        //      2. check stTmpMPK.Respond() == nCurrID
        // method-2 is always OK
        // method-1 could failed if trying to send to an dead actor and stTmpAddress is of the g_Framework
        if(stTmpMPK.Respond() != nCurrID){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "(SyncDriver 0X%0*" PRIXPTR " get ID = %" PRIu32 ", expected ID = %" PRIu32, (int)(sizeof(this) * 2), (uintptr_t)(this), stTmpMPK.Respond(), nCurrID);
            continue;
        }

        // ok finally we get what we want
        if(pMPK){ *pMPK = stTmpMPK; }

        return 0;
    }

    // 5. mysterious errors...
    return 4;
}
