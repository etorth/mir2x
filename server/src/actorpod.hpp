#pragma once
#include <array>
#include <string>
#include <cfloat>
#include <memory>
#include <variant>
#include <optional>
#include <coroutine>
#include <functional>
#include <unordered_map>

#include "corof.hpp"
#include "actormsgbuf.hpp"
#include "actormsgpack.hpp"
#include "actormonitor.hpp"

class ServerObject;
class ActorPod final
{
    private:
        friend class ActorPool;

    private:
        // innHandler() puts the response in the slot and resumes the handle
        // the slot is inside the awaiter, which stays in the suspended coroutine frame until await_resume()
        struct RespondSlot
        {
            std::coroutine_handle<> handle = nullptr;
            std::optional<ActorMsgPack> mpkOpt = std::nullopt;
        };

        template<bool AllowOverwrite> struct RespondAwaiter
        {
            ActorPod * const actor;
            uint64_t   const seqID;

            RespondSlot slot {};

            constexpr bool await_ready() const noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle)
            {
                slot.handle = handle;
                if constexpr(AllowOverwrite){
                    actor->m_respondCBList.insert_or_assign(seqID, std::addressof(slot));
                }
                else{
                    if(!actor->m_respondCBList.try_emplace(seqID, std::addressof(slot)).second){
                        throw fflpanic("{}: seqID {} already has a continuation", to_cstr(uidf::getUIDString(actor->UID())), seqID);
                    }
                }
            }

            ActorMsgPack await_resume()
            {
                return std::move(slot.mpkOpt.value());
            }
        };

    private:
        const uint64_t m_UID;

    private:
        ServerObject * const m_SO;

    private:
        std::array<std::function<corof::awaitable<>(const ActorMsgPack &)>, AM_END> m_msgOpList;

    private:
        // used by rollSeqID()
        // to create unique proper ID for an message expcecting response
        uint64_t m_nextSeqID = 1;

    private:
        std::unordered_map<uint64_t, std::variant<RespondSlot *, std::function<void(const ActorMsgPack &)>>> m_respondCBList;

    private:
        ActorPodMonitor m_podMonitor;

    private:
        uint32_t m_channID = 0;

    public:
        explicit ActorPod(ServerObject *);

    public:
        ~ActorPod();

    private:
        uint64_t rollSeqID();

    private:
        void innHandler(const ActorMsgPack &);

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    public:
        void attach();
        void detach(std::function<void()>) const;

    public:
        static bool checkUIDValid(uint64_t);

    public:
        void PrintMonitor() const;

    public:
        ActorPodMonitor dumpPodMonitor() const
        {
            return m_podMonitor;
        }

    public:
        ServerObject *getSO() const
        {
            return m_SO;
        }

    public:
        void registerOp(int type, std::function<corof::awaitable<>(const ActorMsgPack &)> op)
        {
            if(op){
                m_msgOpList.at(type) = std::move(op);
            }
        }

    public:
        void postNet(          uint8_t, const void *, size_t, uint64_t);
        void postNet(uint32_t, uint8_t, const void *, size_t, uint64_t);

    public:
        bool   bindNet(uint32_t);
        void unbindNet();
        void  closeNet();

    private:
        std::optional<uint64_t> doPost(const std::pair<uint64_t, uint64_t> &, ActorMsgBuf, bool);

    public:
        bool post(const std::pair<uint64_t, uint64_t> &addr, ActorMsgBuf mbuf)
        {
            return doPost(addr, std::move(mbuf), false).has_value();
        }

        bool post(uint64_t addr, ActorMsgBuf mbuf)
        {
            return post({addr, 0}, std::move(mbuf));
        }

    public:
        corof::awaitable<ActorMsgPack> send(std::pair<uint64_t, uint64_t> addr, ActorMsgBuf mbuf)
        {
            if(const auto seqIDOpt = doPost(addr, std::move(mbuf), true); seqIDOpt.has_value()){
                co_return co_await RespondAwaiter<false>
                {
                    .actor = this,
                    .seqID = seqIDOpt.value(),
                };
            }
            else{
                co_return ActorMsgPack{AM_BADACTORPOD};
            }
        }

    public:
        corof::awaitable<ActorMsgPack> send(uint64_t addr, ActorMsgBuf mbuf)
        {
            return send({addr, 0}, std::move(mbuf));
        }

    private:
        std::pair<uint64_t, uint64_t> doCreateWaitToken(uint64_t, std::function<void(const ActorMsgPack &)>);

    public:
        std::pair<uint64_t, uint64_t> createWaitToken(uint64_t tick, std::function<void(const ActorMsgPack &)> op)
        {
            fflassert(op);
            return doCreateWaitToken(tick, std::move(op));
        }

    public:
        void cancelWaitToken(const std::pair<uint64_t, uint64_t> &);

    public:
        corof::awaitable<ActorMsgPack> wait(uint64_t tick, std::pair<uint64_t, uint64_t> * tokenPtr = nullptr)
        {
            // don't introduce waitToken()
            // we need to immediately await a token before scheduled AM_TIMEOUT comes

            const auto token = doCreateWaitToken(tick, nullptr);

            if(tokenPtr){
                *tokenPtr = token;
            }

            co_return co_await RespondAwaiter<true>
            {
                .actor = this,
                .seqID = token.second,
            };
        }
};
