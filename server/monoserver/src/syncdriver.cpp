/*
 * =====================================================================================
 *
 *       Filename: syrcdriver.cpp
 *        Created: 04/27/2016 00:33:56
 *  Last Modified: 04/27/2016 00:39:32
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

#include "syrcdriver.hpp"

bool SyrcDriver::Send(const MessagePack &rstMPK,
        const Theron::Address &rstAddress, MessagePack *pMsgPack)
{
    MessagePack stMPK;
    // 1. clean the cather
    while(true){ if(!m_Catcher.Pop(stMPK, rstAddress)){ break; } }

    // 2. send message
    extern Theron::Framework *g_Framework;
    g_Framework->Send(rstMPK, m_Receiver.GetAddress(), rstAddress);

    // 3. wait for response
    m_Receiver.Wait(1);

    // 5. handle response from service core
    if(m_Catcher.Pop(stMPK, m_ServiceCoreAddress) && stMPK.Type == MPK_OK){
        // accepted!
        g_MonoServer->AddLog(LOGTYPE_INFO, "connection from (%s:%d) allowed",
                m_Socket.remote_endpoint().address().to_string().c_str(),
                m_Socket.remote_endpoint().port());

        // 1. keep it in the hub
        m_SessionMap[pNewSession->ID()] = pNewSession;

        // 2. echo to the client
        pNewSession->Send(SM_PING);

        // 3. ready login information
    }
