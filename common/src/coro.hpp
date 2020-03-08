/*
 * =====================================================================================
 *
 *       Filename: coro.hpp
 *        Created: 03/07/2020 12:36:32
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
#ifdef _MSC_VER
#include <windows.h>
#else
#include "aco.h"
#endif
#include "fflerror.hpp"

inline auto &getMainCORef()
{
#ifdef _MSC_VER
    thread_local LPVOID t_mainCO = nullptr;
#else
    thread_local aco_t *t_mainCO = nullptr;
#endif
    return t_mainCO;
}

inline void coro_init_thread()
{
    if(getMainCORef()){
        throw fflerror("Call coro_init_thread() twice");
    }

#ifdef _MSC_VER
    {
        getMainCORef() = ConvertThreadToFiber(nullptr);
    }
#else
    {
        aco_thread_init(nullptr);
        getMainCORef() = aco_create(nullptr, nullptr, 0, nullptr, nullptr);
    }
#endif

    if(!getMainCORef()){
        throw fflerror("Failed to allocate mainCO");
    }
}

template<typename T> class coro
{
    private:
        const T m_func;

    private:
#ifdef _MSC_VER
        LPVOID m_handle = nullptr;
#else
        aco_t *m_handle = nullptr;
        aco_share_stack_t *m_stack = nullptr;
#endif

    public:
        coro(T t): m_func(t)
        {
#ifdef _MSC_VER
            {
                auto fnRoutine = [](void *coptr) -> void
                {
                    reinterpret_cast<coro<T> *>(coptr)->m_func(reinterpret_cast<coro<T> *>(coptr));
                };
                m_handle = CreateFiber(0, fnRoutine, (void *)(this));
            }
#else
            {
                auto fnRoutine = []() -> void
                {
                    reinterpret_cast<coro<T> *>(aco_get_arg())->m_func(reinterpret_cast<coro<T> *>(aco_get_arg()));
                };
                m_stack = aco_share_stack_new(0);
                m_handle = aco_create(getMainCORef(), m_stack, 0, fnRoutine, (void *)(this));
            }
#endif
        }

    public:
        void coro_yield()
        {
#ifdef _MSC_VER
            {
                SwitchToFiber(getMainCORef());
            }
#else
            {
                aco_yield();
            }
#endif
        }

        void coro_resume()
        {
#ifdef _MSC_VER
            {
                if(GetCurrentFiber() != getMainCORef()){
                    throw fflerror("Call coro_resume() not from mainCO");
                }
                SwitchToFiber(m_handle);
            }
#else
            {
                if(m_handle->main_co != getMainCORef()){
                    throw fflerror("Call coro_resume() not from its mainCO");
                }
                aco_resume(m_handle);
            }
#endif
        }
};
