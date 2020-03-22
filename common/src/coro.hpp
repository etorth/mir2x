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
#include <optional>
#ifdef _MSC_VER
#include <windows.h>
#else
#include "aco.h"
#endif
#include "fflerror.hpp"

inline auto &coro_get_main_coref()
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
    if(coro_get_main_coref()){
        throw fflerror("Call coro_init_thread() twice");
    }

#ifdef _MSC_VER
    {
        coro_get_main_coref() = ConvertThreadToFiber(nullptr);
    }
#else
    {
        aco_thread_init(nullptr);
        coro_get_main_coref() = aco_create(nullptr, nullptr, 0, nullptr, nullptr);
    }
#endif

    if(!coro_get_main_coref()){
        throw fflerror("Failed to allocate mainCO");
    }
}

inline void coro_yield()
{
#ifdef _MSC_VER
    SwitchToFiber(coro_get_main_coref());
#else
    aco_yield();
#endif
}

class coro_stack
{
#ifndef _MSC_VER
    private:
        template<typename T> friend class coro;

    private:
        aco_share_stack_t *m_stack = nullptr;

    public:
        coro_stack()
        {
            m_stack = aco_share_stack_new(0);
            if(!m_stack){
                throw fflerror("Failed to allocate aco shared stack");
            }
        }

        ~coro_stack()
        {
            aco_share_stack_destroy(m_stack);
        }
#endif
};

inline coro_stack *coro_get_tls_stack()
{
    thread_local coro_stack t_stack;
    return &t_stack;
}

template<typename T> class coro final
{
    private:
        std::shared_ptr<coro_stack> m_stack;

    private:
        const T m_func;

#ifdef _MSC_VER
    private:
        bool m_done = false;
#endif

    private:
#ifdef _MSC_VER
        LPVOID m_handle = nullptr;
#else
        aco_t *m_handle = nullptr;
#endif

    public:
        explicit coro(T t)
            : coro(nullptr, t)
        {}

        coro(std::shared_ptr<coro_stack> sstk, T t)
            : coro(sstk.get(), t)
        {
#ifndef _MSC_VER
            if(sstk){
                m_stack = sstk;
            }
#endif
        }

        coro(coro_stack *sstk, T t)
            : m_func(t)
        {
            if(!coro_get_main_coref()){
                coro_init_thread();
            }

#ifdef _MSC_VER
            {
                auto fnRoutine = [](void *coptr) -> void
                {
                    reinterpret_cast<coro<T> *>(coptr)->m_func();
                    reinterpret_cast<coro<T> *>(coptr)->m_done = true;
                    SwitchToFiber(coro_get_main_coref());
                };
                m_handle = CreateFiber(0, fnRoutine, (void *)(this));
            }
#else
            {
                auto fnRoutine = []() -> void
                {
                    reinterpret_cast<coro<T> *>(aco_get_arg())->m_func();
                    aco_exit();
                };

                if(!sstk){
                    m_stack = std::make_shared<coro_stack>();
                    sstk = m_stack.get();
                }
                m_handle = aco_create(coro_get_main_coref(), sstk->m_stack, 0, fnRoutine, (void *)(this));
            }
#endif
        }

    public:
        ~coro()
        {
#ifdef _MSC_VER
            DeleteFiber(m_handle);
#else
            aco_destroy(m_handle);
#endif
            m_handle = nullptr;
        }

        void coro_resume()
        {
            if(is_done()){
                throw fflerror("Resume finished coroutine");
            }

#ifdef _MSC_VER
            {
                if(GetCurrentFiber() != coro_get_main_coref()){
                    throw fflerror("Call coro_resume() not from mainCO");
                }
                SwitchToFiber(m_handle);
            }
#else
            {
                if(m_handle->main_co != coro_get_main_coref()){
                    throw fflerror("Call coro_resume() not from its mainCO");
                }
                aco_resume(m_handle);
            }
#endif
        }

        bool in_main() const
        {
            if(is_done()){
                throw fflerror("Resume finished coroutine");
            }

#ifdef _MSC_VER
            return GetCurrentFiber() == coro_get_main_coref();
#else
            return m_handle->main_co == coro_get_main_coref();
#endif
        }

        bool is_done() const
        {
#ifdef _MSC_VER
            return m_done;
#else
            return m_handle->is_end;
#endif
        }
};

class coro_yielder final
{
    public:
        ~coro_yielder()
        {
            coro_yield();
        }
};

template<typename T> class coro_variable
{
    private:
        std::optional<T> m_var;

    public:
        template<typename U = T> void assign(U &&u)
        {
            if(m_var.has_value()){
                throw fflerror("Assign value to coro_variable twice");
            }
            m_var = std::move(u);
        }

    public:
        coro_variable() {}

    public:
        template<typename U> coro_variable(const coro_variable<U> &) = delete;
        template<typename U = T> coro_variable<T> &operator = (const coro_variable<U> &) = delete;

    public:
        const auto &wait() const
        {
            while(!m_var.has_value()){
                coro_yield();
            }
            return m_var.value();
        }

        T &wait()
        {
            return const_cast<T &>(static_cast<const coro_variable<T> *>(this)->wait());
        }
};
