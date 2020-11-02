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
#include "fflerror.hpp"
#include "cppcoro/generator.hpp"

namespace corof
{
    template <typename T> class [[nodiscard]] long_jmper 
    {
        public:
            class long_jmper_promise;

        public:
            using promise_type = long_jmper_promise;
            using  handle_type = std::coroutine_handle<promise_type>;

        public:
            mutable handle_type m_handle;

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
            ~long_jmper()
            {
                if(m_handle){
                    m_handle.destroy();
                }
            }

        public:
            operator bool () const noexcept
            {
                return bool(m_handle.address());
            }

            long_jmper & operator = (long_jmper && other) noexcept
            {
                m_handle = other.m_handle;
                other.m_handle = nullptr;
                return *this;
            }

        public:
            bool await_ready() noexcept
            {
                return false;
            }

            bool await_suspend(std::coroutine_handle<>) noexcept
            {
                return true;
            }

            bool await_suspend(std::coroutine_handle<promise_type> handle) noexcept
            {
                handle  .promise().m_inner_handler = m_handle;
                m_handle.promise().m_outer_handler =   handle;
                return true;
            }

            auto await_resume()
            {
                return m_handle.promise().m_value.value();
            }

        public:
            bool poll_one()
            {
                auto curr_handle = m_handle;
                while(curr_handle){
                    if(!curr_handle.promise().m_inner_handler){
                        while(!curr_handle.done()){
                            curr_handle.resume();
                            if(!curr_handle.done()){
                                return false;
                            }

                            if(curr_handle.promise().m_outer_handler){
                                curr_handle = curr_handle.promise().m_outer_handler;
                                curr_handle.promise().m_inner_handler = nullptr;
                            }
                            else{
                                return true;
                            }
                        }
                        break;
                    }
                    curr_handle = curr_handle.promise().m_inner_handler;
                }
                return curr_handle.done();
            }

        public:
            class long_jmper_promise 
            {
                private:
                    friend struct long_jmper;

                private:
                    std::optional<T> m_value {};
                    std::coroutine_handle<promise_type> m_inner_handler {};
                    std::coroutine_handle<promise_type> m_outer_handler {};

                public:
                    auto value()
                    {
                        return m_value;
                    }

                    auto initial_suspend() noexcept
                    {
                        return cppcoro::suspend_never{};
                    }

                    auto final_suspend() noexcept
                    {
                        return cppcoro::suspend_always{};
                    }

                    auto return_value(T t)
                    {
                        m_value = t;
                        return std::suspend_always{};
                    }

                    long_jmper<T> get_return_object() noexcept
                    {
                        return {handle_type::from_promise(*this)};
                    }

                    void unhandled_exception() noexcept
                    {
                        std::terminate();
                    }

                    void rethrow_if_unhandled_exception()
                    {
                    }
            };
    };

    template<typename T> class async_variable
    {
        private:
            size_t m_count = 0;

        private:
            std::optional<T> m_var;
            cppcoro::generator<int> m_generator;
            cppcoro::generator<int>::iterator m_update;

        public:
            template<typename U = T> void assign(U &&u)
            {
                if(m_var.has_value()){
                    throw fflerror("Assign value to async_variable twice");
                }
                m_var = std::move(u);
            }

        public:
            async_variable()
                : m_generator([this]() -> cppcoro::generator<int>
                  {
                      while(!m_var.has_value()){
                          co_yield m_count++;
                      }
                  }())
            {}

        public:
            template<typename U> async_variable(const async_variable<U> &) = delete;
            template<typename U = T> async_variable<T> &operator = (const async_variable<U> &) = delete;

        public:
            auto &wait()
            {
                while(!m_var.has_value()){
                    if(m_update == m_generator.end()){
                        m_update = m_generator.begin();
                    }
                    else{
                        m_update++;
                    }
                }
                return m_var.value();
            }
    };
}
