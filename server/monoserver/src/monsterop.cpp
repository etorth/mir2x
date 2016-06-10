/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 06/09/2016 18:30:19
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

#include "monster.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "monoserver.hpp"

void Monster::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    // set the RM address, it's the source of information
    m_RMAddress = rstFromAddr;

    QuerySCAddress();
    QueryMapAddress();
}

void Monster::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    Update();
}
