#pragma once
#include <optional>
#include <coroutine>
#include "actormsgpack.hpp"

struct SendMsgCoro;
struct SendMsgCoroPromise;
struct SendMsgCoroPromiseFinalAwaiter
{
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<SendMsgCoroPromise>) noexcept;
    void await_resume() noexcept {}
};

struct SendMsgCoroPromise
{
    std::optional<ActorMsgPack> reply;
    std::coroutine_handle<> continuation;

    SendMsgCoro get_return_object() noexcept;
    void return_void(){}

    std::suspend_always          initial_suspend() const noexcept { return {}; }
    SendMsgCoroPromiseFinalAwaiter final_suspend() const noexcept { return {}; }

    void unhandled_exception()
    {
        std::terminate();
    }
};

struct SendMsgCoroAwaiter
{
    std::coroutine_handle<SendMsgCoroPromise> handle;

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept;
    ActorMsgPack await_resume() noexcept;
};

struct SendMsgCoro
{
    using promise_type = SendMsgCoroPromise;
    std::coroutine_handle<promise_type> handle;

    SendMsgCoro(std::coroutine_handle<promise_type> h)
        : handle(h)
    {}

    SendMsgCoroAwaiter operator co_await() && noexcept
    {
        return {handle};
    }
};
