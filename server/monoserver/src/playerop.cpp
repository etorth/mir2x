/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
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

#include <cinttypes>
#include "totype.hpp"
#include "mathf.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "actorpod.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"
#include "colorf.hpp"
#include "cerealf.hpp"
#include "serdesmsg.hpp"
#include "buildconfig.hpp"

extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

void Player::on_AM_METRONOME(const ActorMsgPack &)
{
    update();
}

void Player::on_AM_BADACTORPOD(const ActorMsgPack &rstMPK)
{
    AMBadActorPod amBAP;
    std::memcpy(&amBAP, rstMPK.data(), sizeof(amBAP));
    reportDeadUID(amBAP.UID);
}

void Player::on_AM_BINDCHANNEL(const ActorMsgPack &rstMPK)
{
    AMBindChannel amBC;
    std::memcpy(&amBC, rstMPK.data(), sizeof(amBC));

    // bind channel here
    // set the channel actor as this->GetAddress()
    m_channID = amBC.channID;
    g_netDriver->bindActor(channID(), UID());

    postOnLoginOK();
    PullRectCO(10, 10);
}

void Player::on_AM_SENDPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amSP = mpk.conv<AMSendPackage>();
    sendNetBuf(amSP.package.type, amSP.package.buf(), amSP.package.size);
    freeActorDataPackage(&(amSP.package));
}

void Player::on_AM_RECVPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amRP = mpk.conv<AMRecvPackage>();
    operateNet(amRP.package.type, amRP.package.buf(), amRP.package.size);
    freeActorDataPackage(&(amRP.package));
}

void Player::on_AM_ACTION(const ActorMsgPack &rstMPK)
{
    AMAction amA;
    std::memcpy(&amA, rstMPK.data(), sizeof(amA));

    if(amA.UID == UID()){
        return;
    }

    if(amA.mapID != mapID()){
        RemoveInViewCO(amA.UID);
        return;
    }

    // for all action types
    // the x/y are always well-defined

    int nDirection = -1;
    switch(amA.action.type){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                nDirection = amA.action.direction;
                break;
            }
        default:
            {
                break;
            }
    }

    if(mathf::LDistance2<int>(amA.action.x, amA.action.y, X(), Y()) > 20 * 20){
        return;
    }

    switch(amA.action.type){
        case ACTION_SPAWN:
        case ACTION_SPACEMOVE2:
            {
                dispatchAction(amA.UID, makeActionStand());
                break;
            }
        default:
            {
                break;
            }
    }

    AddInViewCO(amA.UID, amA.mapID, amA.action.x, amA.action.y, nDirection);
    reportAction(amA.UID, amA.action);
}

void Player::on_AM_NOTIFYNEWCO(const ActorMsgPack &mpk)
{
    const auto amNNCO = mpk.conv<AMNotifyNewCO>();
    if(m_dead.get()){
        notifyDead(amNNCO.UID);
    }
    else{
        // should make an valid action node and send it
        // currently just dispatch through map
        dispatchAction(amNNCO.UID, makeActionStand());
    }
}

void Player::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Player::on_AM_MAPSWITCH(const ActorMsgPack &mpk)
{
    const auto amMS = mpk.conv<AMMapSwitch>();
    if(!(amMS.UID && amMS.mapID)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Map switch request failed: (UID = %llu, mapID = %llu)", to_llu(amMS.UID), to_llu(amMS.mapID));
    }
    requestMapSwitch(amMS.mapID, amMS.X, amMS.Y, false);
}

void Player::on_AM_NPCQUERY(const ActorMsgPack &mpk)
{
    const auto tokenList = parseNPCQuery(mpk.conv<AMNPCQuery>().query);
    fflassert(!tokenList.empty());

    AMNPCEvent amNPCE;
    std::memset(&amNPCE, 0, sizeof(amNPCE));
    std::strcpy(amNPCE.event, tokenList.front().c_str());

    auto fnResp = [amNPCE, from = mpk.from(), seqID = mpk.seqID(), this](std::string value) mutable
    {
        fflassert(value.length() < std::extent_v<decltype(amNPCE.value)>);
        std::strcpy(amNPCE.value, value.c_str());
        m_actorPod->forward(from, {AM_NPCEVENT, amNPCE}, seqID);
    };

    if(tokenList.front() == "GOLD"){
        fnResp(std::to_string(m_sdItemStorage.gold));
        return;
    }

    if(tokenList.front() == "LEVEL"){
        fnResp(std::to_string(level()));
        return;
    }

    if(tokenList.front() == "NAME"){
        fnResp(to_cstr(m_name));
        return;
    }

    if(tokenList.front() == "SPACEMOVE"){
        const auto argMapID = std::stoi(tokenList.at(1));
        const auto argX     = std::stoi(tokenList.at(2));
        const auto argY     = std::stoi(tokenList.at(3));

        if(to_u32(argMapID) == mapID()){
            requestSpaceMove(argX, argY, false,
            [fnResp]() mutable
            {
                fnResp("1");
            },
            [fnResp]() mutable
            {
                fnResp("0");
            });
        }
        else{
            requestMapSwitch(argMapID, argX, argY, false,
            [fnResp]() mutable
            {
                fnResp("1");
            },
            [fnResp]() mutable
            {
                fnResp("0");
            });
        }
        return;
    }

    if(tokenList.front() == "CONSUME"){
        const auto argItemID = to_u32(std::stoi(tokenList.at(1)));
        const auto argCount  = to_uz (std::stoi(tokenList.at(2)));

        if(argItemID == DBCOM_ITEMID(u8"金币")){
            if(m_sdItemStorage.gold >= argCount){
                fnResp("1");
                setGold(m_sdItemStorage.gold - argCount);
            }
            else{
                fnResp("0");
            }
        }
        else{
            const auto delCount = removeInventoryItem(argItemID, 0, argCount);
            fnResp(delCount ? "1" : "0");
        }
        return;
    }
    fnResp(SYS_NPCERROR);
}

void Player::on_AM_QUERYLOCATION(const ActorMsgPack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.mapID     = mapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(rstMPK.from(), {AM_LOCATION, amL}, rstMPK.seqID());
}

void Player::on_AM_ATTACK(const ActorMsgPack &rstMPK)
{
    AMAttack amA;
    std::memcpy(&amA, rstMPK.data(), sizeof(amA));

    if(mathf::LDistance2(X(), Y(), amA.X, amA.Y) > 2){
        switch(uidf::getUIDType(amA.UID)){
            case UID_MON:
            case UID_PLY:
                {
                    AMMiss amM;
                    std::memset(&amM, 0, sizeof(amM));

                    amM.UID = amA.UID;
                    m_actorPod->forward(amA.UID, {AM_MISS, amM});
                    return;
                }
            default:
                {
                    return;
                }
        }
    }

    for(auto slaveUID: m_slaveList){
        m_actorPod->forward(slaveUID, AM_MASTERHITTED);
    }

    dispatchAction(ActionHitted
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    });
    StruckDamage({amA.UID, amA.Type, amA.Damage, amA.Element});
    reportAction(UID(), ActionHitted
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    });
}

void Player::on_AM_UPDATEHP(const ActorMsgPack &rstMPK)
{
    AMUpdateHP amUHP;
    std::memcpy(&amUHP, rstMPK.data(), sizeof(amUHP));

    if(amUHP.UID != UID()){
        SMUpdateHP smUHP;
        smUHP.UID   = amUHP.UID;
        smUHP.mapID = amUHP.mapID;
        smUHP.HP    = amUHP.HP;
        smUHP.HPMax = amUHP.HPMax;

        g_netDriver->Post(channID(), SM_UPDATEHP, smUHP);
    }
}

void Player::on_AM_DEADFADEOUT(const ActorMsgPack &rstMPK)
{
    AMDeadFadeOut amDFO;
    std::memcpy(&amDFO, rstMPK.data(), sizeof(amDFO));

    RemoveInViewCO(amDFO.UID);
    if(amDFO.UID != UID()){
        SMDeadFadeOut smDFO;
        smDFO.UID   = amDFO.UID;
        smDFO.mapID = amDFO.mapID;
        smDFO.X     = amDFO.X;
        smDFO.Y     = amDFO.Y;
        g_netDriver->Post(channID(), SM_DEADFADEOUT, smDFO);
    }
}

void Player::on_AM_EXP(const ActorMsgPack &mpk)
{
    const auto amE = mpk.conv<AMExp>();
    gainExp(amE.exp);
}

void Player::on_AM_MISS(const ActorMsgPack &rstMPK)
{
    AMMiss amM;
    std::memcpy(&amM, rstMPK.data(), sizeof(amM));

    SMMiss smM;
    std::memset(&smM, 0, sizeof(smM));

    smM.UID = amM.UID;
    postNetMessage(SM_MISS, smM);
}

void Player::on_AM_GIFT(const ActorMsgPack &mpk)
{
    const auto amG = mpk.conv<AMGift>();
    if(amG.itemID == DBCOM_ITEMID(u8"金币")){
        setGold(m_sdItemStorage.gold + amG.count);
    }
    else{
        if(DBCOM_ITEMRECORD(amG.itemID).packable()){
            addInventoryItem(SDItem
            {
                .itemID = amG.itemID,
                .count  = amG.count,
            }, false);
        }
        else{
            for(size_t i = 0; i < amG.count; ++i){
                addInventoryItem(SDItem
                {
                    .itemID = amG.itemID,
                    .count  = 1,
                }, false);
            }
        }
    }
}

void Player::on_AM_BADCHANNEL(const ActorMsgPack &rstMPK)
{
    AMBadChannel amBC;
    std::memcpy(&amBC, rstMPK.data(), sizeof(amBC));

    condcheck(channID() == amBC.channID);
    g_netDriver->Shutdown(channID(), false);

    Offline();
}

void Player::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    reportOffline(amO.UID, amO.mapID);
}

void Player::on_AM_QUERYPLAYERWLDESP(const ActorMsgPack &mpk)
{
    sendNetPackage(mpk.from(), SM_PLAYERWLDESP, cerealf::serialize(SDUIDWLDesp
    {
        .uid = UID(),
        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },
    }, true));
}

void Player::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &mpk)
{
    const auto amQFT = mpk.conv<AMQueryFriendType>();
    checkFriend(amQFT.UID, [this, mpk](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->forward(mpk.from(), {AM_FRIENDTYPE, amFT}, mpk.seqID());
    });
}

void Player::on_AM_REMOVEGROUNDITEM(const ActorMsgPack &rstMPK)
{
    AMRemoveGroundItem amRGI;
    std::memcpy(&amRGI, rstMPK.data(), sizeof(amRGI));

    SMRemoveGroundItem smRGI;
    smRGI.X    = amRGI.X;
    smRGI.Y    = amRGI.Y;
    smRGI.ID   = amRGI.ID;
    smRGI.DBID = amRGI.DBID;

    postNetMessage(SM_REMOVEGROUNDITEM, smRGI);
}

void Player::on_AM_CORECORD(const ActorMsgPack &mpk)
{
    const auto amCOR = mpk.conv<AMCORecord>();

    SMCORecord smCOR;
    std::memset(&smCOR, 0, sizeof(smCOR));

    smCOR.UID = amCOR.UID;
    smCOR.mapID = amCOR.mapID;
    smCOR.action = amCOR.action;

    switch(uidf::getUIDType(amCOR.UID)){
        case UID_PLY:
            {
                smCOR.Player.Level = amCOR.Player.Level;
                break;
            }
        case UID_MON:
            {
                smCOR.Monster.MonsterID = amCOR.Monster.MonsterID;
                break;
            }
        default:
            {
                break;
            }
    }
    postNetMessage(SM_CORECORD, smCOR);
}

void Player::on_AM_NOTIFYDEAD(const ActorMsgPack &)
{
}

void Player::on_AM_CHECKMASTER(const ActorMsgPack &rstMPK)
{
    m_slaveList.insert(rstMPK.from());
    m_actorPod->forward(rstMPK.from(), AM_OK, rstMPK.seqID());
}
