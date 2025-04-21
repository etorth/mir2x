#include "actormsgpack.hpp"
#include "sendmsgcoro.hpp"

void SendMsgCoroPromiseFinalAwaiter::await_suspend(std::coroutine_handle<SendMsgCoroPromise> handle) noexcept
{
    handle.promise().continuation.resume();
    handle.destroy();
}

SendMsgCoro SendMsgCoroPromise::get_return_object() noexcept
{
    return {std::coroutine_handle<SendMsgCoroPromise>::from_promise(*this)};
}

void SendMsgCoroAwaiter::await_suspend(std::coroutine_handle<> h) noexcept
{
    handle.promise().continuation = h;
    handle.resume();
}

ActorMsgPack SendMsgCoroAwaiter::await_resume() noexcept
{
    return handle.promise().reply.value_or(ActorMsgPack(AM_ERROR));
}
