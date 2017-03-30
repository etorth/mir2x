/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 05/23/2016 18:22:01
 *  Last Modified: 03/30/2017 00:22:07
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

extern MonoServer *g_MonoServer;
ServerObject::ServerObject(bool bActive)
    : ServerObject(bActive, g_MonoServer->GetUID())
{}
