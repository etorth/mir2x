#include "serverobject.hpp"
#include "serverluaobject.hpp"

ServerLuaObject::LuaThreadRunner::LuaThreadRunner(ServerLuaObject *serverLuaObjectPtr)
    : ServerObject::LuaThreadRunner(serverLuaObjectPtr)
{
    fflassert(dynamic_cast<ServerLuaObject *>(getSO()));
    fflassert(dynamic_cast<ServerLuaObject *>(getSO()) == serverLuaObjectPtr);
}

ServerLuaObject::ServerLuaObject(uint32_t luaObjIndex)
    : ServerObject(uidf::getServerLuaObjectUID(luaObjIndex))
{}

corof::awaitable<> ServerLuaObject::onActivate()
{
    co_await ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerLuaObject::LuaThreadRunner>(this);
}

corof::awaitable<> ServerLuaObject::onActorMsg(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_REMOTECALL:
            {
                return on_AM_REMOTECALL(mpk);
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}
