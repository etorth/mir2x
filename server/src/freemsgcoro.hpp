#pragma once
#include <coroutine>
#include <exception>

class FreeMsgCoro
{
    public:
        struct FreeMsgCoroFinalAwaiter
        {
            bool await_ready  ()       const noexcept { return false; }
            void await_suspend(auto h) const noexcept { h.destroy() ; }
            void await_resume ()       const noexcept {}
        };

        struct promise_type
        {
            FreeMsgCoro get_return_object()
            {
                return {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always   initial_suspend() noexcept { return {}; }
            FreeMsgCoroFinalAwaiter final_suspend() noexcept { return {}; }

            void return_void() {}
            void unhandled_exception() { std::terminate(); }
        };

    private:
        std::coroutine_handle<promise_type> m_handle;

    public:
        FreeMsgCoro() = default;

    private:
        FreeMsgCoro(std::coroutine_handle<promise_type> h)
            : m_handle(h)
        {}

    public:
        void operator()() &&
        {
            if(m_handle){
                m_handle.resume();
            }
        }
};
