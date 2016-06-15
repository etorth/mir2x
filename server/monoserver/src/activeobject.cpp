/*
 * =====================================================================================
 *
 *       Filename: activeobject.cpp
 *        Created: 06/14/2016 23:11:17
 *  Last Modified: 06/15/2016 00:13:33
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

void ActiveObject::ResetStateTime(uint8_t nState)
{
    extern MonoServer *g_MonoServer;
    m_StateTimeV[nState] = g_MonoServer->GetTimeTick();
}

double ActiveObject::StateTime(uint8_t nState)
{
    extern MonoServer *g_MonoServer;
    return g_MonoServer->GetTimeTick() - m_StateTimeV[nState];
}
