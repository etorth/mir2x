#include "serdesmsg.hpp"
#include "serverluaobject.hpp"

corof::awaitable<> ServerLuaObject::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    auto sdRC = mpk.deserialize<SDRemoteCall>();
    m_luaRunner->spawn(m_threadKey++, mpk.fromAddr(), std::move(sdRC.code), std::move(sdRC.args));
    return {};
}
