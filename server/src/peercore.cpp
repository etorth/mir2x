#include <string>
#include <cstring>
#include "uidf.hpp"
#include "uidsf.hpp"
#include "quest.hpp"
#include "player.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "rotatecoord.hpp"
#include "actorpod.hpp"
#include "mapbindb.hpp"
#include "serdesmsg.hpp"
#include "server.hpp"
#include "servicecore.hpp"
#include "npchar.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "serverguard.hpp"
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

extern MapBinDB *g_mapBinDB;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

PeerCore::PeerCore()
    : ServerObject(uidsf::getPeerCoreUID())
{}

std::pair<bool, bool> PeerCore::loadMap(uint64_t mapUID)
{
    if(!g_mapBinDB->retrieve(uidf::getMapID(mapUID))){
        return {false, false};
    }

    if(m_mapList.contains(mapUID)){
        return {true, false};
    }

    auto mapPtr = new ServerMap(mapUID);
    mapPtr->activate();

    m_mapList.insert(mapUID);
    return {true, true};
}

void PeerCore::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_ADDCO:
            {
                on_AM_ADDCO(mpk);
                break;
            }
        case AM_PEERLOADMAP:
            {
                on_AM_PEERLOADMAP(mpk);
                break;
            }
        default:
            {
                g_server->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(mpk.type()));
                break;
            }
    }
}

bool PeerCore::adjustMapGLoc(uint32_t mapID, int &x, int &y, bool strictLoc)
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

std::optional<std::pair<int, int>> PeerCore::getMapGLoc(uint32_t mapID, int x, int y, std::optional<int> checkOpt) const
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

CharObject *PeerCore::addCO(SDInitCharObject sdICO)
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

ServerGuard *PeerCore::addGuard(SDInitGuard sdIG)
{
    if(g_serverArgParser->disableGuardSpawn){
        return nullptr;
    }

    if(!adjustMapGLoc(uidf::getMapID(sdIG.mapUID), sdIG.x, sdIG.y, sdIG.strictLoc)){
        return nullptr;
    }

    auto guardPtr = new ServerGuard(sdIG);
    guardPtr->activate();
    return guardPtr;
}

Player *PeerCore::addPlayer(SDInitPlayer sdIP)
{
    if(!adjustMapGLoc(uidf::getMapID(sdIP.mapUID), sdIP.x, sdIP.y, sdIP.strictLoc)){
        return nullptr;
    }

    auto playerPtr = new Player(sdIP);
    playerPtr->activate();

    AMBindChannel amBC;
    std::memset(&amBC, 0, sizeof(amBC));
    amBC.channID = sdIP.channID;

    m_actorPod->forward(playerPtr->UID(), {AM_BINDCHANNEL, amBC});
    return playerPtr;
}

NPChar *PeerCore::addNPChar(SDInitNPChar sdINPC)
{
    if(g_serverArgParser->disableNPCSpawn){
        return nullptr;
    }

    if(!adjustMapGLoc(uidf::getMapID(sdINPC.mapUID), sdINPC.x, sdINPC.y, sdINPC.strictLoc)){
        return nullptr;
    }

    auto npcPtr = new NPChar(sdINPC);
    npcPtr->activate();
    return npcPtr;
}

Monster *PeerCore::addMonster(SDInitMonster sdIM)
{
    if(uidf::getUIDType(sdIM.masterUID) == UID_PLY){
        if(g_serverArgParser->disablePetSpawn){
            return nullptr;
        }
    }
    else{
        if(g_serverArgParser->disableMonsterSpawn){
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
