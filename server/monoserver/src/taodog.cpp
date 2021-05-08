/*
 * =====================================================================================
 *
 *       Filename: taodog.cpp
 *        Created: 04/10/2016 02:32:45
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

#include "taodog.hpp"
corof::long_jmper TaoDog::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            co_await coro_trackAttackUID(targetUID);
        }

        else if(m_actorPod->checkUIDValid(masterUID())){
            co_await coro_followMaster();
        }
        else{
            break;
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
