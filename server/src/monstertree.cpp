/*
 * =====================================================================================
 *
 *       Filename: bugbatmaggot.cpp
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
#include "monstertree.hpp"

corof::long_jmper MonsterTree::updateCoroFunc()
{
    while(HP() > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
    co_return true;
}
