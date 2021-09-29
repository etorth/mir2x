/*
 * =====================================================================================
 *
 *       Filename: corof.hpp
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
#include <any>
#include <optional>
#include <coroutine>
#include <exception>
#include "fflerror.hpp"
#include "raiitimer.hpp"

namespace corof
{
    template<typename T> class eval_awaiter;
    class [[nodiscard]] eval_poller
    {
        private:
            class eval_poller_promise;

        public:
            using promise_type = eval_poller_promise;
            using  handle_type = std::coroutine_handle<promise_type>;

        private:
            class eval_poller_promise final
            {
                // hiden its definition and expose by aliasing to promise_type
                // this type is for compiler, user should never instantiate an eval_poller_promise object

                private:
                    friend class eval_poller;
                    template<typename T> friend class eval_awaiter;

                private:
                    std::any m_value;

                private:
                    handle_type m_inner_handle;
                    handle_type m_outer_handle;

                private:
                    std::exception_ptr m_exception = nullptr;

                public:
                    template<typename T> T &get_value()
                    {
                        return std::any_cast<T &>(m_value);
                    }

                    auto initial_suspend()
                    {
                        return std::suspend_never{};
                    }

                    auto final_suspend() noexcept
                    {
                        return std::suspend_always{};
                    }

                    template<typename T> void return_value(T t)
                    {
                        m_value = std::move(t);
                    }

                    eval_poller get_return_object()
                    {
                        return {handle_type::from_promise(*this)};
                    }

                    void unhandled_exception()
                    {
                        m_exception = std::current_exception();
                    }

                    void rethrow_if_unhandled_exception()
                    {
                        if(m_exception){
                            std::rethrow_exception(m_exception);
                        }
                    }
            };

        private:
            handle_type m_handle;

        public:
            eval_poller(handle_type handle = nullptr)
                : m_handle(handle)
            {}

        public:
            eval_poller(eval_poller && other) noexcept
                : m_handle(other.m_handle)
            {
                other.m_handle = nullptr;
            }

        public:
            eval_poller & operator = (eval_poller && other) noexcept
            {
                m_handle = other.m_handle;
                other.m_handle = nullptr;
                return *this;
            }

        public:
            ~eval_poller()
            {
                if(m_handle){
                    m_handle.destroy();
                }
            }

        public:
            bool valid() const
            {
                return m_handle.address();
            }

        public:
            bool poll()
            {
                fflassert(m_handle);
                handle_type curr_handle = find_handle(m_handle);
                if(curr_handle.done()){
                    if(!curr_handle.promise().m_outer_handle){
                        return true;
                    }

                    // jump out for one layer
                    // should I call destroy() for done handle?

                    curr_handle = curr_handle.promise().m_outer_handle;
                    curr_handle.promise().m_inner_handle = nullptr;

                    if(curr_handle.done()){
                        throw fflerror("linked done handle detected");
                    }
                }

                // resume only once and return immediately
                // after resume curr_handle can be in done state, next call to poll should unlink it

                curr_handle.resume();
                return m_handle.done();
            }

        private:
            static inline handle_type find_handle(handle_type start_handle)
            {
                fflassert(start_handle);
                auto curr_handle = start_handle;
                auto next_handle = start_handle.promise().m_inner_handle;

                while(curr_handle && next_handle){
                    curr_handle = next_handle;
                    next_handle = next_handle.promise().m_inner_handle;
                }
                return curr_handle;
            }

        public:
            template<typename T> decltype(auto) sync_eval();
            template<typename T> [[nodiscard]] eval_awaiter<T> to_awaiter();
    };

    template<typename T> class [[nodiscard]] eval_awaiter
    {
        private:
            friend class eval_poller;

        private:
            eval_poller::handle_type m_eval_handle;

        private:
            // eval_awaiter can only be created by eval_poller::to_awaiter()
            // do NOT permit user to create eval_awaiter from raw eval_poller::handle_type
            explicit eval_awaiter(eval_poller::handle_type handle)
                : m_eval_handle(handle)
            {
                fflassert(m_eval_handle);
            }

        public:
            eval_awaiter(eval_awaiter && other)
                : m_eval_handle(other.m_eval_handle)
            {
                other.m_eval_handle = nullptr;
            }

        public:
            eval_awaiter              (const eval_awaiter &) = delete;
            eval_awaiter & operator = (const eval_awaiter &) = delete;

        public:
            ~eval_awaiter()
            {
                if(m_eval_handle){
                    m_eval_handle.destroy();
                }
            }

        public:
            bool await_ready() noexcept
            {
                return false;
            }

        public:
            bool await_suspend(eval_poller::handle_type handle) noexcept
            {
                /*  */ handle.promise().m_inner_handle = m_eval_handle;
                m_eval_handle.promise().m_outer_handle =        handle;
                return true;
            }

        public:
            decltype(auto) await_resume()
            {
                return m_eval_handle.promise().get_value<T>();
            }
    };

    template<typename T> decltype(auto) eval_poller::sync_eval()
    {
        while(!poll()){
            continue;
        }
        return to_awaiter<T>().await_resume();
    }

    template<typename T> [[nodiscard]] eval_awaiter<T> eval_poller::to_awaiter()
    {
        fflassert(valid());
        auto hd = m_handle;
        m_handle = nullptr;
        return eval_awaiter<T>(hd);
    }
}

namespace corof
{
    template<typename T> class async_variable
    {
        private:
            std::optional<T> m_var;

        public:
            template<typename U = T> void assign(U && u)
            {
                if(m_var.has_value()){
                    throw fflerror("assign value to async_variable twice");
                }
                m_var = std::move(u);
            }

        public:
            async_variable() = default;

        public:
            template<typename U> async_variable(const async_variable<U> &) = delete;
            template<typename U = T> async_variable<T> & operator = (const async_variable<U> &) = delete;

        public:
            auto operator co_await() noexcept
            {
                const auto fnwait = +[](corof::async_variable<T> *p) -> corof::eval_poller
                {
                    while(!p->m_var.has_value()){
                        co_await std::suspend_always{};
                    }
                    co_return p->m_var.value();
                };
                return fnwait(this). template to_awaiter<T>();
            }
    };

    inline auto async_wait(uint64_t msec) noexcept
    {
        const auto fnwait = +[](uint64_t msec) -> corof::eval_poller
        {
            size_t count = 0;
            if(msec == 0){
                co_await std::suspend_always{};
                count++;
            }
            else{
                hres_timer timer;
                while(timer.diff_msec() < msec){
                    co_await std::suspend_always{};
                    count++;
                }
            }
            co_return count;
        };
        return fnwait(msec).to_awaiter<size_t>();
    }

    template<typename T> inline auto delay_value(uint64_t msec, T t) // how about variadic template argument
    {
        const auto fnwait = +[](uint64_t msec, T t) -> corof::eval_poller
        {
            co_await corof::async_wait(msec);
            co_return t;
        };
        return fnwait(msec, std::move(t)). template to_awaiter<T>();
    }
}
