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
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "cppcoro/coroutine.hpp"

namespace corof
{
    class long_jmper_promise;
    class [[nodiscard]] long_jmper 
    {
        public:
            using promise_type = long_jmper_promise;
            using  handle_type = cppcoro::coroutine_handle<promise_type>;

        public:
            handle_type m_handle;

        public:
            long_jmper(handle_type handle = nullptr)
                : m_handle(handle) 
            {}

            long_jmper(long_jmper&& other) noexcept
                : m_handle(other.m_handle)
            {
                other.m_handle = nullptr;
            }

        public:
            long_jmper & operator = (long_jmper && other) noexcept
            {
                m_handle = other.m_handle;
                other.m_handle = nullptr;
                return *this;
            }

        public:
            ~long_jmper()
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
            inline bool poll_one();

        private:
            static inline handle_type find_handle(handle_type);

        public:
            template<typename T> class [[nodiscard]] eval_op
            {
                private:
                    handle_type m_eval_handle;

                public:
                    eval_op(long_jmper &&jmper) noexcept
                        : m_eval_handle(jmper.m_handle)
                    {
                        jmper.m_handle = nullptr;
                    }

                    ~eval_op()
                    {
                        if(m_eval_handle){
                            m_eval_handle.destroy();
                        }
                    }

                public:
                    eval_op & operator = (eval_op && other) noexcept
                    {
                        m_eval_handle = other.m_eval_handle;
                        other.m_eval_handle = nullptr;
                        return *this;
                    }

                public:
                    bool await_ready() noexcept
                    {
                        return false;
                    }

                public:
                    bool await_suspend(handle_type handle) noexcept;
                    decltype(auto) await_resume();
            };

        public:
            template<typename T> [[nodiscard]] eval_op<T> eval()
            {
                if(valid()){
                    return eval_op<T>(std::move(*this));
                }
                throw fflerror("long_jmper has no eval-context associated");
            }

            template<typename T> auto sync_eval()
            {
                while(!poll_one()){
                    continue;
                }
                return eval<T>().await_resume();
            }
    };

    class long_jmper_promise 
    {
        private:
            friend class long_jmper;

        private:
            std::any m_value;
            long_jmper::handle_type m_inner_handle;
            long_jmper::handle_type m_outer_handle;

        public:
            template<typename T> T &get_value()
            {
                return std::any_cast<T &>(m_value);
            }

            auto initial_suspend()
            {
                return std::suspend_never{};
            }

            auto final_suspend()
            {
                return std::suspend_always{};
            }

            template<typename T> auto return_value(T t)
            {
                m_value = std::move(t);
                return std::suspend_always{};
            }

            long_jmper get_return_object()
            {
                return {std::coroutine_handle<long_jmper_promise>::from_promise(*this)};
            }

            void unhandled_exception()
            {
                std::terminate();
            }

            void rethrow_if_unhandled_exception()
            {
            }
    };

    inline long_jmper::handle_type long_jmper::find_handle(long_jmper::handle_type start_handle)
    {
        if(!start_handle){
            throw fflerror("invalid argument: find_handle(nullptr)");
        }

        auto curr_handle = start_handle;
        auto next_handle = start_handle.promise().m_inner_handle;

        while(curr_handle && next_handle){
            curr_handle = next_handle;
            next_handle = next_handle.promise().m_inner_handle;
        }
        return curr_handle;
    }

    inline bool long_jmper::poll_one()
    {
        if(!m_handle){
            throw fflerror("long_jmper has no eval-context associated");
        }

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
        // after resume curr_handle can be in done state, next call to poll_one should unlink it

        curr_handle.resume();
        return m_handle.done();
    }

    // inline bool long_jmper::poll_one()
    // {
    //     if(!m_handle){
    //         throw fflerror("long_jmper has no eval-context associated");
    //     }
    //
    //     bool resumed = false;
    //     auto curr_handle = m_handle;
    //
    //     while(curr_handle){
    //         if(!curr_handle.promise().m_inner_handle){
    //             while(!curr_handle.done()){
    //                 curr_handle.resume();
    //                 resumed = true;
    //
    //                 if(!curr_handle.done()){
    //                     return false;
    //                 }
    //
    //                 if(!curr_handle.promise().m_outer_handle){
    //                     return true;
    //                 }
    //
    //                 curr_handle = curr_handle.promise().m_outer_handle;
    //                 curr_handle.promise().m_inner_handle = nullptr;
    //             }
    //
    //             if(!curr_handle.promise().m_outer_handle){
    //                 return true;
    //             }
    //
    //             curr_handle = curr_handle.promise().m_outer_handle;
    //             curr_handle.promise().m_inner_handle = nullptr;
    //
    //             if(resumed){
    //                 return m_handle.done();
    //             }
    //         }
    //         curr_handle = curr_handle.promise().m_inner_handle;
    //     }
    //     return m_handle.done();
    // }

    template<typename T> inline bool long_jmper::eval_op<T>::await_suspend(handle_type handle) noexcept
    {
        handle       .promise().m_inner_handle = m_eval_handle;
        m_eval_handle.promise().m_outer_handle = handle;
        return true;
    }

    template<typename T> inline decltype(auto) long_jmper::eval_op<T>::await_resume()
    {
        return m_eval_handle.promise().get_value<T>();
    }
}

namespace corof
{
    template<typename T> class async_variable
    {
        private:
            std::optional<T> m_var;

        public:
            template<typename U = T> void assign(U &&u)
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
                const auto fnwait = +[](corof::async_variable<T> *p) -> corof::long_jmper
                {
                    while(!p->m_var.has_value()){
                        co_await std::suspend_always{};
                    }
                    co_return p->m_var.value();
                };
                return fnwait(this). template eval<T>();
            }
    };

    inline auto async_wait(uint64_t msec) noexcept
    {
        const auto fnwait = +[](uint64_t msec) -> corof::long_jmper
        {
            size_t count = 0;
            if(msec == 0){
                co_await cppcoro::suspend_always{};
                count++;
            }
            else{
                hres_timer timer;
                while(timer.diff_msec() < msec){
                    co_await cppcoro::suspend_always{};
                    count++;
                }
            }
            co_return count;
        };
        return fnwait(msec).eval<size_t>();
    }

    template<typename T> inline auto delay_value(uint64_t msec, T t) // how about variadic template argument
    {
        const auto fnwait = +[](uint64_t msec, T t) -> corof::long_jmper
        {
            co_await corof::async_wait(msec);
            co_return t;
        };
        return fnwait(msec, std::move(t)). template eval<T>();
    }
}
