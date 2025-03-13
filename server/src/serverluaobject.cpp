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

void ServerLuaObject::onActivate()
{
    ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerLuaObject::LuaThreadRunner>(this);
}

void ServerLuaObject::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(mpk);
                break;
            }
        case AM_REMOTECALL:
            {
                on_AM_REMOTECALL(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}
