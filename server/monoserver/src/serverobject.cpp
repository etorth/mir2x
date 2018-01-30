/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 05/23/2016 18:22:01
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

#include "log.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"

extern MonoServer *g_MonoServer;
ServerObject::ServerObject()
    : m_UID(g_MonoServer->GetUID())
{
    // link to the mono object pool
    // never allocate ServerObject on stack
    // MonoServer::EraseUID() will use *this* to call destructor
    g_MonoServer->LinkUID(m_UID, this);
}
