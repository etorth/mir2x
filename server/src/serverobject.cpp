#include <cinttypes>
#include "serdesmsg.hpp"
#include "actorpod.hpp"
#include "serverargparser.hpp"
#include "server.hpp"
#include "serverobject.hpp"
#include "actorpool.hpp"
#include "uidf.hpp"
#include "uidsf.hpp"

extern ActorPool *g_actorPool;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;

ServerObject::LuaThreadRunner::LuaThreadRunner(ServerObject *serverObject)
    : ServerLuaCoroutineRunner(serverObject->m_actorPod)
{
    bindCoop("_RSVD_NAME_queryQuestUID", [thisptr = this](this auto, LuaCoopResumer onDone, std::string questName) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto rmpk = co_await thisptr->m_actorPod->send(uidf::getServiceCoreUID(), {AM_QUERYQUESTUID, cerealf::serialize(SDQueryQuestUID
        {
            .name = std::move(questName),
        })});

        if(closed){
            co_return;
        }

        onDone.popOnClose();
        switch(rmpk.type()){
            case AM_UID:
                {
                    const auto amUID = rmpk.template conv<AMUID>();
                    if(amUID.uid){
                        onDone(amUID.uid);
                    }
                    else{
                        onDone();
                    }
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });

    bindCoop("_RSVD_NAME_queryQuestUIDList", [thisptr = this](this auto, LuaCoopResumer onDone) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto rmpk = co_await thisptr->m_actorPod->send(uidf::getServiceCoreUID(), AM_QUERYQUESTUIDLIST);
        if(closed){
            co_return;
        }

        onDone.popOnClose();

        switch(rmpk.type()){
            case AM_UIDLIST:
                {
                    const auto uidList = rmpk.template deserialize<SDUIDList>();
                    onDone(sol::as_table(uidList));
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });

    bindCoop("_RSVD_NAME_loadMap", [thisptr = this](this auto, LuaCoopResumer onDone, sol::object mapName) -> corof::awaitable<>
    {
        const auto mapID = [&mapName]() -> uint32_t
        {
            if(mapName.is<std::string>()) return DBCOM_MAPID(to_u8cstr(mapName.as<std::string>()));
            if(mapName.is<lua_Integer>()) return static_cast<uint32_t>(mapName.as<lua_Integer>());
            throw fflerror("invalid sol::object type");
        }();

        fflassert(mapID);

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        AMLoadMap amLM;
        std::memset(&amLM, 0, sizeof(AMLoadMap));

        amLM.mapUID = uidsf::getMapBaseUID(mapID);
        amLM.waitActivated = true;

        const auto mpk = co_await thisptr->m_actorPod->send(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM});
        if(closed){
            co_return;
        }

        onDone.popOnClose();
        switch(mpk.type()){
            case AM_LOADMAPOK:
                {
                    onDone(amLM.mapUID);
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });

    bindCoop("_RSVD_NAME_waitActivated", [thisptr = this](this auto, LuaCoopResumer onDone) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        co_await thisptr->getSO()->waitActivated();

        if(closed){
            co_return;
        }

        onDone.popOnClose();
        onDone();
    });

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "serverobject.lua"
    END_LUAINC()));
}

ServerObject::ServerObject(uint64_t uid)
    : m_UID([uid]() -> uint64_t
      {
          fflassert(uidf::validUID(uid));
          return uid;
      }())
{
    if(g_serverArgParser->sharedConfig().traceActorMessageCount){
        defer([this, lastCheckTick = to_u32(0)]() mutable -> bool
        {
            if(const auto currTick = g_server->getCurrTick(); lastCheckTick + 1000 < currTick){
                if(hasActorPod()){
                    m_actorPod->PrintMonitor();
                }
                lastCheckTick = currTick;
            }
            return false;
        });
    }
}

ServerObject::~ServerObject()
{
    delete m_actorPod;
}

uint64_t ServerObject::activate()
{
    fflassert(!m_actorPod);
    m_actorPod = new ActorPod(this);

    beforeActivate();
    m_actorPod->attach();

    // can NOT support afterActivate() here
    // after ActorPod::attach(), access to m_actorPod must be in actor thread

    return UID();
}

void ServerObject::deactivate()
{
    if(m_actorPod){
        m_actorPod->detach([this](){ delete this; });
    }
}

corof::awaitable<bool> ServerObject::queryDead(uint64_t uid)
{
    switch(const auto rmpk = co_await m_actorPod->send(uid, AM_QUERYDEAD); rmpk.type()){
        case AM_TRUE:
        case AM_BADACTORPOD:
            {
                co_return true;
            }
        case AM_FALSE:
            {
                co_return false;
            }
        default:
            {
                throw fflerror("unexpected message: %s", rmpk.str(UID()).c_str());
            }
    }
}

void ServerObject::forwardNetPackage(uint64_t uid, uint8_t type, const void *buf, size_t bufLen)
{
    fflassert(uid != UID());
    fflassert(uidf::isPlayer(uid));

    const auto *bufPtr = reinterpret_cast<const uint8_t *>(buf);
    std::vector<uint8_t> packData
    {
        bufPtr,
        bufPtr + bufLen,
    };

    m_actorPod->post(uid, {AM_SENDPACKAGE, cerealf::serialize(SDSendPackage
    {
        .type = type,
        .buf = std::move(packData),
    })});
}
