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
#include "actorpod.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"

extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

void Player::on_MPK_METRONOME(const MessagePack &)
{
    update();
}

void Player::on_MPK_BADACTORPOD(const MessagePack &rstMPK)
{
    AMBadActorPod amBAP;
    std::memcpy(&amBAP, rstMPK.Data(), sizeof(amBAP));
    reportDeadUID(amBAP.UID);
}

void Player::on_MPK_BINDCHANNEL(const MessagePack &rstMPK)
{
    AMBindChannel amBC;
    std::memcpy(&amBC, rstMPK.Data(), sizeof(amBC));

    // bind channel here
    // set the channel actor as this->GetAddress()
    m_channID = amBC.ChannID;

    g_netDriver->BindActor(ChannID(), UID());

    SMLoginOK smLOK;
    std::memset(&smLOK, 0, sizeof(smLOK));

    smLOK.UID       = UID();
    smLOK.DBID      = DBID();
    smLOK.MapID     = m_map->ID();
    smLOK.X         = X();
    smLOK.Y         = Y();
    smLOK.Male      = true;
    smLOK.Direction = Direction();
    smLOK.JobID     = JobID();
    smLOK.Level     = Level();

    g_netDriver->Post(ChannID(), SM_LOGINOK, smLOK);
    PullRectCO(10, 10);
}

void Player::on_MPK_NETPACKAGE(const MessagePack &rstMPK)
{
    AMNetPackage amNP;
    std::memcpy(&amNP, rstMPK.Data(), sizeof(AMNetPackage));

    uint8_t *pDataBuf = nullptr;
    if(amNP.DataLen){
        if(amNP.Data){
            pDataBuf = amNP.Data;
        }else{
            pDataBuf = amNP.DataBuf;
        }
    }

    operateNet(amNP.Type, pDataBuf, amNP.DataLen);

    if(amNP.Data){
        delete [] amNP.Data;
    }
}

void Player::on_MPK_ACTION(const MessagePack &rstMPK)
{
    AMAction amA;
    std::memcpy(&amA, rstMPK.Data(), sizeof(amA));

    if(amA.UID == UID()){
        return;
    }

    if(amA.MapID != MapID()){
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
                dispatchAction(amA.UID, ActionStand
                {
                    .x = X(),
                    .y = Y(),
                    .direction = Direction(),
                });
                break;
            }
        default:
            {
                break;
            }
    }

    AddInViewCO(amA.UID, amA.MapID, amA.action.x, amA.action.y, nDirection);
    reportAction(amA.UID, amA.action);
}

void Player::on_MPK_NOTIFYNEWCO(const MessagePack &mpk)
{
    const auto amNNCO = mpk.conv<AMNotifyNewCO>();
    if(m_dead.get()){
        notifyDead(amNNCO.UID);
    }
    else{
        // should make an valid action node and send it
        // currently just dispatch through map
        dispatchAction(amNNCO.UID, ActionStand
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
        });
    }
}

void Player::on_MPK_QUERYCORECORD(const MessagePack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.Data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Player::on_MPK_MAPSWITCH(const MessagePack &mpk)
{
    const auto amMS = mpk.conv<AMMapSwitch>();
    if(!(amMS.UID && amMS.MapID)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Map switch request failed: (UID = %llu, MapID = %llu)", to_llu(amMS.UID), to_llu(amMS.MapID));
    }
    requestMapSwitch(amMS.MapID, amMS.X, amMS.Y, false);
}

void Player::on_MPK_NPCQUERY(const MessagePack &mpk)
{
    const std::string queryName = mpk.conv<AMNPCQuery>().query;
    AMNPCEvent amNPCE;

    std::memset(&amNPCE, 0, sizeof(amNPCE));
    std::strcpy(amNPCE.event, queryName.c_str());

    if(queryName == "GOLD"){
        std::sprintf(amNPCE.value, "%d", m_gold);
    }

    else if(queryName == "LEVEL"){
        std::sprintf(amNPCE.value, "%d", m_level);
    }

    else if(queryName == "NAME"){
        std::sprintf(amNPCE.value, "%s", to_cstr(m_name));
    }

    else{
        std::strcpy(amNPCE.value, SYS_NPCERROR);
    }

    std::strcpy(amNPCE.event, SYS_NPCQUERY);
    m_actorPod->forward(mpk.from(), {MPK_NPCEVENT, amNPCE}, mpk.ID());
}

void Player::on_MPK_QUERYLOCATION(const MessagePack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.MapID     = MapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(rstMPK.from(), {MPK_LOCATION, amL}, rstMPK.ID());
}

void Player::on_MPK_ATTACK(const MessagePack &rstMPK)
{
    AMAttack amA;
    std::memcpy(&amA, rstMPK.Data(), sizeof(amA));

    if(mathf::LDistance2(X(), Y(), amA.X, amA.Y) > 2){
        switch(uidf::getUIDType(amA.UID)){
            case UID_MON:
            case UID_PLY:
                {
                    AMMiss amM;
                    std::memset(&amM, 0, sizeof(amM));

                    amM.UID = amA.UID;
                    m_actorPod->forward(amA.UID, {MPK_MISS, amM});
                    return;
                }
            default:
                {
                    return;
                }
        }
    }

    for(auto slaveUID: m_slaveList){
        m_actorPod->forward(slaveUID, MPK_MASTERHITTED);
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

void Player::on_MPK_UPDATEHP(const MessagePack &rstMPK)
{
    AMUpdateHP amUHP;
    std::memcpy(&amUHP, rstMPK.Data(), sizeof(amUHP));

    if(amUHP.UID != UID()){
        SMUpdateHP smUHP;
        smUHP.UID   = amUHP.UID;
        smUHP.MapID = amUHP.MapID;
        smUHP.HP    = amUHP.HP;
        smUHP.HPMax = amUHP.HPMax;

        g_netDriver->Post(ChannID(), SM_UPDATEHP, smUHP);
    }
}

void Player::on_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut amDFO;
    std::memcpy(&amDFO, rstMPK.Data(), sizeof(amDFO));

    RemoveInViewCO(amDFO.UID);
    if(amDFO.UID != UID()){
        SMDeadFadeOut smDFO;
        smDFO.UID   = amDFO.UID;
        smDFO.MapID = amDFO.MapID;
        smDFO.X     = amDFO.X;
        smDFO.Y     = amDFO.Y;
        g_netDriver->Post(ChannID(), SM_DEADFADEOUT, smDFO);
    }
}

void Player::on_MPK_EXP(const MessagePack &rstMPK)
{
    AMExp amE;
    std::memcpy(&amE, rstMPK.Data(), sizeof(amE));

    GainExp(amE.Exp);

    if(amE.Exp > 0){
        SMExp smE;
        smE.Exp = amE.Exp;
        g_netDriver->Post(ChannID(), SM_EXP, smE);
    }
}

void Player::on_MPK_MISS(const MessagePack &rstMPK)
{
    AMMiss amM;
    std::memcpy(&amM, rstMPK.Data(), sizeof(amM));

    SMMiss smM;
    std::memset(&smM, 0, sizeof(smM));

    smM.UID = amM.UID;
    postNetMessage(SM_MISS, smM);
}

void Player::on_MPK_SHOWDROPITEM(const MessagePack &rstMPK)
{
    AMShowDropItem amSDI;
    std::memcpy(&amSDI, rstMPK.Data(), sizeof(amSDI));

    SMShowDropItem smSDI;
    std::memset(&smSDI, 0, sizeof(smSDI));

    smSDI.X  = amSDI.X;
    smSDI.Y  = amSDI.Y;

    constexpr auto nSMIDListLen = std::extent<decltype(smSDI.IDList)>::value;
    constexpr auto nAMIDListLen = std::extent<decltype(amSDI.IDList)>::value;

    static_assert(nSMIDListLen >= nAMIDListLen, "");
    for(size_t nIndex = 0; nIndex < nAMIDListLen; ++nIndex){
        if(amSDI.IDList[nIndex].ID){
            smSDI.IDList[nIndex].ID   = amSDI.IDList[nIndex].ID;
            smSDI.IDList[nIndex].DBID = amSDI.IDList[nIndex].DBID;
        }else{
            break;
        }
    }
    g_netDriver->Post(ChannID(), SM_SHOWDROPITEM, smSDI);
}

void Player::on_MPK_NPCXMLLAYOUT(const MessagePack &msg)
{
    if(uidf::getUIDType(msg.from()) != UID_NPC){
        throw fflerror("actor message AMNPCXMLLayout from %s", uidf::getUIDTypeString(msg.from()));
    }

    const auto amNPCXMLL = msg.conv<AMNPCXMLLayout>();
    SMNPCXMLLayout smNPCXMLL;
    std::memset(&smNPCXMLL, 0, sizeof(smNPCXMLL));

    smNPCXMLL.NPCUID = msg.from();
    if(std::strlen(amNPCXMLL.xmlLayout) >= sizeof(smNPCXMLL.xmlLayout)){
        throw fflerror("actor message is too long: %zu", std::strlen(amNPCXMLL.xmlLayout));
    }
    std::strcpy(smNPCXMLL.xmlLayout, amNPCXMLL.xmlLayout);
    postNetMessage(SM_NPCXMLLAYOUT, smNPCXMLL);
}

void Player::on_MPK_BADCHANNEL(const MessagePack &rstMPK)
{
    AMBadChannel amBC;
    std::memcpy(&amBC, rstMPK.Data(), sizeof(amBC));

    condcheck(ChannID() == amBC.ChannID);
    g_netDriver->Shutdown(ChannID(), false);

    Offline();
}

void Player::on_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.Data(), sizeof(amO));

    reportOffline(amO.UID, amO.MapID);
}

void Player::on_MPK_REMOVEGROUNDITEM(const MessagePack &rstMPK)
{
    AMRemoveGroundItem amRGI;
    std::memcpy(&amRGI, rstMPK.Data(), sizeof(amRGI));

    SMRemoveGroundItem smRGI;
    smRGI.X    = amRGI.X;
    smRGI.Y    = amRGI.Y;
    smRGI.ID   = amRGI.ID;
    smRGI.DBID = amRGI.DBID;

    postNetMessage(SM_REMOVEGROUNDITEM, smRGI);
}

void Player::on_MPK_PICKUPOK(const MessagePack &rstMPK)
{
    AMPickUpOK amPUOK;
    std::memcpy(&amPUOK, rstMPK.Data(), sizeof(amPUOK));

    SMPickUpOK smPUOK;
    std::memset(&smPUOK, 0, sizeof(smPUOK));

    smPUOK.X    = amPUOK.X;
    smPUOK.Y    = amPUOK.Y;
    smPUOK.ID   = amPUOK.ID;
    smPUOK.DBID = amPUOK.DBID;

    postNetMessage(SM_PICKUPOK, smPUOK);

    switch(amPUOK.ID){
        case DBCOM_ITEMID(u8"金币"):
            {
                m_gold += std::rand() % 500;
                reportGold();
                break;
            }
        default:
            {
                m_inventory.emplace_back(amPUOK.ID, amPUOK.DBID);
                break;
            }
    }

}

void Player::on_MPK_CORECORD(const MessagePack &mpk)
{
    const auto amCOR = mpk.conv<AMCORecord>();

    SMCORecord smCOR;
    std::memset(&smCOR, 0, sizeof(smCOR));

    smCOR.UID = amCOR.UID;
    smCOR.MapID = amCOR.MapID;
    smCOR.action = amCOR.action;

    switch(uidf::getUIDType(amCOR.UID)){
        case UID_PLY:
            {
                smCOR.Player.DBID  = amCOR.Player.DBID;
                smCOR.Player.JobID = amCOR.Player.JobID;
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

void Player::on_MPK_NOTIFYDEAD(const MessagePack &)
{
}

void Player::on_MPK_CHECKMASTER(const MessagePack &rstMPK)
{
    m_slaveList.insert(rstMPK.from());
    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
}
