/*
 * =====================================================================================
 *
 *       Filename: serverguardco.cpp
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

#include "serverguard.hpp"
corof::long_jmper::eval_op<bool> ServerGuard::coro_jumpBack()
{
    const auto fnwait = +[](ServerGuard *p) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->jumpBack([&done](){ done.assign(true); }, [&done](){ done.assign(false); });
        const auto result = co_await done;
        co_return result;
    };
    return fnwait(this).eval<bool>();
}
