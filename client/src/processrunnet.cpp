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
#include "clientguard.hpp"
#include "clientpiranhaplant.hpp"
#include "uidf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "cerealf.hpp"
#include "serdesmsg.hpp"

void ProcessRun::net_LOGINOK(const uint8_t *buf, size_t bufSize)
{
    auto sdLOK = cerealf::deserialize<SDLoginOK>(buf, bufSize);
    loadMap(sdLOK.mapID);

    m_myHeroUID = sdLOK.uid;
    m_coList[m_myHeroUID] = std::make_unique<MyHero>(m_myHeroUID, this, ActionStand
    {
        .x = sdLOK.x,
        .y = sdLOK.y,
        .direction = DIR_DOWN,
    });

    getMyHero()->setName(to_cstr(sdLOK.name), sdLOK.nameColor);
    getMyHero()->setWLDesp(std::move(sdLOK.desp));

    centerMyHero();
    getMyHero()->pullGold();
}

void ProcessRun::net_SELLITEMLIST(const uint8_t *buf, size_t bufSize)
{
    auto sdSIL = cerealf::deserialize<SDSellItemList>(buf, bufSize);
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));
    purchaseBoardPtr->setSellItemList(std::move(sdSIL));
}

void ProcessRun::net_ACTION(const uint8_t *bufPtr, size_t)
{
    SMAction smA;
    std::memcpy(&smA, bufPtr, sizeof(smA));

    if(smA.mapID != mapID()){
        if(smA.UID != getMyHero()->UID()){
            return;
        }

        // detected map switch for myhero
        // need to do map switch and parse current action

        // call getMyHero before loadMap()
        // getMyHero() checks if current creature is on current map

        m_actionBlocker.clear();
        getMyHero()->flushMotionPending();
        loadMap(smA.mapID);

        // directly assign a stand motion
        // this need to skip all location validation

        auto myHeroPtr = std::move(m_coList[m_myHeroUID]);
        const auto oldDir = myHeroPtr->currMotion()->direction;
        dynamic_cast<Hero *>(myHeroPtr.get())->jumpLoc(smA.action.x, smA.action.y, oldDir);

        m_coList.clear();
        m_coList[m_myHeroUID] = std::move(myHeroPtr);

        centerMyHero();
        getMyHero()->parseAction(smA.action);
        return;
    }

    // map doesn't change
    // action from an existing charobject for current processrun

    if(auto coPtr = findUID(smA.UID)){
        // shouldn't accept ACTION_SPAWN
        // we shouldn't have spawn action after co created
        if(smA.action.type == ACTION_SPAWN){
            throw fflerror("existing CO get spawn action: name = %s", uidf::getUIDString(smA.UID).c_str());
        }

        coPtr->parseAction(smA.action);
        switch(smA.action.type){
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
                switch(smA.action.type){
                    case ACTION_SPAWN:
                        {
                            onActionSpawn(smA.UID, smA.action);
                            return;
                        }
                    default:
                        {
                            switch(const auto monID = uidf::getMonsterID(smA.UID)){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID] = std::make_unique<ClientTaoSkeleton>(smA.UID, this, smA.action);
                                        }
                                        return;
                                    }
                                case DBCOM_MONSTERID(u8"神兽"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID] = std::make_unique<ClientTaoDog>(smA.UID, this, smA.action);
                                        }
                                        return;
                                    }
                                case DBCOM_MONSTERID(u8"食人花"):
                                    {
                                        m_coList[smA.UID] = std::make_unique<ClientPiranhaPlant>(smA.UID, this, smA.action);
                                        return;
                                    }
                                default:
                                    {
                                        if(DBCOM_MONSTERRECORD(monID).guard){
                                            m_coList[smA.UID] = std::make_unique<ClientGuard>(smA.UID, this, smA.action);
                                        }
                                        else{
                                            m_coList[smA.UID] = std::make_unique<ClientMonster>(smA.UID, this, smA.action);
                                        }
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
                m_coList[smA.UID] = std::make_unique<ClientNPC>(smA.UID, this, smA.action);
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
    if(smCOR.mapID != mapID()){
        return;
    }

    if(auto p = m_coList.find(smCOR.UID); p != m_coList.end()){
        p->second->parseAction(smCOR.action);
        return;
    }

    switch(uidf::getUIDType(smCOR.UID)){
        case UID_MON:
            {
                switch(const auto monID = uidf::getMonsterID(smCOR.UID)){
                    case DBCOM_MONSTERID(u8"变异骷髅"):
                        {
                            m_coList[smCOR.UID].reset(new ClientTaoSkeleton(smCOR.UID, this, smCOR.action));
                            break;
                        }
                    case DBCOM_MONSTERID(u8"神兽"):
                        {
                            m_coList[smCOR.UID].reset(new ClientTaoDog(smCOR.UID, this, smCOR.action));
                            break;
                        }
                    default:
                        {
                            if(DBCOM_MONSTERRECORD(monID).guard){
                                m_coList[smCOR.UID] = std::make_unique<ClientGuard>(smCOR.UID, this, smCOR.action);
                            }
                            else{
                                m_coList[smCOR.UID] = std::make_unique<ClientMonster>(smCOR.UID, this, smCOR.action);
                            }
                            break;
                        }
                }
                break;
            }
        case UID_PLY:
            {
                m_coList[smCOR.UID] = std::make_unique<Hero>(smCOR.UID, this, smCOR.action);
                queryPlayerWLDesp(smCOR.UID);
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

    if(stSMUHP.mapID == mapID()){
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
        p->parseAction(ActionDie
        {
            .x = p->x(),
            .y = p->y(),
            .extParam
            {
                .fadeOut = true,
                .none = {},
            }
        });
    }
}

void ProcessRun::net_DEADFADEOUT(const uint8_t *bufPtr, size_t)
{
    SMDeadFadeOut stSMDFO;
    std::memcpy(&stSMDFO, bufPtr, sizeof(stSMDFO));

    if(stSMDFO.mapID == mapID()){
        if(auto p = findUID(stSMDFO.UID)){
            p->deadFadeOut();
        }
    }
}

void ProcessRun::net_EXP(const uint8_t *buf, size_t)
{
    const auto smExp = ServerMsg::conv<SMExp>(buf);
    const uint32_t currExp = getMyHero()->getExp();

    getMyHero()->setExp(smExp.exp);
    if((smExp.exp > currExp) && (currExp > 0)){
        addCBLog(CBLOG_SYS, u8"你获得了经验值%llu", to_llu(smExp.exp - currExp));
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

void ProcessRun::net_BUYERROR(const uint8_t *buf, size_t bufSize)
{
    const auto smBE = ServerMsg::conv<SMBuyError>(buf, bufSize);
    switch(smBE.error){
        case BUYERR_INSUFFCIENT:
            {
                addCBLog(CBLOG_ERR, u8"金币不够");
                break;
            }
        default:
            {
                addCBLog(CBLOG_ERR, u8"购买失败");
                break;
            }
    }
}

void ProcessRun::net_BUYSUCCEED(const uint8_t *buf, size_t)
{
    const auto smBS = ServerMsg::conv<SMBuySucceed>(buf);
    dynamic_cast<PurchaseBoard *>(getWidget("PurchaseBoard"))->onBuySucceed(smBS.npcUID, smBS.itemID, smBS.seqID);
}

void ProcessRun::net_GROUNDITEMIDLIST(const uint8_t *buf, size_t bufSize)
{
    const auto sdGIIDL = cerealf::deserialize<SDGroundItemIDList>(buf, bufSize);
    if(sdGIIDL.mapID != m_mapID){
        return;
    }

    for(const auto &[x, y, itemIDList]: sdGIIDL.gridItemIDList){
        if(!onMap(x, y)){
            throw fflerror("invalid grid: x= %d, y = %d", x, y);
        }

        clearGroundItemIDList(x, y);
        for(const auto itemID: itemIDList){
            if(!DBCOM_ITEMRECORD(itemID)){
                throw fflerror("invalid itemID = %llu", to_llu(itemID));
            }
            addGroundItemID(itemID, x, y);
        }
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
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"开始")))->addOnDone([smCM, this](MagicBase *)
                    {
                        if(auto coPtr = findUID(smCM.UID)){
                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"运行")));
                        }
                    });
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

    if(stSMO.mapID == mapID()){
        if(auto coPtr = findUID(stSMO.UID)){
            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"启动")));
        }
    }
}

void ProcessRun::net_PICKUPERROR(const uint8_t *buf, size_t)
{
    const auto smPUE = ServerMsg::conv<SMPickUpError>(buf);
    if(smPUE.failedItemID){
        if(const auto &ir = DBCOM_ITEMRECORD(smPUE.failedItemID)){
            addCBLog(CBLOG_SYS, u8"无法捡起%s", to_cstr(ir.name));
        }
        else{
            addCBLog(CBLOG_SYS, u8"无法捡起物品ID = %llu", to_cstr(ir.name), to_llu(smPUE.failedItemID));
        }
    }
    else{
        addCBLog(CBLOG_SYS, u8"当前无法捡起物品，请稍后再试");
    }
}

void ProcessRun::net_GOLD(const uint8_t *buf, size_t)
{
    auto myHeroPtr = getMyHero();
    const int lastGold = to_d(myHeroPtr->getGold());
    const int newGold = to_d(ServerMsg::conv<SMGold>(buf).gold);

    myHeroPtr->setGold(newGold);
    if(lastGold != newGold){
        addCBLog(CBLOG_SYS, u8"你%s了%d金币", to_cstr((lastGold < newGold) ? u8"获得" : u8"失去"), std::abs(lastGold - newGold));
    }
}

void ProcessRun::net_NPCXMLLAYOUT(const uint8_t *buf, size_t bufSize)
{
    const auto sdNPCXMLL = cerealf::deserialize<SDNPCXMLLayout>(buf, bufSize);
    auto npcChatBoardPtr  = dynamic_cast<NPCChatBoard  *>(getGUIManager()->getWidget("NPCChatBoard"));
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));

    npcChatBoardPtr->loadXML(sdNPCXMLL.npcUID, sdNPCXMLL.xmlLayout.c_str());
    npcChatBoardPtr->show(true);
    purchaseBoardPtr->show(false);
}

void ProcessRun::net_NPCSELL(const uint8_t *buf, size_t bufSize)
{
    auto sdNPCS = cerealf::deserialize<SDNPCSell>(buf, bufSize);
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));
    auto npcChatBoardPtr  = dynamic_cast<NPCChatBoard  *>(getGUIManager()->getWidget("NPCChatBoard"));

    purchaseBoardPtr->loadSell(sdNPCS.npcUID, std::move(sdNPCS.itemList));
    purchaseBoardPtr->show(true);
    npcChatBoardPtr->show(false);
}

void ProcessRun::net_TEXT(const uint8_t *buf, size_t)
{
    addCBLog(CBLOG_SYS, u8"%s", to_cstr((const char *)(buf)));
}

void ProcessRun::net_PLAYERWLDESP(const uint8_t *buf, size_t bufSize)
{
    auto sdUIDWLD = cerealf::deserialize<SDUIDWLDesp>(buf, bufSize);
    if(uidf::getUIDType(sdUIDWLD.uid) != UID_PLY){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(sdUIDWLD.uid));
    }

    if(auto playerPtr = dynamic_cast<Hero *>(findUID(sdUIDWLD.uid)); playerPtr){
        playerPtr->setWLDesp(std::move(sdUIDWLD.desp));
    }
}

void ProcessRun::net_PLAYERNAME(const uint8_t *buf, size_t)
{
    const auto smPN = ServerMsg::conv<SMPlayerName>(buf);
    if(uidf::getUIDType(smPN.uid) == UID_PLY){
        if(auto playerPtr = dynamic_cast<Hero *>(findUID(smPN.uid)); playerPtr){
            playerPtr->setName(smPN.name, smPN.nameColor);
        }
    }
}

void ProcessRun::net_UPDATEITEM(const uint8_t *buf, size_t bufSize)
{
    const auto sdUI = cerealf::deserialize<SDUpdateItem>(buf, bufSize);
    const auto &ir = DBCOM_ITEMRECORD(sdUI.item.itemID);
    if(!ir){
        throw fflerror("bad item: itemID = %llu", to_llu(sdUI.item.itemID));
    }

    const auto changed = getMyHero()->getInvPack().update(sdUI.item);
    if(changed != 0){
        if(ir.packable()){
            addCBLog(CBLOG_SYS, u8"你%s了%d个%s", to_cstr((changed > 0) ? u8"获得" : u8"失去"), std::abs(changed), to_cstr(ir.name));
        }
        else{
            addCBLog(CBLOG_SYS, u8"你%s了%s", to_cstr((changed > 0) ? u8"获得" : u8"失去"), to_cstr(ir.name));
        }
    }
}

void ProcessRun::net_REMOVEITEM(const uint8_t *buf, size_t)
{
    const auto smRI = ServerMsg::conv<SMRemoveItem>(buf);
    /* */ auto grabbedItem = getMyHero()->getInvPack().getGrabbedItem();
    if((smRI.itemID == grabbedItem.itemID) && (smRI.seqID == grabbedItem.seqID)){
        if(smRI.count < grabbedItem.count){
            grabbedItem.count -= smRI.count;
            getMyHero()->getInvPack().setGrabbedItem(grabbedItem);
        }
        else{
            getMyHero()->getInvPack().setGrabbedItem({});
        }
    }
    else{
        getMyHero()->getInvPack().remove(smRI.itemID, smRI.seqID, smRI.count);
    }
}

void ProcessRun::net_INVENTORY(const uint8_t *buf, size_t bufSize)
{
    getMyHero()->getInvPack().setInventory(cerealf::deserialize<SDInventory>(buf, bufSize));
}

void ProcessRun::net_BELT(const uint8_t *buf, size_t bufSize)
{
    getMyHero()->setBelt(cerealf::deserialize<SDBelt>(buf, bufSize));
}

void ProcessRun::net_EQUIPWEAR(const uint8_t *buf, size_t bufSize)
{
    auto sdEW = cerealf::deserialize<SDEquipWear>(buf, bufSize);
    if(uidf::getUIDType(sdEW.uid) != UID_PLY){
        throw fflerror("invalid uid type to equip wear item: %s", uidf::getUIDTypeCStr(sdEW.uid));
    }

    if(auto coPtr = dynamic_cast<Hero *>(findUID(sdEW.uid))){
        coPtr->setWLItem(sdEW.wltype, std::move(sdEW.item));
    }
}

void ProcessRun::net_EQUIPWEARERROR(const uint8_t *buf, size_t)
{
    const auto smEWE = ServerMsg::conv<SMEquipWearError>(buf);
    switch(smEWE.error){
        case EQWERR_NOITEM:
        case EQWERR_BADITEM:
            {
                addCBLog(CBLOG_ERR, u8"无效的物品");
                return;
            }
        case EQWERR_BADWLTYPE:
            {
                addCBLog(CBLOG_ERR, u8"无法放置：%s", to_cstr(DBCOM_ITEMRECORD(smEWE.itemID).name));
                return;
            }
        default:
            {
                return;
            }
    }
}

void ProcessRun::net_GRABWEAR(const uint8_t *buf, size_t bufSize)
{
    auto sdGW = cerealf::deserialize<SDGrabWear>(buf, bufSize);
    if(!sdGW.item){
        throw fflerror("invalid wear item: %s", to_cstr(sdGW.item.str()));
    }

    const auto wltype = to_d(sdGW.wltype);
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wear grid type: %d", wltype);
    }

    auto myHeroPtr = getMyHero();
    auto &invPackRef = myHeroPtr->getInvPack();

    myHeroPtr->setWLItem(wltype, {});
    if(auto currItem = invPackRef.getGrabbedItem()){
        invPackRef.add(std::move(currItem));
    }
    invPackRef.setGrabbedItem(std::move(sdGW.item));
}

void ProcessRun::net_GRABWEARERROR(const uint8_t *buf, size_t)
{
    const auto smGWE = ServerMsg::conv<SMGrabWearError>(buf);
    switch(smGWE.error){
        case GWERR_BIND:
            {
                addCBLog(CBLOG_ERR, u8"无法取下装备");
                return;
            }
        case GWERR_NOITEM:
        default:
            {
                break;
            }
    }
}

void ProcessRun::net_EQUIPBELT(const uint8_t *buf, size_t bufSize)
{
    auto sdEB = cerealf::deserialize<SDEquipBelt>(buf, bufSize);
    const auto &ir = DBCOM_ITEMRECORD(sdEB.item.itemID);
    if(!ir.beltable()){
        throw fflerror("invalid item to equip belt: %s", to_cstr(sdEB.item.str()));
    }
    getMyHero()->setBelt(sdEB.slot, std::move(sdEB.item));
}

void ProcessRun::net_EQUIPBELTERROR(const uint8_t *buf, size_t)
{
    const auto smEBE = ServerMsg::conv<SMEquipBeltError>(buf);
    switch(smEBE.error){
        case EQBERR_NOITEM:
        case EQBERR_BADITEM:
            {
                addCBLog(CBLOG_ERR, u8"无效的物品");
                return;
            }
        case EQBERR_BADITEMTYPE:
            {
                addCBLog(CBLOG_ERR, u8"无法装备：%s", to_cstr(DBCOM_ITEMRECORD(smEBE.itemID).name));
                return;
            }
        default:
            {
                return;
            }
    }
}

void ProcessRun::net_GRABBELT(const uint8_t *buf, size_t bufSize)
{
    auto sdGB = cerealf::deserialize<SDGrabBelt>(buf, bufSize);
    if(!sdGB.item){
        throw fflerror("invalid belt item: %s", to_cstr(sdGB.item.str()));
    }

    const auto slot = to_d(sdGB.slot);
    if(!(slot >= 0 && slot < 6)){
        throw fflerror("invalid belt slot: %d", slot);
    }

    auto myHeroPtr = getMyHero();
    auto &invPackRef = myHeroPtr->getInvPack();

    myHeroPtr->setBelt(sdGB.slot, {});
    if(auto currItem = invPackRef.getGrabbedItem()){
        invPackRef.add(std::move(currItem));
    }
    invPackRef.setGrabbedItem(std::move(sdGB.item));
}

void ProcessRun::net_GRABBELTERROR(const uint8_t *buf, size_t)
{
    const auto smGBE = ServerMsg::conv<SMGrabBeltError>(buf);
    switch(smGBE.error){
        default:
            {
                break;
            }
    }
}
