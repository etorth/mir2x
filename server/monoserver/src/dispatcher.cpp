/*
 * =====================================================================================
 *
 *       Filename: dispatcher.cpp
 *        Created: 01/26/2018 15:37:14
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
#include "dispatcher.hpp"
#include "messagepack.hpp"

bool Dispatcher::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "(Dispatcher: 0X%0*" PRIXPTR ", Name: Dispatcher, UID: NA) -> (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    extern Theron::Framework *g_Framework;
    return g_Framework->Send<MessagePack>({rstMB, 0, nRespond}, Theron::Address::Null(), rstAddr);
}
