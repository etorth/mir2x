/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <memory>
#include <cstring>

#include "log.hpp"
#include "pathf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"
#include "clienttaodog.hpp"
#include "clienttaoskeleton.hpp"
#include "clientnpc.hpp"
#include "uidf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"

// we get all needed initialization info for init the process run
void ProcessRun::net_LOGINOK(const uint8_t *bufPtr, size_t nLen)
{
    if(bufPtr && nLen && (nLen == sizeof(SMLoginOK))){
        SMLoginOK stSMLOK;
        std::memcpy(&stSMLOK, bufPtr, nLen);

        uint64_t nUID     = stSMLOK.UID;
        uint32_t nDBID    = stSMLOK.DBID;
        bool     bGender  = stSMLOK.Male;
        uint32_t nMapID   = stSMLOK.MapID;
        uint32_t nDressID = 0;

        int nX = stSMLOK.X;
        int nY = stSMLOK.Y;
        int nDirection = stSMLOK.Direction;

        loadMap(nMapID);

        m_myHeroUID = nUID;
        m_coList[nUID] = std::make_unique<MyHero>(nUID, nDBID, bGender, nDressID, this, ActionStand(nX, nY, nDirection));

        centerMyHero();
        getMyHero()->pullGold();
    }
}

void ProcessRun::net_ACTION(const uint8_t *bufPtr, size_t)
{
    SMAction smA;
    std::memcpy(&smA, bufPtr, sizeof(smA));

    ActionNode stAction
    {
        smA.Action,
        smA.Speed,
        smA.Direction,
        smA.X,
        smA.Y,
        smA.AimX,
        smA.AimY,
        smA.AimUID,
        smA.ActionParam,
    };

    if(smA.MapID != MapID()){
        if(smA.UID != getMyHero()->UID()){
            return;
        }

        // detected map switch for myhero
        // need to do map switch and parse current action

        // call getMyHero before loadMap()
        // getMyHero() checks if current creature is on current map

        auto nUID       = getMyHero()->UID();
        auto nDBID      = getMyHero()->DBID();
        auto bGender    = getMyHero()->Gender();
        auto nDress     = getMyHero()->Dress();
        auto nDirection = getMyHero()->currMotion()->direction;

        auto nX = smA.X;
        auto nY = smA.Y;

        m_actionBlocker.clear();
        loadMap(smA.MapID);

        m_coList.clear();
        m_coList[m_myHeroUID] = std::make_unique<MyHero>(nUID, nDBID, bGender, nDress, this, ActionStand(nX, nY, nDirection));

        centerMyHero();
        getMyHero()->parseAction(stAction);
        return;
    }

    // map doesn't change
    // action from an existing charobject for current processrun

    if(auto creaturePtr = findUID(smA.UID)){
        // shouldn't accept ACTION_SPAWN
        // we shouldn't have spawn action after co created
        if(smA.Action == ACTION_SPAWN){
            throw fflerror("existing CO get spawn action: name = %s", uidf::getUIDString(smA.UID).c_str());
        }

        creaturePtr->parseAction(stAction);
        switch(stAction.Action){
            case ACTION_SPACEMOVE2:
                {
                    if(smA.UID == m_myHeroUID){
                        centerMyHero();
                    }
                    return;
                }
            default:
                {
                    return;
                }
        }
        return;
    }

    // map doesn't change
    // action from an non-existing charobject, may need query

    switch(uidf::getUIDType(smA.UID)){
        case UID_PLY:
            {
                // do query only for player
                // can't create new player based on action information
                queryCORecord(smA.UID);
                return;
            }
        case UID_MON:
            {
                switch(smA.Action){
                    case ACTION_SPAWN:
                        {
                            onActionSpawn(smA.UID, stAction);
                            return;
                        }
                    default:
                        {
                            switch(uidf::getMonsterID(smA.UID)){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID] = std::make_unique<ClientTaoSkeleton>(smA.UID, this, stAction);
                                        }
                                        return;
                                    }
                                case DBCOM_MONSTERID(u8"神兽"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID] = std::make_unique<ClientTaoDog>(smA.UID, this, stAction);
                                        }
                                        return;
                                    }
                                default:
                                    {
                                        m_coList[smA.UID] = std::make_unique<ClientMonster>(smA.UID, this, stAction);
                                        return;
                                    }
                            }
                            return;
                        }
                }
                return;
            }
        case UID_NPC:
            {
                m_coList[smA.UID] = std::make_unique<ClientNPC>(smA.UID, this, stAction);
                return;
            }
        default:
            {
                return;
            }
    }
}

void ProcessRun::net_CORECORD(const uint8_t *bufPtr, size_t)
{
    const auto smCOR = ServerMsg::conv<SMCORecord>(bufPtr);
    if(smCOR.Action.MapID != MapID()){
        return;
    }

    const ActionNode actionNode
    {
        smCOR.Action.Action,
        smCOR.Action.Speed,
        smCOR.Action.Direction,
        smCOR.Action.X,
        smCOR.Action.Y,
        smCOR.Action.AimX,
        smCOR.Action.AimY,
        smCOR.Action.AimUID,
        smCOR.Action.ActionParam,
    };

    if(auto p = m_coList.find(smCOR.Action.UID); p != m_coList.end()){
        p->second->parseAction(actionNode);
        return;
    }

    switch(uidf::getUIDType(smCOR.Action.UID)){
        case UID_MON:
            {
                switch(uidf::getMonsterID(smCOR.Action.UID)){
                    case DBCOM_MONSTERID(u8"变异骷髅"):
                        {
                            m_coList[smCOR.Action.UID].reset(new ClientTaoSkeleton(smCOR.Action.UID, this, actionNode));
                            break;
                        }
                    case DBCOM_MONSTERID(u8"神兽"):
                        {
                            m_coList[smCOR.Action.UID].reset(new ClientTaoDog(smCOR.Action.UID, this, actionNode));
                            break;
                        }
                    default:
                        {
                            m_coList[smCOR.Action.UID] = std::make_unique<ClientMonster>(smCOR.Action.UID, this, actionNode);
                            break;
                        }
                }
                break;
            }
        case UID_PLY:
            {
                m_coList[smCOR.Action.UID] = std::make_unique<Hero>(smCOR.Action.UID, smCOR.Player.DBID, true, 0, this, actionNode);
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::net_UPDATEHP(const uint8_t *bufPtr, size_t)
{
    SMUpdateHP stSMUHP;
    std::memcpy(&stSMUHP, bufPtr, sizeof(stSMUHP));

    if(stSMUHP.MapID == MapID()){
        if(auto p = findUID(stSMUHP.UID)){
            p->updateHealth(stSMUHP.HP, stSMUHP.HPMax);
        }
    }
}

void ProcessRun::net_NOTIFYDEAD(const uint8_t *bufPtr, size_t)
{
    SMNotifyDead stSMND;
    std::memcpy(&stSMND, bufPtr, sizeof(stSMND));

    if(auto p = findUID(stSMND.UID)){
        p->parseAction(ActionDie(p->x(), p->y(), p->currMotion()->direction, true));
    }
}

void ProcessRun::net_DEADFADEOUT(const uint8_t *bufPtr, size_t)
{
    SMDeadFadeOut stSMDFO;
    std::memcpy(&stSMDFO, bufPtr, sizeof(stSMDFO));

    if(stSMDFO.MapID == MapID()){
        if(auto p = findUID(stSMDFO.UID)){
            p->deadFadeOut();
        }
    }
}

void ProcessRun::net_EXP(const uint8_t *bufPtr, size_t)
{
    const auto smExp = ServerMsg::conv<SMExp>(bufPtr);
    if(smExp.Exp){
        addCBLog(CBLOG_SYS, u8"你获得了经验值%d", (int)(smExp.Exp));
    }
}

void ProcessRun::net_MISS(const uint8_t *bufPtr, size_t)
{
    SMMiss stSMM;
    std::memcpy(&stSMM, bufPtr, sizeof(stSMM));

    if(auto p = findUID(stSMM.UID)){
        int nX = p->x() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2 - 20;
        int nY = p->y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;
        addAscendStr(ASCENDSTR_MISS, 0, nX, nY);
    }
}

void ProcessRun::net_PING(const uint8_t *bufPtr, size_t)
{
    if(m_lastPingDone){
        throw fflerror("received echo while no ping has been sent to server");
    }

    const auto currTick = SDL_GetTicks();
    const auto smP = ServerMsg::conv<SMPing>(bufPtr);

    if(currTick < smP.Tick){
        throw fflerror("invalid ping tick: %llu -> %llu", to_llu(smP.Tick), to_llu(currTick));
    }

    m_lastPingDone = true;
    addCBLog(CBLOG_SYS, u8"延迟%llums", to_llu(currTick - smP.Tick));
}

void ProcessRun::net_SHOWDROPITEM(const uint8_t *bufPtr, size_t)
{
    const auto smSDI = ServerMsg::conv<SMShowDropItem>(bufPtr);
    clearGroundItem(smSDI.X, smSDI.Y);

    for(size_t i = 0; i < std::extent<decltype(smSDI.IDList)>::value; ++i){
        const CommonItem item(smSDI.IDList[i].ID, smSDI.IDList[i].DBID);
        if(!item){
            break;
        }
        addGroundItem(item, smSDI.X, smSDI.Y);
    }
}

void ProcessRun::net_CASTMAGIC(const uint8_t *bufPtr, size_t)
{
    const auto smCM = ServerMsg::conv<SMCastMagic>(bufPtr);
    const auto &mr = DBCOM_MAGICRECORD(smCM.Magic);

    if(!mr){
        return;
    }

    addCBLog(CBLOG_SYS, u8"使用魔法: %s", mr.name);
    switch(smCM.Magic){
        case DBCOM_MAGICID(u8"魔法盾"):
            {
                if(auto coPtr = findUID(smCM.UID)){
                    auto magicPtr = new AttachMagic(u8"魔法盾", u8"开始");
                    magicPtr->addOnDone([smCM, this]()
                    {
                        if(auto coPtr = findUID(smCM.UID)){
                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"运行")));
                        }
                    });
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(magicPtr));
                }
                return;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                if(auto coPtr = findUID(smCM.AimUID)){
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"雷电术", u8"运行")));
                }
                return;
            }
        case DBCOM_MAGICID(u8"灵魂火符"):
            {
                return;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::net_OFFLINE(const uint8_t *bufPtr, size_t)
{
    SMOffline stSMO;
    std::memcpy(&stSMO, bufPtr, sizeof(stSMO));

    if(stSMO.MapID == MapID()){
        if(auto creaturePtr = findUID(stSMO.UID)){
            creaturePtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"启动")));
        }
    }
}

void ProcessRun::net_PICKUPOK(const uint8_t *bufPtr, size_t)
{
    const auto smPUOK = ServerMsg::conv<SMPickUpOK>(bufPtr);
    getMyHero()->getInvPack().Add(smPUOK.ID);

    removeGroundItem(CommonItem(smPUOK.ID, 0), smPUOK.X, smPUOK.Y);
    addCBLog(CBLOG_SYS, u8"捡起%s于坐标(%d, %d)", DBCOM_ITEMRECORD(smPUOK.ID).name, (int)(smPUOK.X), (int)(smPUOK.Y));
}

void ProcessRun::net_GOLD(const uint8_t *bufPtr, size_t)
{
    SMGold stSMG;
    std::memcpy(&stSMG, bufPtr, sizeof(stSMG));
    getMyHero()->setGold(stSMG.Gold);
}

void ProcessRun::net_NPCXMLLAYOUT(const uint8_t *buf, size_t)
{
    const auto smNPCXMLL = ServerMsg::conv<SMNPCXMLLayout>(buf);
    auto chatBoardPtr = dynamic_cast<NPCChatBoard *>(getGUIManager()->getWidget("NPCChatBoard"));
    chatBoardPtr->loadXML(smNPCXMLL.NPCUID, smNPCXMLL.xmlLayout);
    chatBoardPtr->show(true);
}
