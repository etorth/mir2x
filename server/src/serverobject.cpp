#include <cinttypes>
#include "serdesmsg.hpp"
#include "actorpod.hpp"
#include "serverargparser.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"
#include "actorpool.hpp"
#include "uidf.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ServerObject::LuaThreadRunner::LuaThreadRunner(ServerObject *serverObject)
    : ServerLuaCoroutineRunner(serverObject->m_actorPod)
{
    bindFunctionCoop("_RSVD_NAME_queryQuestUID", [this](LuaCoopResumer onDone, std::string questName)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_QUERYQUESTUID, cerealf::serialize(SDQueryQuestUID
        {
            .name = std::move(questName),
        })},

        [closed, onDone](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(rmpk.type()){
                case AM_UID:
                    {
                        const auto amUID = rmpk.conv<AMUID>();
                        if(amUID.UID){
                            onDone(amUID.UID);
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
    });

    bindFunctionCoop("_RSVD_NAME_queryQuestUIDList", [this](LuaCoopResumer onDone)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        m_actorPod->forward(uidf::getServiceCoreUID(), AM_QUERYQUESTUIDLIST, [closed, onDone](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(rmpk.type()){
                case AM_UIDLIST:
                    {
                        const auto uidList = rmpk.deserialize<SDUIDList>();
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
    });

    bindFunctionCoop("_RSVD_NAME_loadMap", [this](LuaCoopResumer onDone, std::string mapName)
    {
        fflassert(str_haschar(mapName));

        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed, this]()
        {
            *closed = true;
        });

        AMLoadMap amLM;
        std::memset(&amLM, 0, sizeof(AMLoadMap));

        amLM.mapID = DBCOM_MAPID(to_u8cstr(mapName));
        amLM.activateMap = true;

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [closed, mapID = amLM.mapID, onDone, this](const ActorMsgPack &mpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(mpk.type()){
                case AM_LOADMAPOK:
                    {
                        const auto amLMOK = mpk.conv<AMLoadMapOK>();
                        if(amLMOK.uid){
                            onDone(amLMOK.uid);
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
    });
}

ServerObject::ServerObject(uint64_t uid)
    : m_UID(uid)
{
    m_stateTrigger.install([this]() -> bool
    {
        m_delayCmdQ.exec();
        return false;
    });

    if(g_serverArgParser->traceActorMessageCount){
        m_stateTrigger.install([this, lastCheckTick = to_u32(0)]() mutable -> bool
        {
            if(const auto currTick = g_monoServer->getCurrTick(); lastCheckTick + 1000 < currTick){
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
uint64_t ServerObject::activate(double metronomeFreq, uint64_t expireTime)
{
    fflassert(!m_actorPod);
    m_actorPod = new ActorPod
    {
        m_UID,
        this,

        [this]()
        {
            m_stateTrigger.run();
        },

        [this](const ActorMsgPack &mpk)
        {
            operateAM(mpk);
        },

        metronomeFreq,
        expireTime,
    };

    // seperate attach call
    // this triggers the startup callback, i.e. the onActivate()
    // if automatically call attach() in ActorPod::ctor() then m_actorPod is invalid yet

    m_actorPod->attach([this]()
    {
        onActivate();
    });
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
    fflassert(uidf::getUIDType(uid) == UID_PLY);

    AMSendPackage amSP;
    std::memset(&amSP, 0, sizeof(amSP));

    buildActorDataPackage(&(amSP.package), type, buf, bufLen);
    m_actorPod->forward(uid, {AM_SENDPACKAGE, amSP}); // TODO when actor is offline, we should register callback to delete buffer allocated here
}
