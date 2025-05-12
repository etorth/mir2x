#pragma once
#include "serverobject.hpp"

class ServerLuaObject: public ServerObject
{
    protected:
        class LuaThreadRunner: public ServerObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(ServerLuaObject *);

            public:
                ServerLuaObject *getServerLuaObject() const
                {
                    return static_cast<ServerLuaObject *>(m_actorPod->getSO());
                }
        };

    private:
        std::unique_ptr<ServerLuaObject::LuaThreadRunner> m_luaRunner;

    private:
        uint64_t m_threadKey = 1;

    public:
        ServerLuaObject(uint32_t);

    protected:
        corof::awaitable<> onActivate() override;

    protected:
        corof::awaitable<> onActorMsg(const ActorMsgPack &) override;

    protected:
        corof::awaitable<> on_AM_REMOTECALL(const ActorMsgPack &);
};
