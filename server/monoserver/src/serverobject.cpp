/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 05/23/2016 18:22:01
 *  Last Modified: 03/21/2017 22:33:19
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
ServerObject::ServerObject(uint8_t nCategory)
    : ServerObject(nCategory, g_MonoServer->GetUID(), g_MonoServer->GetTimeTick())
{}
