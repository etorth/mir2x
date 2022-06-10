#include <cinttypes>
#include "totype.hpp"
#include "luaf.hpp"
#include "mathf.hpp"
#include "pathf.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "friendtype.hpp"
#include "actorpod.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"
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
    // BINDCHANNEL message sent by servermap, not servicecore or g_netDirver

    fflassert(amBC.channID);
    fflassert(!m_channID.has_value());

    m_channID = amBC.channID;
    g_netDriver->bindPlayer(m_channID.value(), UID());

    postOnlineOK();
}

void Player::on_AM_SENDPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amSP = mpk.conv<AMSendPackage>();
    postNetMessage(amSP.package.type, amSP.package.buf(), amSP.package.size);
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
    const auto amA = rstMPK.conv<AMAction>();
    if(amA.UID == UID()){
        return;
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
        if(amA.mapID != mapID()){
            return true;
        }

        if(const auto coLocPtr = getInViewCOPtr(amA.UID)){
            return mathf::LDistance2<int>(X(), Y(), coLocPtr->x, coLocPtr->y) != mathf::LDistance2<int>(X(), Y(), dstX, dstY);
        }
        return true;
    }();

    const auto addedInView = updateInViewCO(COLocation
    {
        .uid = amA.UID,
        .mapID = amA.mapID,

        .x = dstX,
        .y = dstY,
        .direction = amA.action.direction,
    });

    if(distChanged){
        m_buffList.updateAura(amA.UID);
    }

    if(addedInView > 0){
        dispatchAction(amA.UID, makeActionStand());
    }

    // always need to notify client for CO gets added/moved/removed
    reportAction(amA.UID, amA.mapID, amA.action);
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

void Player::on_AM_QUERYHEALTH(const ActorMsgPack &rmpk)
{
    m_actorPod->forward(rmpk.from(), {AM_HEALTH, cerealf::serialize(m_sdHealth)}, rmpk.seqID());
}

void Player::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Player::on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &mpk)
{
    const auto amMST = mpk.conv<AMMapSwitchTrigger>();
    if(!(amMST.UID && amMST.mapID)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Map switch request failed: (UID = %llu, mapID = %llu)", to_llu(amMST.UID), to_llu(amMST.mapID));
    }

    if(amMST.mapID == mapID()){
        requestSpaceMove(amMST.X, amMST.Y, false);
    }
    else{
        requestMapSwitch(amMST.mapID, amMST.X, amMST.Y, false);
    }
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

void Player::on_AM_ATTACK(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID != UID()){
        return;
    }

    if(m_dead.get()){
        notifyDead(amA.UID);
        return;
    }

    if(const auto &mr = DBCOM_MAGICRECORD(amA.damage.magicID); !pathf::inDCCastRange(mr.castRange, X(), Y(), amA.X, amA.Y)){
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
        .fromUID = amA.UID,
    });
    struckDamage(amA.UID, amA.damage);
    reportAction(UID(), mapID(), ActionHitted
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
        .fromUID = amA.UID,
    });
}

void Player::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    m_inViewCOList.erase(amDFO.UID);

    if(amDFO.UID != UID()){
        SMDeadFadeOut smDFO;
        std::memset(&smDFO, 0, sizeof(smDFO));

        smDFO.UID   = amDFO.UID;
        smDFO.mapID = amDFO.mapID;
        smDFO.X     = amDFO.X;
        smDFO.Y     = amDFO.Y;

        postNetMessage(SM_DEADFADEOUT, smDFO);
    }
}

void Player::on_AM_ADDBUFF(const ActorMsgPack &mpk)
{
    const auto amAB = mpk.conv<AMAddBuff>();
    fflassert(amAB.id);
    fflassert(DBCOM_BUFFRECORD(amAB.id));

    checkFriend(amAB.fromUID, [amAB, this](int friendType)
    {
        const auto &br = DBCOM_BUFFRECORD(amAB.id);
        fflassert(br);

        switch(friendType){
            case FT_FRIEND:
                {
                    if(br.favor >= 0){
                        addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    }
                    return;
                }
            case FT_ENEMY:
                {
                    if(br.favor <= 0){
                        addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    }
                    return;
                }
            case FT_NEUTRAL:
                {
                    addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

void Player::on_AM_REMOVEBUFF(const ActorMsgPack &mpk)
{
    const auto amRB = mpk.conv<AMRemoveBuff>();
    removeFromBuff(amRB.fromUID, amRB.fromBuffSeq, true);
}

void Player::on_AM_EXP(const ActorMsgPack &mpk)
{
    const auto amE = mpk.conv<AMExp>();
    gainExp(amE.exp);
}

void Player::on_AM_MISS(const ActorMsgPack &mpk)
{
    const auto amM = mpk.conv<AMMiss>();
    if(amM.UID != UID()){
        return;
    }

    SMMiss smM;
    std::memset(&smM, 0, sizeof(smM));

    smM.UID = amM.UID;
    dispatchNetPackage(true, SM_MISS, smM);
}

void Player::on_AM_HEAL(const ActorMsgPack &mpk)
{
    const auto amH = mpk.conv<AMHeal>();
    if(amH.mapID == mapID()){
        updateHealth(amH.addHP, amH.addMP);
    }
}

void Player::on_AM_BADCHANNEL(const ActorMsgPack &mpk)
{
    const auto amBC = mpk.conv<AMBadChannel>();
    fflassert(m_channID.has_value());
    fflassert(m_channID.value() == amBC.channID, m_channID.value(), amBC.channID);

    // AM_BADCHANNEL is sent by Channel::dtor
    // when player get this message the channel has already been destructed
    // assign zero to m_channID to indicate player has received AM_BADCHANNEL instead of not bind to a channel yet
    // code after this line should know it shall not post any network message, because the channel slot has been released

    m_channID = 0;
    goOffline();
}

void Player::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    reportOffline(amO.UID, amO.mapID);
}

void Player::on_AM_QUERYUIDBUFF(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_BUFFIDLIST, cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    }));
}

void Player::on_AM_QUERYPLAYERWLDESP(const ActorMsgPack &mpk)
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
}

void Player::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &mpk)
{
    const auto amQFT = mpk.conv<AMQueryFriendType>();
    checkFriend(amQFT.UID, [this, mpk](int friendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = friendType;
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
    m_actorPod->forward(rstMPK.from(), {AM_CHECKMASTEROK, amCMOK}, rstMPK.seqID());
}

void Player::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    const auto fnSendSerVarList = [from = mpk.from(), seqID = mpk.seqID(), this](std::vector<std::string> error, std::vector<std::string> serVarList)
    {
        if(!error.empty()){
            fflassert(serVarList.empty(), error, serVarList.size());
        }

        m_actorPod->forward(from, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
        {
            .error = std::move(error),
            .serVarList = std::move(serVarList),
        })}, seqID);
    };

    const auto sdRC = mpk.deserialize<SDRemoteCall>();
    const auto runSeqID = m_runSeqID++;

    PlayerLuaCORunner runner
    {
        m_luaModulePtr->getLuaState(),
        mpk.from(),
        mpk.seqID(),
    };

    const auto pfr = runner.callback(str_printf(
        R"###( getTLSTable().runSeqID = %llu )###""\n"
        R"###( do                            )###""\n"
        R"###(    %s                         )###""\n"
        R"###( end                           )###""\n", to_llu(runSeqID), to_cstr(sdRC.code)));

    std::vector<std::string> error;
    const auto fnDrainError = [&error](const std::string &s)
    {
        error.push_back(s);
    };

    if(m_luaModulePtr->pfrCheck(pfr, fnDrainError)){
        if(runner.callback){
            // initial run succeeds but coroutine is not finished yet
            // need to save for later trigger
            m_runnerList.insert_or_assign(runSeqID, std::move(runner));
        }
        else{
            // initial run succeeds and coroutine finished
            // simple cases like: uidExecute(uid, [[ return getName() ]])
            //
            // for this case there is no need to uses coroutine
            // but we can not predict script from NPC is synchronized call or not
            //
            // for cases like spaceMove() we can use quasi-function from NPC side
            // but it prevents NPC side to execute commands like:
            //
            //     uidExecute(uid, [[
            //         spaceMove(12, 22, 12) -- impossible if using quasi-function
            //         return getLevel()
            //     ]])
            //
            // for the quasi-function solution player side has no spaceMove() function avaiable
            // player side can only support like:
            //
            //     'SPACEMOVE 12, 22, 12'
            //
            // this is because spaveMove is a async operation, it needs callbacks
            // this limits the script for NPC side, put everything into coroutine is a solution for it, but with cost
            //
            // for how quasi-function was implemented
            // check commit: 30981cc539a05b41309330eaa04fbf3042c9d826
            //
            // starting/running an coroutine with a light function in Lua costs ~280 ns
            // alternatively call a light function directly takes ~30 ns, best try is every time let uidExecute() do as much as possible
            //
            // light cases, no yield in script
            // directly return the result with save the runner
            fnSendSerVarList(std::move(error), luaf::pfrBuildBlobList(pfr));
        }
    }
    else{
        // put error message if error happened but there is no explicit error message
        // if error and serVarList are both empty, caller side takes it as a successfully call without explicit results
        if(error.empty()){
            error.push_back("unknown error");
        }
        fnSendSerVarList(std::move(error), {});
    }
}
