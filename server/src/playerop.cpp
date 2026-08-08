#include <cinttypes>
#include "totype.hpp"
#include "luaf.hpp"
#include "uidsf.hpp"
#include "mathf.hpp"
#include "pathf.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "friendtype.hpp"
#include "actorpod.hpp"
#include "server.hpp"
#include "cerealf.hpp"
#include "serdesmsg.hpp"
#include "buildconfig.hpp"
#include "directtradedb.hpp"

extern Server *g_server;

namespace
{
    bool sameDirectTradeItem(const SDItem &lhs, const SDItem &rhs)
    {
        return lhs.itemID == rhs.itemID
            && lhs.seqID == rhs.seqID
            && lhs.count == rhs.count
            && lhs.duration[0] == rhs.duration[0]
            && lhs.duration[1] == rhs.duration[1]
            && lhs.extAttrList == rhs.extAttrList;
    }

    bool sameDirectTradeOffer(const SDDirectTradeOffer &lhs, const SDDirectTradeOffer &rhs)
    {
        if(lhs.uid != rhs.uid
                || lhs.gold != rhs.gold
                || lhs.locked != rhs.locked
                || lhs.confirmed != rhs.confirmed
                || lhs.itemList.size() != rhs.itemList.size()){
            return false;
        }

        for(size_t i = 0; i < lhs.itemList.size(); ++i){
            if(!sameDirectTradeItem(lhs.itemList.at(i), rhs.itemList.at(i))){
                return false;
            }
        }
        return true;
    }
}

corof::awaitable<> Player::on_AM_BADACTORPOD(const ActorMsgPack &mpk)
{
    AMBadActorPod amBAP;
    std::memcpy(&amBAP, mpk.data(), sizeof(amBAP));
    reportDeadUID(amBAP.UID);
    return {};
}

corof::awaitable<> Player::on_AM_BINDCHANNEL(const ActorMsgPack &mpk)
{
    AMBindChannel amBC;
    std::memcpy(&amBC, mpk.data(), sizeof(amBC));

    // bind channel here
    // BINDCHANNEL message sent by servermap, not servicecore or g_netDirver

    fflassert(amBC.channID);
    fflassert(!m_channID.has_value());

    m_channID = amBC.channID;
    m_actorPod->bindNet(m_channID.value());

    postOnlineOK();
    return {};
}

corof::awaitable<> Player::on_AM_SENDPACKAGE(const ActorMsgPack &mpk)
{
    const auto sdSP = mpk.deserialize<SDSendPackage>();
    postNetMessage(sdSP.type, sdSP.buf.empty() ? nullptr : sdSP.buf.data(), sdSP.buf.size(), 0);
    return {};
}

corof::awaitable<> Player::on_AM_PLAYERSAY(const ActorMsgPack &mpk)
{
    const auto amPS = mpk.conv<AMPlayerSay>();

    SMPlayerSay smPS;
    std::memset(&smPS, 0, sizeof(smPS));

    smPS.uid = amPS.uid;
    std::memcpy(smPS.content, amPS.content, sizeof(smPS.content));
    postNetMessage(SM_PLAYERSAY, smPS);
    return {};
}

corof::awaitable<> Player::on_AM_PLAYERBROADCAST(const ActorMsgPack &mpk)
{
    const auto amPB = mpk.conv<AMPlayerBroadcast>();

    SMPlayerBroadcast smPB;
    std::memset(&smPB, 0, sizeof(smPB));

    smPB.uid = amPB.uid;
    std::memcpy(smPB.content, amPB.content, sizeof(smPB.content));
    postNetMessage(SM_PLAYERBROADCAST, smPB);
    return {};
}

corof::awaitable<> Player::on_AM_RECVPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amRP = mpk.conv<AMRecvPackage>();
    co_await operateNet(amRP.package.type, amRP.package.buf(), amRP.package.size, amRP.package.resp);
    freeActorDataPackage(&(amRP.package));
}

corof::awaitable<> Player::on_AM_ACTION(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAction>();
    if(amA.UID == UID()){
        co_return;
    }

    const auto [dstX, dstY] = [&amA]() -> std::tuple<int, int>
    {
        switch(amA.action.type){
            case ACTION_MOVE:
            case ACTION_SPACEMOVE:
                {
                    return {amA.action.aimX, amA.action.aimY};
                }
            default:
                {
                    return {amA.action.x, amA.action.y};
                }
        }
    }();

    const auto distChanged = [dstX, dstY, amA, this]() -> bool
    {
        if(amA.mapUID != mapUID()){
            return true;
        }

        if(const auto coLocPtr = getInViewCOPtr(amA.UID)){
            return mathf::LDistance2<int>(X(), Y(), coLocPtr->x, coLocPtr->y) != mathf::LDistance2<int>(X(), Y(), dstX, dstY);
        }
        return true;
    }();

    const auto addedInView = updateInViewCO(COLocation
    {
        .uid    = amA.UID,
        .mapUID = amA.mapUID,

        .x = dstX,
        .y = dstY,
        .direction = amA.action.direction,
    });

    if(distChanged){
        co_await m_buffList.updateAura(amA.UID);
    }

    if(addedInView > 0){
        dispatchAction(amA.UID, makeActionStand());
    }

    // always need to notify client for CO gets added/moved/removed
    reportAction(amA.UID, amA.mapUID, amA.action);

    if(m_teamLeader == amA.UID){
        co_await followTeamLeader();
    }
}

corof::awaitable<> Player::on_AM_NOTIFYNEWCO(const ActorMsgPack &mpk)
{
    const auto amNNCO = mpk.conv<AMNotifyNewCO>();
    if(m_sdHealth.dead()){
        notifyDead(amNNCO.UID);
    }
    else{
        // should make an valid action node and send it
        // currently just dispatch through map
        dispatchAction(amNNCO.UID, makeActionStand());
    }
    return {};
}

corof::awaitable<> Player::on_AM_QUERYHEALTH(const ActorMsgPack &rmpk)
{
    m_actorPod->post(rmpk.fromAddr(), {AM_HEALTH, cerealf::serialize(m_sdHealth)});
    return {};
}

corof::awaitable<> Player::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
    return {};
}

corof::awaitable<> Player::on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &mpk)
{
    const auto amMST = mpk.conv<AMMapSwitchTrigger>();
    if(!uidf::isMap(amMST.mapUID)){
        g_server->addLog(LOGTYPE_WARNING, "Map switch request failed: mapUID %llu", to_llu(amMST.mapUID));
    }

    if(amMST.mapUID == mapUID()){
        co_await requestSpaceMove(amMST.X, amMST.Y, false);
    }
    else{
        co_await requestMapSwitch(amMST.mapUID, amMST.X, amMST.Y, false);
    }
}

corof::awaitable<> Player::on_AM_QUERYLOCATION(const ActorMsgPack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.mapUID    = mapUID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->post(rstMPK.fromAddr(), {AM_LOCATION, amL});
    return {};
}

corof::awaitable<> Player::on_AM_ATTACK(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID == UID()){
        return {};
    }

    if(m_sdHealth.dead()){
        notifyDead(amA.UID);
        return {};
    }

    if(const auto &mr = DBCOM_MAGICRECORD(amA.damage.magicID); !pathf::inDCCastRange(mr.castRange, X(), Y(), amA.X, amA.Y)){
        switch(uidf::getUIDType(amA.UID)){
            case UID_MON:
            case UID_PLY:
                {
                    AMMiss amM;
                    std::memset(&amM, 0, sizeof(amM));

                    amM.UID = amA.UID;
                    m_actorPod->post(amA.UID, {AM_MISS, amM});
                    return {};
                }
            default:
                {
                    return {};
                }
        }
    }

    for(auto slaveUID: m_slaveList){
        m_actorPod->post(slaveUID, AM_MASTERHITTED);
    }

    const ActionHitted hitted
    {
        .direction = Direction(),
        .x = X(),
        .y = Y(),
        .fromUID = amA.UID,
    };

    dispatchAction(hitted);
    reportAction(UID(), mapUID(), hitted);

    struckDamage(amA.UID, amA.damage);
    return {};
}

corof::awaitable<> Player::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    m_inViewCOList.erase(amDFO.UID);

    if(amDFO.UID != UID()){
        SMDeadFadeOut smDFO;
        std::memset(&smDFO, 0, sizeof(smDFO));

        smDFO.UID    = amDFO.UID;
        smDFO.mapUID = amDFO.mapUID;
        smDFO.X      = amDFO.X;
        smDFO.Y      = amDFO.Y;

        postNetMessage(SM_DEADFADEOUT, smDFO);
    }
    return {};
}

corof::awaitable<> Player::on_AM_ADDBUFF(const ActorMsgPack &mpk)
{
    const auto amAB = mpk.conv<AMAddBuff>();
    fflassert(amAB.id);

    const auto &br = DBCOM_BUFFRECORD(amAB.id);
    fflassert(br);

    switch(const auto friendType = co_await checkFriend(amAB.fromUID); friendType){
        case FT_FRIEND:
            {
                if(br.favor >= 0){
                    addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                }
                break;
            }
        case FT_ENEMY:
            {
                if(br.favor <= 0){
                    addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                }
                break;
            }
        case FT_NEUTRAL:
            {
                addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                break;
            }
        default:
            {
                break;
            }
    }
}

corof::awaitable<> Player::on_AM_REMOVEBUFF(const ActorMsgPack &mpk)
{
    const auto amRB = mpk.conv<AMRemoveBuff>();
    removeFromBuff(amRB.fromUID, amRB.fromBuffSeq, true);
    return {};
}

corof::awaitable<> Player::on_AM_EXP(const ActorMsgPack &mpk)
{
    const auto amE = mpk.conv<AMExp>();
    if(!m_slaveList.contains(mpk.from()) && uidf::isMonster(mpk.from())){
        m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_KILL, %llu)", to_llu(uidf::getMonsterID(mpk.from()))));
    }

    gainExp(amE.exp);
    return {};
}

corof::awaitable<> Player::on_AM_MISS(const ActorMsgPack &mpk)
{
    const auto amM = mpk.conv<AMMiss>();
    if(amM.UID != UID()){
        return {};
    }

    SMMiss smM;
    std::memset(&smM, 0, sizeof(smM));

    smM.UID = amM.UID;
    dispatchNetPackage(true, SM_MISS, smM);
    return {};
}

corof::awaitable<> Player::on_AM_HEAL(const ActorMsgPack &mpk)
{
    const auto amH = mpk.conv<AMHeal>();
    if(amH.mapUID == mapUID()){
        updateHealth(amH.addHP, amH.addMP);
    }
    return {};
}

corof::awaitable<> Player::on_AM_BADCHANNEL(const ActorMsgPack &mpk)
{
    const auto amBC = mpk.conv<AMBadChannel>();
    fflassert(m_channID.has_value());
    fflassert(m_channID.value() == amBC.channID, m_channID.value(), amBC.channID);

    // AM_BADCHANNEL is sent by Channel::dtor
    // when player get this message the channel has already been destructed

    // assign zero to m_channID to indicate player has received AM_BADCHANNEL instead of not bind to a channel yet
    // code after this line should know it shall not post any network message, because the channel slot has been released

    goOffline();
    return {};
}

corof::awaitable<> Player::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    reportOffline(amO.UID, amO.mapUID);
    return {};
}

corof::awaitable<> Player::on_AM_QUERYUIDBUFF(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_BUFFIDLIST, cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    }));
    return {};
}

corof::awaitable<> Player::on_AM_QUERYPLAYERNAME(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_PLAYERNAME, cerealf::serialize(SDPlayerName
    {
        .uid = UID(),
        .name = name(),
        .nameColor = nameColor(),
    }));
    return {};
}

corof::awaitable<> Player::on_AM_QUERYPLAYERWLDESP(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_PLAYERWLDESP, cerealf::serialize(SDUIDWLDesp
    {
        .uid = UID(),
        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },
    }, true));
    return {};
}

corof::awaitable<> Player::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &mpk)
{
    const auto amQFT = mpk.conv<AMQueryFriendType>();
    const auto friendType = co_await checkFriend(amQFT.UID);

    AMFriendType amFT;
    std::memset(&amFT, 0, sizeof(amFT));

    amFT.Type = friendType;
    m_actorPod->post(mpk.fromAddr(), {AM_FRIENDTYPE, amFT});
}

corof::awaitable<> Player::on_AM_REMOVEGROUNDITEM(const ActorMsgPack &rstMPK)
{
    AMRemoveGroundItem amRGI;
    std::memcpy(&amRGI, rstMPK.data(), sizeof(amRGI));

    SMRemoveGroundItem smRGI;
    smRGI.X    = amRGI.X;
    smRGI.Y    = amRGI.Y;
    smRGI.ID   = amRGI.ID;
    smRGI.DBID = amRGI.DBID;

    postNetMessage(SM_REMOVEGROUNDITEM, smRGI);
    return {};
}

corof::awaitable<> Player::on_AM_CORECORD(const ActorMsgPack &mpk)
{
    const auto amCOR = mpk.conv<AMCORecord>();

    SMCORecord smCOR;
    std::memset(&smCOR, 0, sizeof(smCOR));

    smCOR.UID = amCOR.UID;
    smCOR.mapUID = amCOR.mapUID;
    smCOR.action = amCOR.action;

    switch(uidf::getUIDType(amCOR.UID)){
        case UID_PLY:
            {
                smCOR.Player.gender = amCOR.Player.gender;
                smCOR.Player.job = amCOR.Player.job;
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
    return {};
}

corof::awaitable<> Player::on_AM_NOTIFYDEAD(const ActorMsgPack &)
{
    return {};
}

corof::awaitable<> Player::on_AM_CHECKMASTER(const ActorMsgPack &rstMPK)
{
    const auto combNode = getCombatNode(m_sdItemStorage.wear, m_sdLearnedMagicList, UID(), level());

    AMCheckMasterOK amCMOK;
    std::memset(&amCMOK, 0, sizeof(amCMOK));

    amCMOK.dc[0] = combNode.dc[0];
    amCMOK.dc[1] = combNode.dc[1];

    amCMOK.mc[0] = combNode.mc[0];
    amCMOK.mc[1] = combNode.mc[1];

    amCMOK.sc[0] = combNode.sc[0];
    amCMOK.sc[1] = combNode.sc[1];

    amCMOK.ac[0] = combNode.ac[0];
    amCMOK.ac[1] = combNode.ac[1];

    amCMOK.mac[0] = combNode.mac[0];
    amCMOK.mac[1] = combNode.mac[1];

    m_slaveList.insert(rstMPK.from());
    m_actorPod->post(rstMPK.fromAddr(), {AM_CHECKMASTEROK, amCMOK});
    return {};
}

corof::awaitable<> Player::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    auto sdRC = mpk.deserialize<SDRemoteCall>();
    m_luaRunner->spawn(m_threadKey++, mpk.fromAddr(), std::move(sdRC.code), std::move(sdRC.args));
    return {};
}

corof::awaitable<> Player::on_AM_REQUESTJOINTEAM(const ActorMsgPack &mpk)
{
    const auto sdRJT = mpk.deserialize<SDRequestJoinTeam>();
    postNetMessage(SM_TEAMCANDIDATE, cerealf::serialize(SDTeamCandidate
    {
        .player = sdRJT.player, // can forward player's friend to the team leader
    }));
    return {};
}

// Cross-actor direct-trade protocol.
//
// Each Player actor stores its own canonical offer plus the latest canonical
// peer offer. Offer updates are relayed immediately for live client display.
// After both offers are locked, each player's CM_COMMITDIRECTTRADE marks only
// that player's confirmation. The lower-UID actor is the sole coordinator:
// it snapshots both confirmed offers, asks the peer actor to validate the exact
// same snapshot, then runs one atomic database exchange.
//
// Cancellation wins if it is processed before the peer accepts that snapshot.
// Once an actor sets m_directTradeCommitPending, its local client can no longer
// cancel; this prevents a completed database transaction from being undone by
// a late close message.
corof::awaitable<> Player::on_AM_REQUESTDIRECTTRADE(const ActorMsgPack &mpk)
{
    const auto reject = [this, &mpk](int error)
    {
        m_actorPod->post(mpk.fromAddr(), {AM_REJECTDIRECTTRADE, AMDirectTradeError{.error = to_u8(error)}});
    };

    if(!uidf::isPlayer(mpk.from()) || !getInViewCOPtr(mpk.from())){
        reject(DTRADEERR_TOOFAR);
        return {};
    }

    if(!SDRuntimeConfig_getConfig<RTCFG_允许交易>(m_sdPlayerConfig.runtimeConfig)){
        reject(DTRADEERR_TARGETNOTALLOWED);
        return {};
    }

    if(m_sdHealth.dead()){
        reject(DTRADEERR_TARGETDEAD);
        return {};
    }

    if(directTradeBusy()){
        reject(DTRADEERR_TARGETBUSY);
        return {};
    }

    m_directTradeRequesterUID = mpk.from();

    const auto amDTP = mpk.conv<AMDirectTradePeer>();
    SMDirectTradeRequest smDTR
    {
        .uid = mpk.from(),
    };
    smDTR.name.assign(amDTP.name.as_sv());
    postNetMessage(SM_DIRECTTRADEREQUEST, smDTR);
    return {};
}

corof::awaitable<> Player::on_AM_ACCEPTDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradeRequestTargetUID != mpk.from() || m_directTradePeerUID || m_directTradeRequesterUID){
        m_actorPod->post(mpk.fromAddr(), AM_CANCELDIRECTTRADE);
        return {};
    }

    if(m_sdHealth.dead() || !getInViewCOPtr(mpk.from())){
        m_directTradeRequestTargetUID = 0;
        m_actorPod->post(mpk.fromAddr(), AM_CANCELDIRECTTRADE);

        SMDirectTradeError smDTE;
        std::memset(&smDTE, 0, sizeof(smDTE));

        smDTE.error = to_u8(m_sdHealth.dead() ? DTRADEERR_DEAD : DTRADEERR_TOOFAR);
        postNetMessage(SM_DIRECTTRADEERROR, smDTE);
        return {};
    }

    m_directTradePeerUID = mpk.from();
    m_directTradeRequestTargetUID = 0;
    m_directTradeOffer.clear();
    m_directTradeOffer.uid = UID();
    m_directTradePeerOffer.clear();
    m_directTradeCommitPending = false;

    const auto amDTP = mpk.conv<AMDirectTradePeer>();
    AMDirectTradePeer amStart;
    amStart.name.assign(name());
    m_actorPod->post(mpk.fromAddr(), {AM_STARTDIRECTTRADE, amStart});

    SMStartDirectTrade smSDT
    {
        .uid = mpk.from(),
    };
    smSDT.name.assign(amDTP.name.as_sv());
    postNetMessage(SM_STARTDIRECTTRADE, smSDT);
    return {};
}

corof::awaitable<> Player::on_AM_REJECTDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradeRequestTargetUID == mpk.from()){
        m_directTradeRequestTargetUID = 0;

        SMDirectTradeError smDTE;
        std::memset(&smDTE, 0, sizeof(smDTE));

        smDTE.error = to_u8(mpk.conv<AMDirectTradeError>().error);
        postNetMessage(SM_DIRECTTRADEERROR, smDTE);
    }
    return {};
}

corof::awaitable<> Player::on_AM_STARTDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradeRequesterUID != mpk.from() || m_directTradePeerUID || m_directTradeRequestTargetUID){
        m_actorPod->post(mpk.fromAddr(), AM_CANCELDIRECTTRADE);
        return {};
    }

    m_directTradePeerUID = mpk.from();
    m_directTradeRequesterUID = 0;
    m_directTradeOffer.clear();
    m_directTradeOffer.uid = UID();
    m_directTradePeerOffer.clear();
    m_directTradeCommitPending = false;

    const auto amDTP = mpk.conv<AMDirectTradePeer>();
    SMStartDirectTrade smSDT
    {
        .uid = mpk.from(),
    };
    smSDT.name.assign(amDTP.name.as_sv());
    postNetMessage(SM_STARTDIRECTTRADE, smSDT);
    return {};
}

corof::awaitable<> Player::on_AM_UPDATEDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradePeerUID != mpk.from()){
        co_return;
    }

    if(m_directTradeCommitPending){
        co_return;
    }

    if(m_sdHealth.dead() || !getInViewCOPtr(mpk.from())){
        cancelDirectTrade();
        co_return;
    }

    const auto offer = mpk.deserialize<SDDirectTradeOffer>();
    if(offer.uid != mpk.from()
            || offer.locked > 1
            || offer.confirmed > 1
            || (to_bool(offer.confirmed) && !to_bool(offer.locked))
            || offer.itemList.size() > SYS_DIRECTTRADEMAXITEM){
        cancelDirectTrade();
        co_return;
    }

    if(to_bool(m_directTradePeerOffer.confirmed)
            && !sameDirectTradeOffer(offer, m_directTradePeerOffer)){
        cancelDirectTrade();
        co_return;
    }

    m_directTradePeerOffer = offer;
    postNetMessage(SM_UPDATEDIRECTTRADE, cerealf::serialize(offer));

    if(directTradeConfirmed() && !m_directTradeCommitPending){
        if(UID() < m_directTradePeerUID){
            co_await commitDirectTrade(mpk.from());
        }
        else{
            m_actorPod->post(m_directTradePeerUID, AM_COMMITDIRECTTRADE);
        }
    }
    co_return;
}

corof::awaitable<> Player::on_AM_COMMITDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradePeerUID == mpk.from()
            && UID() < mpk.from()
            && !m_directTradeCommitPending){
        co_await commitDirectTrade(mpk.from());
    }
    co_return;
}

corof::awaitable<> Player::on_AM_PREPAREDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradePeerUID != mpk.from()
            || m_directTradeCommitPending
            || !directTradeConfirmed()
            || m_sdHealth.dead()
            || !getInViewCOPtr(mpk.from())){
        m_actorPod->post(mpk.fromAddr(), AM_ERROR);
        return {};
    }

    const auto commit = mpk.deserialize<SDDirectTradeCommit>();
    const auto ownOffer = commit.findDirectTradeOffer(UID());
    const auto peerOffer = commit.findDirectTradeOffer(m_directTradePeerUID);
    if(!ownOffer
            || !peerOffer
            || !sameDirectTradeOffer(*ownOffer, m_directTradeOffer)
            || !sameDirectTradeOffer(*peerOffer, m_directTradePeerOffer)){
        m_actorPod->post(mpk.fromAddr(), AM_ERROR);
        return {};
    }

    // The peer has verified both offers byte-for-byte and now freezes them.
    m_directTradeCommitPending = true;
    m_actorPod->post(mpk.fromAddr(), AM_OK);
    return {};
}

corof::awaitable<> Player::on_AM_COMPLETEDIRECTTRADE(const ActorMsgPack &mpk)
{
    if(m_directTradePeerUID != mpk.from() || !m_directTradeCommitPending){
        return {};
    }

    auto result = mpk.deserialize<SDDirectTradeResult>();
    if(result.uid != UID()){
        cancelDirectTrade();
        return {};
    }

    applyDirectTradeResult(std::move(result));
    return {};
}

corof::awaitable<> Player::on_AM_DIRECTTRADEERROR(const ActorMsgPack &mpk)
{
    if(m_directTradePeerUID == mpk.from()){
        SMDirectTradeError smDTE;
        std::memset(&smDTE, 0, sizeof(smDTE));

        smDTE.error = to_u8(mpk.conv<AMDirectTradeError>().error);
        postNetMessage(SM_DIRECTTRADEERROR, smDTE);
    }
    return {};
}

corof::awaitable<> Player::on_AM_CANCELDIRECTTRADE(const ActorMsgPack &mpk)
{
    // Deliberately accept peer cancellation even if this actor is pending:
    // the coordinator sets pending before the peer validates the snapshot. A
    // cancel emitted by the peer before that validation must still win. After
    // validation the peer ignores local close, so it cannot emit a new cancel.
    if(m_directTradePeerUID == mpk.from()){
        m_directTradePeerUID = 0;
        m_directTradeOffer.clear();
        m_directTradePeerOffer.clear();
        m_directTradeCommitPending = false;
        postNetMessage(SM_CLOSEDIRECTTRADE, SMCloseDirectTrade
        {
            .uid = mpk.from(),
        });
    }

    if(m_directTradeRequestTargetUID == mpk.from()){
        m_directTradeRequestTargetUID = 0;
    }

    if(m_directTradeRequesterUID == mpk.from()){
        m_directTradeRequesterUID = 0;
    }
    return {};
}

corof::awaitable<> Player::on_AM_REQUESTLEAVETEAM(const ActorMsgPack &mpk)
{
    if(m_teamLeader == UID()){
        for(const auto member: m_teamMemberList){
            if(member != UID()){
                m_actorPod->post(member, AM_TEAMUPDATE);
            }
        }

        m_teamMemberList.erase(std::remove(m_teamMemberList.begin(), m_teamMemberList.end(), mpk.from()), m_teamMemberList.end());
        co_await reportTeamMemberList();
    }
}

corof::awaitable<> Player::on_AM_QUERYTEAMPLAYER(const ActorMsgPack &mpk)
{
    m_actorPod->post(mpk.fromAddr(), {AM_TEAMPLAYER, cerealf::serialize(SDTeamPlayer
    {
        .uid = UID(),
        .level = level(),
        .name = name(),
    })});
    return {};
}

corof::awaitable<> Player::on_AM_TEAMUPDATE(const ActorMsgPack &mpk)
{
    switch(const auto rmpk = co_await m_actorPod->send(mpk.from(), AM_QUERYTEAMMEMBERLIST); rmpk.type()){
        case AM_TEAMMEMBERLIST:
            {
                const auto sdTML = rmpk.deserialize<SDTeamMemberList>();
                if(sdTML.hasMember(UID())){
                    fflassert(sdTML.teamLeader == rmpk.from());
                }

                if(sdTML.hasMember(UID())){
                    m_teamLeader = rmpk.from();
                    m_teamMemberList.clear();
                    postNetMessage(SM_TEAMMEMBERLIST, cerealf::serialize(sdTML));
                }
                else{
                    m_teamLeader = 0;
                    m_teamMemberList.clear();
                    postNetMessage(SM_TEAMMEMBERLIST, cerealf::serialize(SDTeamMemberList{}));
                }

                break;
            }
        default:
            {
                m_teamLeader = 0;
                m_teamMemberList.clear();
                postNetMessage(SM_TEAMMEMBERLIST, cerealf::serialize(SDTeamMemberList{}));
                break;
            }
    }
}

corof::awaitable<> Player::on_AM_QUERYTEAMMEMBERLIST(const ActorMsgPack &mpk)
{
    const auto sdTML = co_await pullTeamMemberList();
    m_actorPod->post(mpk.fromAddr(), {AM_TEAMMEMBERLIST, cerealf::serialize(sdTML.value())});
}
