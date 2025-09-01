#include <memory>
#include <cstring>
#include "luaf.hpp"
#include "strf.hpp"
#include "log.hpp"
#include "pathf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "clientargparser.hpp"
#include "clientmonster.hpp"
#include "clienttaodog.hpp"
#include "clienttaoskeleton.hpp"
#include "clienttaoskeletonext.hpp"
#include "clientnpc.hpp"
#include "uidf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "raiitimer.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "cerealf.hpp"
#include "imeboard.hpp"
#include "gui/controlboard/controlboard.hpp"
#include "gui/friendchatboard/friendchatboard.hpp"
#include "serdesmsg.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

void ProcessRun::on_SM_STARTGAMESCENE(const uint8_t *buf, size_t bufSize)
{
    const auto sdSGS = cerealf::deserialize<SDStartGameScene>(buf, bufSize);

    // verfy mapID and myHeroUID
    // myHeroUID and mapID has been sent by smOnlineOK

    fflassert(sdSGS.mapUID == mapUID(), sdSGS.mapUID, mapUID());
    fflassert(sdSGS.uid == getMyHeroUID(), sdSGS.uid, getMyHeroUID());

    getMyHero()->parseAction(ActionStand
    {
        .direction = DIR_DOWN,
        .x = sdSGS.x,
        .y = sdSGS.y,
    });

    getMyHero()->setName(to_cstr(sdSGS.name), sdSGS.nameColor);
    getMyHero()->setWLDesp(std::move(sdSGS.desp));

    centerMyHero();
    getMyHero()->pullGold();

    if(!g_clientArgParser->inputScript.empty()){
        std::vector<std::string> error;
        const auto pfr = m_luaModule.execFile(g_clientArgParser->inputScript.c_str());

        if(m_luaModule.pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
            addCBLog(CBLOG_SYS, u8"脚本返回: %s", str_any(pfr).c_str());
        }
        else{
            addCBLog(CBLOG_ERR, u8"脚本错误: %s", g_clientArgParser->inputScript.c_str());
            for(const auto &err: error){
                addCBLog(CBLOG_ERR, u8"%s", err.c_str());
            }
        }
    }
}

void ProcessRun::on_SM_PLAYERCONFIG(const uint8_t *buf, size_t bufSize)
{
    const auto sdPC = cerealf::deserialize<SDPlayerConfig>(buf, bufSize);
    dynamic_cast<RuntimeConfigBoard *>(getWidget("RuntimeConfigBoard"))->setConfig(sdPC.runtimeConfig);

    for(const auto &[magicID, key]: sdPC.magicKeyList.keyList){
        dynamic_cast<SkillBoard *>(getWidget("SkillBoard"))->getConfig().setMagicKey(magicID, key);
    }
}

void ProcessRun::on_SM_FRIENDLIST(const uint8_t *buf, size_t bufSize)
{
    const auto sdFL = cerealf::deserialize<SDFriendList>(buf, bufSize);
    dynamic_cast<FriendChatBoard *>(getWidget("FriendChatBoard"))->setFriendList(sdFL);
}

void ProcessRun::on_SM_CHATMESSAGELIST(const uint8_t *buf, size_t bufSize)
{
    for(const auto &message: cerealf::deserialize<SDChatMessageList>(buf, bufSize)){
        dynamic_cast<FriendChatBoard *>(getWidget("FriendChatBoard"))->addMessage({}, message);
    }
    dynamic_cast<ControlBoard *>(getWidget("ControlBoard"))->getButton("FriendChat")->setBlinkTime(100, 100, 5000);
}

void ProcessRun::on_SM_LEARNEDMAGICLIST(const uint8_t *buf, size_t bufSize)
{
    const auto sdLML = cerealf::deserialize<SDLearnedMagicList>(buf, bufSize);
    for(const auto &magic: sdLML.magicList){
        dynamic_cast<SkillBoard *>(getWidget("SkillBoard"))->getConfig().setMagicLevel(magic.magicID, 1);
    }
}

void ProcessRun::on_SM_SELLITEMLIST(const uint8_t *buf, size_t bufSize)
{
    auto sdSIL = cerealf::deserialize<SDSellItemList>(buf, bufSize);
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));
    purchaseBoardPtr->setSellItemList(std::move(sdSIL));
}

void ProcessRun::on_SM_ACTION(const uint8_t *bufPtr, size_t)
{
    SMAction smA;
    std::memcpy(&smA, bufPtr, sizeof(smA));

    if(smA.mapUID != mapUID()){
        if(smA.UID != getMyHero()->UID()){
            m_coList.erase(smA.UID);
            return;
        }

        // detected map switch for myhero
        // need to do map switch and parse current action

        // call getMyHero before loadMap()
        // getMyHero() checks if current creature is on current map

        m_actionBlocker.clear();
        getMyHero()->flushForcedMotion();
        loadMap(smA.mapUID, smA.action.x, smA.action.y);

        // directly assign a stand motion
        // this need to skip all location validation

        auto myHeroPtr = std::move(m_coList[m_myHeroUID]);
        const auto oldDir = myHeroPtr->currMotion()->direction;
        dynamic_cast<Hero *>(myHeroPtr.get())->jumpLoc(smA.action.x, smA.action.y, oldDir);

        m_coList.clear();
        m_coList[m_myHeroUID] = std::move(myHeroPtr);

        m_fixedLocMagicList.clear();
        m_followUIDMagicList.clear();
        m_actionBlocker.clear();
        m_ascendStrList.clear();

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
            // TODO ACTION_SPAWN should be the first action
            //      but current implementation sometimes ACTION_SPAWN comes later, ignore it
            return;
        }

        // clear all pending action
        // this helps to avoid the motion run back and forth
        if(smA.UID == getMyHeroUID()){
            getMyHero()->clearActionQueue();
        }

        coPtr->parseAction(smA.action);
        switch(smA.action.type){
            case ACTION_SPACEMOVE:
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
                            switch(uidf::getMonsterID(smA.UID)){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID].reset(new ClientTaoSkeleton(smA.UID, this, smA.action));
                                            queryUIDBuff(smA.UID);
                                        }
                                        return;
                                    }
                                case DBCOM_MONSTERID(u8"超强骷髅"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID].reset(new ClientTaoSkeletonExt(smA.UID, this, smA.action));
                                            queryUIDBuff(smA.UID);
                                        }
                                        return;
                                    }
                                case DBCOM_MONSTERID(u8"神兽"):
                                    {
                                        if(!m_actionBlocker.contains(smA.UID)){
                                            m_coList[smA.UID].reset(new ClientTaoDog(smA.UID, this, smA.action));
                                            queryUIDBuff(smA.UID);
                                        }
                                        return;
                                    }
                                default:
                                    {
                                        m_coList[smA.UID].reset(ClientMonster::create(smA.UID, this, smA.action));
                                        queryUIDBuff(smA.UID);
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
                m_coList[smA.UID].reset(new ClientNPC(smA.UID, this, smA.action));
                return;
            }
        default:
            {
                return;
            }
    }
}

void ProcessRun::on_SM_CORECORD(const uint8_t *bufPtr, size_t)
{
    const auto smCOR = ServerMsg::conv<SMCORecord>(bufPtr);
    if(smCOR.mapUID != mapUID()){
        return;
    }

    if(auto p = m_coList.find(smCOR.UID); p != m_coList.end()){
        p->second->parseAction(smCOR.action);
        return;
    }

    switch(uidf::getUIDType(smCOR.UID)){
        case UID_MON:
            {
                m_coList[smCOR.UID].reset(ClientMonster::create(smCOR.UID, this, smCOR.action));
                queryUIDBuff(smCOR.UID);
                break;
            }
        case UID_PLY:
            {
                m_coList[smCOR.UID].reset(new Hero(smCOR.UID, smCOR.Player.gender, smCOR.Player.job, this, smCOR.action));
                queryUIDBuff(smCOR.UID);
                queryPlayerWLDesp(smCOR.UID);
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::on_SM_HEALTH(const uint8_t *buf, size_t bufSize)
{
    auto sdH = cerealf::deserialize<SDHealth>(buf, bufSize);
    if(auto p = findUID(sdH.uid)){
        p->updateHealth(std::move(sdH));
    }
}

void ProcessRun::on_SM_BUFFIDLIST(const uint8_t *buf, size_t bufSize)
{
    auto sdBIDL = cerealf::deserialize<SDBuffIDList>(buf, bufSize);
    if(auto p = findUID(sdBIDL.uid)){
        p->setBuffIDList(std::move(sdBIDL));
    }
}

void ProcessRun::on_SM_NEXTSTRIKE(const uint8_t *, size_t)
{
    getMyHero()->setNextStrike(true);
}

void ProcessRun::on_SM_NOTIFYDEAD(const uint8_t *bufPtr, size_t)
{
    SMNotifyDead stSMND;
    std::memcpy(&stSMND, bufPtr, sizeof(stSMND));

    if(auto p = findUID(stSMND.UID)){
        p->parseAction(ActionDie
        {
            .x = p->x(),
            .y = p->y(),
        });
    }
}

void ProcessRun::on_SM_DEADFADEOUT(const uint8_t *bufPtr, size_t)
{
    SMDeadFadeOut stSMDFO;
    std::memcpy(&stSMDFO, bufPtr, sizeof(stSMDFO));

    if(stSMDFO.mapUID == mapUID()){
        if(auto p = findUID(stSMDFO.UID)){
            p->deadFadeOut();
        }
    }
}

void ProcessRun::on_SM_EXP(const uint8_t *buf, size_t)
{
    const auto smExp = ServerMsg::conv<SMExp>(buf);
    const uint32_t currExp = getMyHero()->getExp();

    getMyHero()->setExp(smExp.exp);
    if((smExp.exp > currExp) && (currExp > 0)){
        addCBLog(CBLOG_SYS, u8"你获得了经验值%llu", to_llu(smExp.exp - currExp));
    }
}

void ProcessRun::on_SM_BUFF(const uint8_t *buf, size_t)
{
    const auto smB = ServerMsg::conv<SMBuff>(buf);
    if(auto coPtr = findUID(smB.uid)){
        coPtr->setBuff(smB.type, smB.state);
    }
}

void ProcessRun::on_SM_MISS(const uint8_t *bufPtr, size_t)
{
    SMMiss stSMM;
    std::memcpy(&stSMM, bufPtr, sizeof(stSMM));

    if(auto p = findUID(stSMM.UID)){
        int nX = p->x() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2 - 20;
        int nY = p->y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;
        addAscendStr(ASCENDSTR_MISS, 0, nX, nY);
    }
}

void ProcessRun::on_SM_PING(const uint8_t *bufPtr, size_t)
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

void ProcessRun::on_SM_BUYERROR(const uint8_t *buf, size_t bufSize)
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

void ProcessRun::on_SM_BUYSUCCEED(const uint8_t *buf, size_t)
{
    const auto smBS = ServerMsg::conv<SMBuySucceed>(buf);
    dynamic_cast<PurchaseBoard *>(getWidget("PurchaseBoard"))->onBuySucceed(smBS.npcUID, smBS.itemID, smBS.seqID);
}

void ProcessRun::on_SM_GROUNDITEMIDLIST(const uint8_t *buf, size_t bufSize)
{
    const auto sdGIIDL = cerealf::deserialize<SDGroundItemIDList>(buf, bufSize);
    if(sdGIIDL.mapUID != mapUID()){
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

void ProcessRun::on_SM_GROUNDFIREWALLLIST(const uint8_t *buf, size_t bufSize)
{
    const auto sdGFWL = cerealf::deserialize<SDGroundFireWallList>(buf, bufSize);
    if(sdGFWL.mapUID != mapUID()){
        return;
    }

    for(auto p = sdGFWL.fireWallList.begin(); p != sdGFWL.fireWallList.end(); ++p){
        fflassert(p->count >= 0);
        fflassert(onMap(p->x, p->y));

        std::vector<FireWall_RUN *> runList;
        for(auto imagic = m_fixedLocMagicList.begin(); imagic != m_fixedLocMagicList.end(); imagic++){
            if(true
                    && imagic->get()->x() == p->x
                    && imagic->get()->y() == p->y
                    && imagic->get()->checkMagic(u8"火墙", u8"运行")){
                if(auto magicPtr = dynamic_cast<FireWall_RUN *>(imagic->get()); magicPtr && !magicPtr->hasFadeOut()){
                    runList.push_back(magicPtr);
                }
            }
        }

        // damage is done by server
        // client side firewall doesn't need to keep caster infomation

        if(const int irunDiff = to_d(runList.size()) - p->count; irunDiff != 0){
            if(irunDiff > 0){
                for(int i = 0; i < irunDiff; ++i){
                    constexpr int fadeOutTime = 3000;
                    runList[i]->setFadeOut(fadeOutTime);
                    addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FireAshEffect_RUN
                    {
                        p->x,
                        p->y,
                        fadeOutTime, // use fadeOutTime as FireAsh fadeIn time
                    }));
                }
            }
            else{
                for(int i = 0; i < std::abs(irunDiff); ++i){
                    addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FireWall_RUN
                    {
                        p->x,
                        p->y,
                    }))->addTrigger([this](BaseMagic *magic)
                    {
                        if(const auto fireWallMagic = dynamic_cast<FireWall_RUN *>(magic)){
                            if(const auto seffIDOpt = fireWallMagic->getSeffID(); seffIDOpt.has_value()){
                                playSoundEffectAt(seffIDOpt.value(), fireWallMagic->x(), fireWallMagic->y());
                            }
                        }
                        return true;
                    });
                }
            }
        }
    }
}

void ProcessRun::on_SM_CASTMAGIC(const uint8_t *bufPtr, size_t)
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
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"运行")))->addOnDone([smCM, this](BaseMagic *)
                    {
                        if(auto coPtr = findUID(smCM.UID)){
                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"运行")));
                        }
                    });
                }
                return;
            }
        case DBCOM_MAGICID(u8"阴阳法环"):
            {
                if(auto coPtr = findUID(smCM.UID)){
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"阴阳法环", u8"运行")))->addOnDone([smCM, this](BaseMagic *)
                    {
                        if(auto coPtr = findUID(smCM.UID)){
                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new TaoYellowBlueRing()));
                        }
                    });
                }
                return;
            }
        case DBCOM_MAGICID(u8"雷电术"):
        case DBCOM_MAGICID(u8"沃玛教主_雷电术"):
            {
                if(auto coPtr = findUID(smCM.AimUID)){
                    coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new Thunderbolt()))->addTrigger([smCM, this](BaseMagic *magic)
                    {
                        if(auto targetPtr = findUID(smCM.AimUID)){
                            if(const auto seffIDOpt = magic->getSeffID(); seffIDOpt.has_value()){
                                targetPtr->playSoundEffect(seffIDOpt.value());
                            }
                        }
                        return true;
                    });
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

void ProcessRun::on_SM_OFFLINE(const uint8_t *bufPtr, size_t)
{
    const auto smO = ServerMsg::conv<SMOffline>(bufPtr);
    if(smO.mapUID == mapUID()){
        m_coList.erase(smO.UID);
        // if(auto coPtr = findUID(smO.UID)){
        //     coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"启动")));
        // }
    }
}

void ProcessRun::on_SM_PICKUPERROR(const uint8_t *buf, size_t)
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

void ProcessRun::on_SM_STARTINVOP(const uint8_t *buf, size_t size)
{
    fflassert(buf);
    fflassert(size > 0);
    auto invBoardPtr = dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard"));

    invBoardPtr->setShow(true);
    invBoardPtr->moveAt(DIR_UPRIGHT, g_sdlDevice->getRendererWidth() - 1, 0);
    invBoardPtr->startInvOp(cerealf::deserialize<SDStartInvOp>(buf, size));
}

void ProcessRun::on_SM_GOLD(const uint8_t *buf, size_t)
{
    auto myHeroPtr = getMyHero();
    const int lastGold = to_d(myHeroPtr->getGold());
    const int newGold = to_d(ServerMsg::conv<SMGold>(buf).gold);

    myHeroPtr->setGold(newGold);
    if(lastGold != newGold){
        addCBLog(CBLOG_SYS, u8"你%s了%d金币", to_cstr((lastGold < newGold) ? u8"获得" : u8"失去"), std::abs(lastGold - newGold));
    }
}

void ProcessRun::on_SM_INVOPCOST(const uint8_t *buf, size_t)
{
    const auto smIOPC = ServerMsg::conv<SMInvOpCost>(buf);
    dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard"))->setInvOpCost(smIOPC.invOp, smIOPC.itemID, smIOPC.seqID, smIOPC.cost);
}

void ProcessRun::on_SM_NPCXMLLAYOUT(const uint8_t *buf, size_t bufSize)
{
    const auto sdNPCXMLL = cerealf::deserialize<SDNPCXMLLayout>(buf, bufSize);
    auto npcChatBoardPtr  = dynamic_cast<NPCChatBoard  *>(getGUIManager()->getWidget("NPCChatBoard"));
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));

    npcChatBoardPtr->loadXML(sdNPCXMLL.npcUID, sdNPCXMLL.eventPath.c_str(), sdNPCXMLL.xmlLayout.c_str());
    npcChatBoardPtr->setShow(true);
    purchaseBoardPtr->setShow(false);
}

void ProcessRun::on_SM_NPCSELL(const uint8_t *buf, size_t bufSize)
{
    auto sdNPCS = cerealf::deserialize<SDNPCSell>(buf, bufSize);
    auto purchaseBoardPtr = dynamic_cast<PurchaseBoard *>(getGUIManager()->getWidget("PurchaseBoard"));
    auto npcChatBoardPtr  = dynamic_cast<NPCChatBoard  *>(getGUIManager()->getWidget("NPCChatBoard"));

    if(npcChatBoardPtr->show()){
        // purchaseBoardPtr->moveTo(npcChatBoardPtr->x(), npcChatBoardPtr->y() + npcChatBoardPtr->h());
        purchaseBoardPtr->moveTo(0, 0 + npcChatBoardPtr->h());
    }
    else{
        purchaseBoardPtr->moveTo(0, 0);
    }

    purchaseBoardPtr->loadSell(sdNPCS.npcUID, std::move(sdNPCS.itemList));
    purchaseBoardPtr->setShow(true);
}

void ProcessRun::on_SM_TEXT(const uint8_t *buf, size_t bufSize)
{
    addCBLog(CBLOG_SYS, u8"%s", std::string(buf, buf + bufSize).c_str());
}

void ProcessRun::on_SM_PLAYERWLDESP(const uint8_t *buf, size_t bufSize)
{
    auto sdUIDWLD = cerealf::deserialize<SDUIDWLDesp>(buf, bufSize);
    if(uidf::getUIDType(sdUIDWLD.uid) != UID_PLY){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(sdUIDWLD.uid));
    }

    if(auto playerPtr = dynamic_cast<Hero *>(findUID(sdUIDWLD.uid)); playerPtr){
        playerPtr->setWLDesp(std::move(sdUIDWLD.desp));
    }
}

void ProcessRun::on_SM_PLAYERNAME(const uint8_t *buf, size_t bufSize)
{
    const auto sdPN = cerealf::deserialize<SDPlayerName>(buf, bufSize);
    if(uidf::isPlayer(sdPN.uid)){
        if(auto playerPtr = dynamic_cast<Hero *>(findUID(sdPN.uid)); playerPtr){
            playerPtr->setName(sdPN.name.c_str(), sdPN.nameColor);
        }
    }
}

void ProcessRun::on_SM_STRIKEGRID(const uint8_t *buf, size_t)
{
    const auto smSG = ServerMsg::conv<SMStrikeGrid>(buf);
    m_strikeGridList[{smSG.x, smSG.y}] = hres_tstamp().to_msec();
}

void ProcessRun::on_SM_UPDATEITEM(const uint8_t *buf, size_t bufSize)
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

    if(changed > 0){
        dynamic_cast<ControlBoard *>(getWidget("ControlBoard"))->getButton("Inventory")->setBlinkTime(100, 100, 5000);
    }
}

void ProcessRun::on_SM_REMOVEITEM(const uint8_t *buf, size_t)
{
    const auto smRI = ServerMsg::conv<SMRemoveItem>(buf);
    dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard"))->removeItem(smRI.itemID, smRI.seqID, smRI.count);
}

void ProcessRun::on_SM_REMOVESECUREDITEM(const uint8_t *buf, size_t)
{
    const auto smRI = ServerMsg::conv<SMRemoveSecuredItem>(buf);
    dynamic_cast<SecuredItemListBoard *>(getWidget("SecuredItemListBoard"))->removeItem(smRI.itemID, smRI.seqID);
}

void ProcessRun::on_SM_INVENTORY(const uint8_t *buf, size_t bufSize)
{
    getMyHero()->getInvPack().setInventory(cerealf::deserialize<SDInventory>(buf, bufSize));
}

void ProcessRun::on_SM_BELT(const uint8_t *buf, size_t bufSize)
{
    getMyHero()->setBelt(cerealf::deserialize<SDBelt>(buf, bufSize));
}

void ProcessRun::on_SM_EQUIPWEAR(const uint8_t *buf, size_t bufSize)
{
    auto sdEW = cerealf::deserialize<SDEquipWear>(buf, bufSize);
    if(uidf::getUIDType(sdEW.uid) != UID_PLY){
        throw fflerror("invalid uid type to equip wear item: %s", uidf::getUIDTypeCStr(sdEW.uid));
    }

    if(auto coPtr = dynamic_cast<Hero *>(findUID(sdEW.uid))){
        coPtr->setWLItem(sdEW.wltype, std::move(sdEW.item), sdEW.uid == m_myHeroUID);
    }
}

void ProcessRun::on_SM_EQUIPWEARERROR(const uint8_t *buf, size_t)
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

void ProcessRun::on_SM_GRABWEAR(const uint8_t *buf, size_t bufSize)
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

void ProcessRun::on_SM_GRABWEARERROR(const uint8_t *buf, size_t)
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

void ProcessRun::on_SM_EQUIPBELT(const uint8_t *buf, size_t bufSize)
{
    auto sdEB = cerealf::deserialize<SDEquipBelt>(buf, bufSize);
    const auto &ir = DBCOM_ITEMRECORD(sdEB.item.itemID);
    if(!ir.beltable()){
        throw fflerror("invalid item to equip belt: %s", to_cstr(sdEB.item.str()));
    }
    getMyHero()->setBelt(sdEB.slot, std::move(sdEB.item));
}

void ProcessRun::on_SM_EQUIPBELTERROR(const uint8_t *buf, size_t)
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

void ProcessRun::on_SM_GRABBELT(const uint8_t *buf, size_t bufSize)
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

void ProcessRun::on_SM_GRABBELTERROR(const uint8_t *buf, size_t)
{
    const auto smGBE = ServerMsg::conv<SMGrabBeltError>(buf);
    switch(smGBE.error){
        default:
            {
                break;
            }
    }
}

void ProcessRun::on_SM_CREATECHATGROUP(const uint8_t *buf, size_t size)
{
    const auto sdCP = cerealf::deserialize<SDChatPeer>(buf, size);
    dynamic_cast<FriendChatBoard *>(getWidget("FriendChatBoard"))->addGroup(sdCP);
}

void ProcessRun::on_SM_STARTINPUT(const uint8_t *buf, size_t bufSize)
{
    const auto sdSI = cerealf::deserialize<SDStartInput>(buf, bufSize);
    auto inputBoardPtr = dynamic_cast<InputStringBoard *>(getWidget("InputStringBoard"));

    inputBoardPtr->setSecurity(true);
    inputBoardPtr->waitInput(to_u8cstr(sdSI.title), [uid = sdSI.uid, commitTag = sdSI.commitTag, this](std::u8string input)
    {
        sendNPCEvent(uid, {}, commitTag, to_cstr(input));
    });
}

void ProcessRun::on_SM_SHOWSECUREDITEMLIST(const uint8_t *buf, size_t bufSize)
{
    auto chatBoardPtr = dynamic_cast<NPCChatBoard *>(getWidget("NPCChatBoard"));
    auto itemBoardPtr = dynamic_cast<SecuredItemListBoard *>(getWidget("SecuredItemListBoard"));

    if(chatBoardPtr->show()){
        // itemBoardPtr->moveTo(chatBoardPtr->x(), chatBoardPtr->y() + chatBoardPtr->h());
        itemBoardPtr->moveTo(0, 0 + chatBoardPtr->h());
    }
    else{
        itemBoardPtr->moveTo(0, 0);
    }

    itemBoardPtr->setItemList(std::move(cerealf::deserialize<SDShowSecuredItemList>(buf, bufSize).itemList));
    itemBoardPtr->setShow(true);

    auto invBoardPtr = dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard"));
    invBoardPtr->setShow(true);
    invBoardPtr->moveAt(DIR_UPRIGHT, g_sdlDevice->getRendererWidth() - 1, 0);
}

void ProcessRun::on_SM_TEAMCANDIDATE(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<TeamStateBoard *>(getWidget("TeamStateBoard"))->addTeamCandidate(cerealf::deserialize<SDTeamCandidate>(buf, bufSize));
}

void ProcessRun::on_SM_TEAMMEMBERLIST(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<TeamStateBoard *>(getWidget("TeamStateBoard"))->setTeamMemberList(cerealf::deserialize<SDTeamMemberList>(buf, bufSize));
}

void ProcessRun::on_SM_QUESTDESPUPDATE(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<QuestStateBoard *>(getWidget("QuestStateBoard"))->updateQuestDesp(cerealf::deserialize<SDQuestDespUpdate>(buf, bufSize));
}

void ProcessRun::on_SM_QUESTDESPLIST(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<QuestStateBoard *>(getWidget("QuestStateBoard"))->setQuestDesp(cerealf::deserialize<SDQuestDespList>(buf, bufSize));
}

void ProcessRun::on_SM_ADDFRIENDACCEPTED(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<FriendChatBoard *>(getWidget("FriendChatBoard"))->onAddFriendAccepted(cerealf::deserialize<SDChatPeer>(buf, bufSize));
}

void ProcessRun::on_SM_ADDFRIENDREJECTED(const uint8_t *buf, size_t bufSize)
{
    dynamic_cast<FriendChatBoard *>(getWidget("FriendChatBoard"))->onAddFriendRejected(cerealf::deserialize<SDChatPeer>(buf, bufSize));
}
