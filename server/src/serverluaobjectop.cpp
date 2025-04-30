#include "serdesmsg.hpp"
#include "serverluaobject.hpp"

corof::entrance ServerLuaObject::on_AM_METRONOME(const ActorMsgPack &)
{
    return {};
}

corof::entrance ServerLuaObject::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    auto sdRC = mpk.deserialize<SDRemoteCall>();
    m_luaRunner->spawn(m_threadKey++, mpk.fromAddr(), std::move(sdRC.code), std::move(sdRC.args));
    return {};
}
