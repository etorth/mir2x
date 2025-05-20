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

    bindCoop("_RSVD_NAME_loadMap", [thisptr = this](this auto, LuaCoopResumer onDone, std::string mapName) -> corof::awaitable<>
    {
        fflassert(str_haschar(mapName));

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        AMLoadMap amLM;
        std::memset(&amLM, 0, sizeof(AMLoadMap));
        amLM.mapUID = uidsf::getMapBaseUID(DBCOM_MAPID(to_u8cstr(mapName)));

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
}

ServerObject::ServerObject(uint64_t uid)
    : m_UID(uid)
{
    if(g_serverArgParser->sharedConfig().traceActorMessageCount){
        m_stateTrigger.install([this, lastCheckTick = to_u32(0)]() mutable -> bool
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

// TODO & TBD
// when an actor is activated by more than one time, we can
// 1. delete previously allocated actor and create a new one
// 2. just return current address
//
// Now I use method-2, since the address could hanve been assigned to many other place
// for communication, delete it may cause problems
//
// And if we really want to change the address of current object, maybe we need to
// delete current object totally and create a new one instead
uint64_t ServerObject::activate()
{
    fflassert(!m_actorPod);
    m_actorPod = new ActorPod(m_UID, this);

    // seperate attach call
    // this triggers the startup callback, i.e. the onActivate()
    // if automatically call attach() in ActorPod::ctor() then m_actorPod is invalid yet

    m_actorPod->attach();
    return UID();
}

void ServerObject::deactivate()
{
    if(m_actorPod){
        m_actorPod->detach([this](){ delete this; });
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
