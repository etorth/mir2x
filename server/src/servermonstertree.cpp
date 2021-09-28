/*
 * =====================================================================================
 *
 *       Filename: serverbugbatmaggot.cpp
 *        Created: 04/07/2016 03:48:41 AM
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

#include "sysconst.hpp"
#include "servermonstertree.hpp"

corof::eval_poller ServerMonsterTree::updateCoroFunc()
{
    while(m_sdHealth.HP > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
    co_return true;
}
