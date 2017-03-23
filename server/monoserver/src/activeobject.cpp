/*
 * =====================================================================================
 *
 *       Filename: activeobject.cpp
 *        Created: 06/14/2016 23:11:17
 *  Last Modified: 03/22/2017 16:18:29
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

#include "monoserver.hpp"
#include "activeobject.hpp"

uint8_t ActiveObject::State(uint8_t nState)
{
    return m_StateV[nState];
}

uint32_t ActiveObject::StateTime(uint8_t nState)
{
    extern MonoServer *g_MonoServer;
    return g_MonoServer->GetTimeTick() - m_StateTimeV[nState];
}

void ActiveObject::ResetState(uint8_t nStateLoc, uint8_t nStateValue)
{
    extern MonoServer *g_MonoServer;
    m_StateV[nStateLoc] = nStateValue;
    m_StateTimeV[nStateLoc] = g_MonoServer->GetTimeTick();
}
