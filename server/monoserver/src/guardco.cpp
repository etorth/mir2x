/*
 * =====================================================================================
 *
 *       Filename: guardco.cpp
 *        Created: 04/26/2021 02:32:45
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

#include "guard.hpp"

corof::long_jmper::eval_op<bool> Guard::coro_jumpBack()
{
    const auto fnwait = +[](Guard *p) -> corof::long_jmper
    {
        if(p->X() == p->m_standX && p->Y() == p->m_standY){
            if(p->Direction() != p->m_standDirection){
                p->m_direction = p->m_standDirection;
                p->dispatchAction(p->makeActionStand());
            }
            co_return true;
        }

        corof::async_variable<bool> done;
        p->requestJump(p->m_standX, p->m_standY, p->m_standDirection, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
        const auto result = co_await done;
        co_return result;
    };
    return fnwait(this).eval<bool>();
}
