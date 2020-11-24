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
    AMBadActorPod stAMBAP;
    std::memcpy(&stAMBAP, rstMPK.Data(), sizeof(stAMBAP));
    reportDeadUID(stAMBAP.UID);
}

void Player::on_MPK_BINDCHANNEL(const MessagePack &rstMPK)
{
    AMBindChannel stAMBC;
    std::memcpy(&stAMBC, rstMPK.Data(), sizeof(stAMBC));

    // bind channel here
    // set the channel actor as this->GetAddress()
    m_channID = stAMBC.ChannID;

    g_netDriver->BindActor(ChannID(), UID());

    SMLoginOK stSMLOK;
    std::memset(&stSMLOK, 0, sizeof(stSMLOK));

    stSMLOK.UID       = UID();
    stSMLOK.DBID      = DBID();
    stSMLOK.MapID     = m_map->ID();
    stSMLOK.X         = X();
    stSMLOK.Y         = Y();
    stSMLOK.Male      = true;
    stSMLOK.Direction = Direction();
    stSMLOK.JobID     = JobID();
    stSMLOK.Level     = Level();

    g_netDriver->Post(ChannID(), SM_LOGINOK, stSMLOK);
    PullRectCO(10, 10);
}

void Player::on_MPK_NETPACKAGE(const MessagePack &rstMPK)
{
    AMNetPackage stAMNP;
    std::memcpy(&stAMNP, rstMPK.Data(), sizeof(AMNetPackage));

    uint8_t *pDataBuf = nullptr;
    if(stAMNP.DataLen){
        if(stAMNP.Data){
            pDataBuf = stAMNP.Data;
        }else{
            pDataBuf = stAMNP.DataBuf;
        }
    }

    operateNet(stAMNP.Type, pDataBuf, stAMNP.DataLen);

    if(stAMNP.Data){
        delete [] stAMNP.Data;
    }
}

void Player::on_MPK_ACTION(const MessagePack &rstMPK)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID == UID()){
        return;
    }

    if(stAMA.MapID != MapID()){
        RemoveInViewCO(stAMA.UID);
        return;
    }

    // for all action types
    // the x/y are always well-defined

    int nDirection = -1;
    switch(stAMA.Action){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                nDirection = stAMA.Direction;
                break;
            }
        default:
            {
                break;
            }
    }

    if(mathf::LDistance2(stAMA.X, stAMA.Y, X(), Y()) > 20 * 20){
        return;
    }

    switch(stAMA.Action){
        case ACTION_SPAWN:
        case ACTION_SPACEMOVE2:
            {
                DispatchAction(stAMA.UID, ActionStand(X(), Y(), Direction()));
                break;
            }
        default:
            {
                break;
            }
    }

    AddInViewCO(stAMA.UID, stAMA.MapID, stAMA.X, stAMA.Y, nDirection);
    ReportAction(stAMA.UID, ActionNode
    {
        stAMA.Action,
        stAMA.Speed,
        stAMA.Direction,

        stAMA.X,
        stAMA.Y,
        stAMA.AimX,
        stAMA.AimY,
        stAMA.AimUID,
        stAMA.ActionParam,
    });
}

void Player::on_MPK_NOTIFYNEWCO(const MessagePack &rstMPK)
{
    const auto stAMNNCO = rstMPK.conv<AMNotifyNewCO>();
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                // should make an valid action node and send it
                // currently just dispatch through map

                DispatchAction(ActionStand(X(), Y(), Direction()));
                break;
            }
        default:
            {
                AMNotifyDead stAMND;
                std::memset(&stAMND, 0, sizeof(stAMND));

                stAMND.UID = UID();
                m_actorPod->forward(stAMNNCO.UID, {MPK_NOTIFYDEAD, stAMND});
                break;
            }
    }
}

void Player::on_MPK_QUERYCORECORD(const MessagePack &rstMPK)
{
    AMQueryCORecord stAMQCOR;
    std::memcpy(&stAMQCOR, rstMPK.Data(), sizeof(stAMQCOR));

    reportCO(stAMQCOR.UID);
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
    AMLocation stAML;
    std::memset(&stAML, 0, sizeof(stAML));

    stAML.UID       = UID();
    stAML.MapID     = MapID();
    stAML.X         = X();
    stAML.Y         = Y();
    stAML.Direction = Direction();

    m_actorPod->forward(rstMPK.from(), {MPK_LOCATION, stAML}, rstMPK.ID());
}

void Player::on_MPK_ATTACK(const MessagePack &rstMPK)
{
    AMAttack stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(mathf::LDistance2(X(), Y(), stAMA.X, stAMA.Y) > 2){
        switch(uidf::getUIDType(stAMA.UID)){
            case UID_MON:
            case UID_PLY:
                {
                    AMMiss stAMM;
                    std::memset(&stAMM, 0, sizeof(stAMM));

                    stAMM.UID = stAMA.UID;
                    m_actorPod->forward(stAMA.UID, {MPK_MISS, stAMM});
                    return;
                }
            default:
                {
                    return;
                }
        }
    }

    DispatchAction(ActionHitted(X(), Y(), Direction()));
    StruckDamage({stAMA.UID, stAMA.Type, stAMA.Damage, stAMA.Element});
    ReportAction(UID(), ActionHitted(X(), Y(), Direction()));
}

void Player::on_MPK_UPDATEHP(const MessagePack &rstMPK)
{
    AMUpdateHP stAMUHP;
    std::memcpy(&stAMUHP, rstMPK.Data(), sizeof(stAMUHP));

    if(stAMUHP.UID != UID()){
        SMUpdateHP stSMUHP;
        stSMUHP.UID   = stAMUHP.UID;
        stSMUHP.MapID = stAMUHP.MapID;
        stSMUHP.HP    = stAMUHP.HP;
        stSMUHP.HPMax = stAMUHP.HPMax;

        g_netDriver->Post(ChannID(), SM_UPDATEHP, stSMUHP);
    }
}

void Player::on_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut stAMDFO;
    std::memcpy(&stAMDFO, rstMPK.Data(), sizeof(stAMDFO));

    RemoveInViewCO(stAMDFO.UID);
    if(stAMDFO.UID != UID()){
        SMDeadFadeOut stSMDFO;
        stSMDFO.UID   = stAMDFO.UID;
        stSMDFO.MapID = stAMDFO.MapID;
        stSMDFO.X     = stAMDFO.X;
        stSMDFO.Y     = stAMDFO.Y;
        g_netDriver->Post(ChannID(), SM_DEADFADEOUT, stSMDFO);
    }
}

void Player::on_MPK_EXP(const MessagePack &rstMPK)
{
    AMExp stAME;
    std::memcpy(&stAME, rstMPK.Data(), sizeof(stAME));

    GainExp(stAME.Exp);

    if(stAME.Exp > 0){
        SMExp stSME;
        stSME.Exp = stAME.Exp;
        g_netDriver->Post(ChannID(), SM_EXP, stSME);
    }
}

void Player::on_MPK_MISS(const MessagePack &rstMPK)
{
    AMMiss stAMM;
    std::memcpy(&stAMM, rstMPK.Data(), sizeof(stAMM));

    SMMiss stSMM;
    std::memset(&stSMM, 0, sizeof(stSMM));

    stSMM.UID = stAMM.UID;
    postNetMessage(SM_MISS, stSMM);
}

void Player::on_MPK_SHOWDROPITEM(const MessagePack &rstMPK)
{
    AMShowDropItem stAMSDI;
    std::memcpy(&stAMSDI, rstMPK.Data(), sizeof(stAMSDI));

    SMShowDropItem stSMSDI;
    std::memset(&stSMSDI, 0, sizeof(stSMSDI));

    stSMSDI.X  = stAMSDI.X;
    stSMSDI.Y  = stAMSDI.Y;

    constexpr auto nSMIDListLen = std::extent<decltype(stSMSDI.IDList)>::value;
    constexpr auto nAMIDListLen = std::extent<decltype(stAMSDI.IDList)>::value;

    static_assert(nSMIDListLen >= nAMIDListLen, "");
    for(size_t nIndex = 0; nIndex < nAMIDListLen; ++nIndex){
        if(stAMSDI.IDList[nIndex].ID){
            stSMSDI.IDList[nIndex].ID   = stAMSDI.IDList[nIndex].ID;
            stSMSDI.IDList[nIndex].DBID = stAMSDI.IDList[nIndex].DBID;
        }else{
            break;
        }
    }
    g_netDriver->Post(ChannID(), SM_SHOWDROPITEM, stSMSDI);
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
    AMBadChannel stAMBC;
    std::memcpy(&stAMBC, rstMPK.Data(), sizeof(stAMBC));

    condcheck(ChannID() == stAMBC.ChannID);
    g_netDriver->Shutdown(ChannID(), false);

    Offline();
}

void Player::on_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline stAMO;
    std::memcpy(&stAMO, rstMPK.Data(), sizeof(stAMO));

    reportOffline(stAMO.UID, stAMO.MapID);
}

void Player::on_MPK_REMOVEGROUNDITEM(const MessagePack &rstMPK)
{
    AMRemoveGroundItem stAMRGI;
    std::memcpy(&stAMRGI, rstMPK.Data(), sizeof(stAMRGI));

    SMRemoveGroundItem stSMRGI;
    stSMRGI.X    = stAMRGI.X;
    stSMRGI.Y    = stAMRGI.Y;
    stSMRGI.ID   = stAMRGI.ID;
    stSMRGI.DBID = stAMRGI.DBID;

    postNetMessage(SM_REMOVEGROUNDITEM, stSMRGI);
}

void Player::on_MPK_PICKUPOK(const MessagePack &rstMPK)
{
    AMPickUpOK stAMPUOK;
    std::memcpy(&stAMPUOK, rstMPK.Data(), sizeof(stAMPUOK));

    SMPickUpOK stSMPUOK;
    std::memset(&stSMPUOK, 0, sizeof(stSMPUOK));

    stSMPUOK.X    = stAMPUOK.X;
    stSMPUOK.Y    = stAMPUOK.Y;
    stSMPUOK.ID   = stAMPUOK.ID;
    stSMPUOK.DBID = stAMPUOK.DBID;

    postNetMessage(SM_PICKUPOK, stSMPUOK);

    switch(stAMPUOK.ID){
        case DBCOM_ITEMID(u8"金币"):
            {
                m_gold += std::rand() % 500;
                reportGold();
                break;
            }
        default:
            {
                m_inventory.emplace_back(stAMPUOK.ID, stAMPUOK.DBID);
                break;
            }
    }

}

void Player::on_MPK_CORECORD(const MessagePack &rstMPK)
{
    const auto stAMCOR = rstMPK.conv<AMCORecord>();

    SMCORecord stSMCOR;
    std::memset(&stSMCOR, 0, sizeof(stSMCOR));

    stSMCOR.Action.UID   = stAMCOR.Action.UID;
    stSMCOR.Action.MapID = stAMCOR.Action.MapID;

    stSMCOR.Action.Action    = stAMCOR.Action.Action;
    stSMCOR.Action.Speed     = stAMCOR.Action.Speed;
    stSMCOR.Action.Direction = stAMCOR.Action.Direction;

    stSMCOR.Action.X    = stAMCOR.Action.X;
    stSMCOR.Action.Y    = stAMCOR.Action.Y;
    stSMCOR.Action.AimX = stAMCOR.Action.AimX;
    stSMCOR.Action.AimY = stAMCOR.Action.AimY;

    stSMCOR.Action.AimUID      = stAMCOR.Action.AimUID;
    stSMCOR.Action.ActionParam = stAMCOR.Action.ActionParam;

    switch(uidf::getUIDType(stAMCOR.Action.UID)){
        case UID_PLY:
            {
                stSMCOR.Player.DBID  = stAMCOR.Player.DBID;
                stSMCOR.Player.JobID = stAMCOR.Player.JobID;
                stSMCOR.Player.Level = stAMCOR.Player.Level;
                break;
            }
        case UID_MON:
            {
                stSMCOR.Monster.MonsterID = stAMCOR.Monster.MonsterID;
                break;
            }
        default:
            {
                break;
            }
    }
    postNetMessage(SM_CORECORD, stSMCOR);
}

void Player::on_MPK_NOTIFYDEAD(const MessagePack &)
{
}

void Player::on_MPK_CHECKMASTER(const MessagePack &rstMPK)
{
    m_slaveList.insert(rstMPK.from());
    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
}
