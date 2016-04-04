/*
 * =====================================================================================
 *
 *       Filename: task.hpp
 *        Created: 04/03/2016 19:40:00
 *  Last Modified: 04/03/2016 21:32:12
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

#pragma once
#include <chrono>
#include <functional>

#define LOCAL_SYSTEM_TIME_NOW \
    (std::chrono::system_clock::now())

#define LOCAL_SYSTEM_TIME_NEXT(nMS) \
    (std::chrono::system_clock::now() + std::chrono::milliseconds(nMS))

#define LOCAL_SYSTEM_TIME_ZERO \
    (std::chrono::system_clock::time_point(std::chrono::milliseconds(0)))

class Task
{
    protected:
        std::chrono::system_clock::time_point m_Expiration;
        std::function<void()>                 m_Func;

    public:
        Task(uint32_t nDuraMS, const std::function<void()>& fnOp)
            : m_Func(fnOp)
        {
            // TBD & TODO:
            // they don't put this in the initialization list
            // maybe in case copy of fnOp takes time?
            m_Expiration = LOCAL_SYSTEM_TIME_NEXT(nDuraMS);
        }

        explicit Task(const std::function<void (void)>& nfOp)
            : m_Expiration(SYSTEM_TIME_ZERO)
            , m_Func(fnOp)
        {}

        virtual ~Task() = default;

        void operator()()
        {
            if(m_Func){ m_Func(); }
        }

        void Expire(uint32_t nMS)
        {
            m_Expiration = (nMS ? LOCAL_SYSTEM_TIME_NEXT(nMS) : LOCAL_SYSTEM_TIME_ZERO);
        }

        bool Expired()
        {
            if(m_Expiration == SYSTEM_TIME_ZERO){
                return false;
            }
            return m_Expiration < LOCAL_SYSTEM_TIME_NOW;
        }
};

#undef LOCAL_SYSTEM_TIME_NOW
#undef LOCAL_SYSTEM_TIME_ZERO
#undef LOCAL_SYSTEM_TIME_NEXT
