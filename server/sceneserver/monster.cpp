/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 8/29/2015 10:13:55 PM
 *  Last Modified: 09/04/2015 1:54:58 AM
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
#include "sceneserver.hpp"

Monster::Monster(int nSID, int nUID, int nGenTime)
    : Actor(nSID, nUID, nGenTime)
{
}

Monster::~Monster()
{
}

void Monster::Update()
{
    // printf("monster %05d updated here\n", m_UID);
    extern SceneServer *g_SceneServer;
    int nTime = g_SceneServer->GetTimeMS();
    if(nTime > m_LastSyrcTime + 500){
        m_LastSyrcTime = nTime;
        BroadcastBaseInfo();
    }
}
