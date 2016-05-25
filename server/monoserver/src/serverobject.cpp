/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 05/23/2016 18:22:01
 *  Last Modified: 05/24/2016 21:50:59
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
#include "serverobject.hpp"

ServerObject::ServerObject(uint8_t nCategory)
    : m_Category(nCategory)
{
    extern MonoServer *g_MonoServer;
    m_UID     = g_MonoServer->GetUID();
    m_AddTime = g_MonoServer->GetTimeTick();
}
