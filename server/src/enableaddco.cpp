#include "serverargparser.hpp"
#include "rotatecoord.hpp"
#include "uidf.hpp"
#include "sgf.hpp"
#include "uidsf.hpp"
#include "actorpod.hpp"
#include "actormsg.hpp"
#include "mapbindb.hpp"
#include "enableaddco.hpp"
#include "server.hpp"
#include "serverobject.hpp"
#include "servicecore.hpp"
#include "serverguard.hpp"
#include "player.hpp"
#include "npchar.hpp"
#include "monster.hpp"
#include "servertaodog.hpp"
#include "servertaoskeleton.hpp"
#include "servertaoskeletonext.hpp"
#include "servercannibalplant.hpp"
#include "serverbugbatmaggot.hpp"
#include "servermonstertree.hpp"
#include "serverdualaxeskeleton.hpp"
#include "servereviltentacle.hpp"
#include "serversandcactus.hpp"
#include "serversandghost.hpp"
#include "serverrebornzombie.hpp"
#include "serveranthealer.hpp"
#include "serverwoomataurus.hpp"
#include "serverevilcentipede.hpp"
#include "serverzumamonster.hpp"
#include "serverzumataurus.hpp"
#include "serverbombspider.hpp"
#include "serverrootspider.hpp"
#include "serverredmoonevil.hpp"
#include "servershipwrecklord.hpp"
#include "serverminotaurguardian.hpp"

extern Server *g_server;
extern MapBinDB *g_mapBinDB;
extern ServerArgParser *g_serverArgParser;

EnableAddCO::EnableAddCO(ActorPod *argPod)
    : m_actorPod(argPod)
{
    fflassert(m_actorPod);
    m_actorPod->registerOp(AM_ADDCO, [thisptr = this](this auto, const ActorMsgPack &mpk) -> corof::awaitable<>
    {
        // always create CO if request received
        // it's sender's responsibility to figure out to forward the request to which peer

        const auto sdICO = mpk.deserialize<SDInitCharObject>();
        const auto fnAddCO = [sdICO, fromAddr = mpk.fromAddr(), thisptr]()
        {
            std::string err;
            try{
                if(auto coPtr = thisptr->addCO(sdICO)){
                    AMUID amUID;
                    std::memset(&amUID, 0, sizeof(amUID));

                    amUID.uid = coPtr->UID();
                    thisptr->m_actorPod->post(fromAddr, {AM_UID, amUID});
                    return;
                }
            }
            catch(const std::exception &e){
                err = str_haschar(e.what()) ? e.what() : "unknown error";
            }
            catch(...){
                err = "unknown error";
            }

            if(!err.empty()){
                g_server->addLog(LOGTYPE_WARNING, "Failed in EnableAddCO::ADDCO: %s", to_cstr(err));
            }
            thisptr->m_actorPod->post(fromAddr, AM_ERROR);
        };

        const auto mapUID = std::visit(VarDispatcher
        {
            [](const SDInitGuard   &sdIG  ) { return sdIG  .mapUID; },
            [](const SDInitPlayer  &sdIP  ) { return sdIP  .mapUID; },
            [](const SDInitNPChar  &sdINPC) { return sdINPC.mapUID; },
            [](const SDInitMonster &sdIM  ) { return sdIM  .mapUID; },
        },

        sdICO);

        fflassert(uidf::isMap(mapUID));
        if(thisptr->m_actorPod->UID() == mapUID){
            fnAddCO();
        }
        else if(thisptr->m_actorPod->UID() == uidf::getServiceCoreUID()){
            if(const auto loadRes = co_await dynamic_cast<ServiceCore *>(thisptr->m_actorPod->getSO())->requestLoadMap(mapUID); loadRes.first){
                fnAddCO();
            }
            else{
                thisptr->m_actorPod->post(mpk.fromAddr(), AM_ERROR);
            }
        }
        else{
            AMLoadMap amLM;
            std::memset(&amLM, 0, sizeof(amLM));
            amLM.mapUID = mapUID;

            switch(const auto rmpk = co_await thisptr->m_actorPod->send(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}); rmpk.type()){
                case AM_LOADMAPOK:
                    {
                        fnAddCO();
                        break;
                    }
                default:
                    {
                        thisptr->m_actorPod->post(mpk.fromAddr(), AM_ERROR);
                        break;
                    }
            }
        }
    });
}

bool EnableAddCO::adjustMapGLoc(uint32_t mapID, int &x, int &y, bool strictLoc)
{
    const auto checkOpt = strictLoc ? std::nullopt : std::make_optional(100);
    const auto loc = getMapGLoc(mapID, x, y, checkOpt);

    if(!loc.has_value()){
        return false;
    }

    x = loc.value().first;
    y = loc.value().second;
    return true;
}

std::optional<std::pair<int, int>> EnableAddCO::getMapGLoc(uint32_t mapID, int x, int y, std::optional<int> checkOpt) const
{
    const auto mapBin = g_mapBinDB->retrieve(mapID);
    if(!mapBin){
        return std::nullopt;
    }

    if(mapBin->groundValid(x, y)){
        return std::make_pair(x, y);
    }

    if(!checkOpt.has_value()){
        return std::nullopt;
    }

    if(!mapBin->validC(x, y)){
        x = mathf::rand() % mapBin->w();
        y = mathf::rand() % mapBin->h();
    }

    RotateCoord rc
    {
        x,
        y,
        0,
        0,
        to_d(mapBin->w()),
        to_d(mapBin->h()),
    };

    for(int i = 0; (checkOpt.value() <= 0) || (i < checkOpt.value()); ++i){
        const int currX = rc.x();
        const int currY = rc.y();

        if(mapBin->groundValid(currX, currY)){
            return std::make_pair(currX, currY);
        }

        if(!rc.forward()){
            break;
        }
    }
    return std::nullopt;
}

CharObject *EnableAddCO::addCO(SDInitCharObject sdICO)
{
    return std::visit(VarDispatcher
    {
        [this](const SDInitGuard   &sdIG  ) { return static_cast<CharObject *>(addGuard  (sdIG  )); },
        [this](const SDInitPlayer  &sdIP  ) { return static_cast<CharObject *>(addPlayer (sdIP  )); },
        [this](const SDInitNPChar  &sdINPC) { return static_cast<CharObject *>(addNPChar (sdINPC)); },
        [this](const SDInitMonster &sdIM  ) { return static_cast<CharObject *>(addMonster(sdIM  )); },
    },

    std::move(sdICO));
}

ServerGuard *EnableAddCO::addGuard(SDInitGuard sdIG)
{
    if(g_serverArgParser->sharedConfig().disableGuardSpawn){
        return nullptr;
    }

    if(!adjustMapGLoc(uidf::getMapID(sdIG.mapUID), sdIG.x, sdIG.y, sdIG.strictLoc)){
        return nullptr;
    }

    auto guardPtr = new ServerGuard(sdIG);
    guardPtr->activate();
    return guardPtr;
}

Player *EnableAddCO::addPlayer(SDInitPlayer sdIP)
{
    if(!adjustMapGLoc(uidf::getMapID(sdIP.mapUID), sdIP.x, sdIP.y, sdIP.strictLoc)){
        return nullptr;
    }

    if(uidsf::peerIndex()){
        throw fflerror("can not spawn player on peer server %zu", uidsf::peerIndex());
    }

    auto playerPtr = new Player(sdIP);
    playerPtr->activate();

    AMBindChannel amBC;
    std::memset(&amBC, 0, sizeof(amBC));
    amBC.channID = sdIP.channID;

    m_actorPod->post(playerPtr->UID(), {AM_BINDCHANNEL, amBC});
    return playerPtr;
}

NPChar *EnableAddCO::addNPChar(SDInitNPChar sdINPC)
{
    if(g_serverArgParser->sharedConfig().disableNPCSpawn){
        return nullptr;
    }

    if(!adjustMapGLoc(uidf::getMapID(sdINPC.mapUID), sdINPC.x, sdINPC.y, sdINPC.strictLoc)){
        return nullptr;
    }

    if(uidsf::peerIndex()){
        throw fflerror("can not spawn NPC on peer server %zu", uidsf::peerIndex());
    }

    auto npcPtr = new NPChar(sdINPC);
    npcPtr->activate();
    return npcPtr;
}

Monster *EnableAddCO::addMonster(SDInitMonster sdIM)
{
    if(uidf::getUIDType(sdIM.masterUID) == UID_PLY){
        if(g_serverArgParser->sharedConfig().disablePetSpawn){
            return nullptr;
        }
    }
    else{
        if(g_serverArgParser->sharedConfig().disableMonsterSpawn){
            return nullptr;
        }
    }

    if(!adjustMapGLoc(uidf::getMapID(sdIM.mapUID), sdIM.x, sdIM.y, sdIM.strictLoc)){
        return nullptr;
    }

    auto monsterPtr = [sdIM = std::move(sdIM)]() -> Monster *
    {
        switch(sdIM.monsterID){
            case DBCOM_MONSTERID(u8"变异骷髅"):
                {
                    return new ServerTaoSkeleton
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"超强骷髅"):
                {
                    return new ServerTaoSkeletonExt
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"神兽"):
                {
                    return new ServerTaoDog
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP, // TODO face its master
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"食人花"):
                {
                    return new ServerCannibalPlant
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"角蝇"):
                {
                    return new ServerBugbatMaggot
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"蝙蝠"):
                {
                    return new Monster
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_LEFT, // direction for initial gfx when born
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"栗子树"):
            case DBCOM_MONSTERID(u8"圣诞树"):
            case DBCOM_MONSTERID(u8"圣诞树1"):
                {
                    return new ServerMonsterTree
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"沙漠树魔"):
                {
                    return new ServerSandCactus
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"掷斧骷髅"):
                {
                    return new ServerDualAxeSkeleton
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"触角神魔"):
            case DBCOM_MONSTERID(u8"爆毒神魔"):
                {
                    return new ServerEvilTentacle
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                    };
                }
            case DBCOM_MONSTERID(u8"沙鬼"):
                {
                    return new ServerSandGhost
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                    };
                }
            case DBCOM_MONSTERID(u8"僵尸_1"):
            case DBCOM_MONSTERID(u8"僵尸_2"):
            case DBCOM_MONSTERID(u8"腐僵"):
                {
                    return new ServerRebornZombie
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                    };
                }
            case DBCOM_MONSTERID(u8"蚂蚁道士"):
                {
                    return new ServerAntHealer
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"沃玛教主"):
                {
                    return new ServerWoomaTaurus
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"触龙神"):
                {
                    return new ServerEvilCentipede
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"祖玛雕像"):
            case DBCOM_MONSTERID(u8"祖玛卫士"):
                {
                    return new ServerZumaMonster
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"祖玛教主"):
                {
                    return new ServerZumaTaurus
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"爆裂蜘蛛"):
                {
                    return new ServerBombSpider
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                    };
                }
            case DBCOM_MONSTERID(u8"幻影蜘蛛"):
                {
                    return new ServerRootSpider
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"赤月恶魔"):
                {
                    return new ServerRedMoonEvil
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                    };
                }
            case DBCOM_MONSTERID(u8"霸王教主"):
                {
                    return new ServerShipwreckLord
                    {
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            case DBCOM_MONSTERID(u8"潘夜左护卫"):
            case DBCOM_MONSTERID(u8"潘夜右护卫"):
                {
                    return new ServerMinotaurGuardian
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
            default:
                {
                    return new Monster
                    {
                        sdIM.monsterID,
                        sdIM.mapUID,
                        sdIM.x,
                        sdIM.y,
                        DIR_UP,
                        sdIM.masterUID,
                    };
                }
        }
    }();

    monsterPtr->activate();
    return monsterPtr;
}
