#include <vector>
#include <cstring>
#include <algorithm>
#include "uidf.hpp"
#include "client.hpp"
#include "cerealf.hpp"
#include "serdesmsg.hpp"
#include "clientmsg.hpp"
#include "servermsg.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"
#include "gui/stallboard/stallboard.hpp"

extern Client *g_client;

void ProcessRun::requestAddStallItem(uint32_t itemID, uint32_t seqID, size_t price)
{
    CMAddStallItem cmASI;
    std::memset(&cmASI, 0, sizeof(cmASI));

    cmASI.itemID = itemID;
    cmASI.seqID  = seqID;
    cmASI.price  = price;
    g_client->send({CM_ADDSTALLITEM, cmASI});
}

void ProcessRun::requestRemoveStallItem(uint32_t itemID, uint32_t seqID)
{
    CMRemoveStallItem cmRSI;
    std::memset(&cmRSI, 0, sizeof(cmRSI));

    cmRSI.itemID = itemID;
    cmRSI.seqID  = seqID;
    g_client->send({CM_REMOVESTALLITEM, cmRSI});
}

void ProcessRun::requestStartStall(const std::string &name)
{
    CMStartStall cmSS;
    std::memset(&cmSS, 0, sizeof(cmSS));

    fflassert(name.size() < cmSS.name.capacity(), name.size(), cmSS.name.capacity());
    cmSS.name.assign(name);
    g_client->send({CM_STARTSTALL, cmSS});
}

void ProcessRun::requestCloseStall()
{
    g_client->send(CM_CLOSESTALL);
}

void ProcessRun::requestQueryStall(uint32_t dbid) const
{
    CMQueryStall cmQS;
    std::memset(&cmQS, 0, sizeof(cmQS));

    cmQS.dbid = dbid;
    g_client->send({CM_QUERYSTALL, cmQS});
}

void ProcessRun::requestBuyStallItem(uint32_t sellerDBID, uint32_t itemID, uint32_t seqID, size_t count, size_t price)
{
    CMBuyStallItem cmBSI;
    std::memset(&cmBSI, 0, sizeof(cmBSI));

    cmBSI.sellerDBID = sellerDBID;
    cmBSI.itemID     = itemID;
    cmBSI.seqID      = seqID;
    cmBSI.count      = to_u32(count);
    cmBSI.price      = price;
    g_client->send({CM_BUYSTALLITEM, cmBSI});
}

std::string ProcessRun::getStallSellerName(uint32_t dbid) const
{
    if(dbid == getMyHeroDBID()){
        return getMyHero()->getName();
    }

    if(auto heroPtr = dynamic_cast<Hero *>(findUID(uidf::getPlayerUID(dbid), false))){
        return heroPtr->getName();
    }
    return {};
}

void ProcessRun::openStallBoard(uint32_t dbid)
{
    auto stallBoard = dynamic_cast<StallBoard *>(getWidget("StallBoard"));
    fflassert(stallBoard);

    if(dbid == getMyHeroDBID()){
        auto sdS = m_myStall;
        sdS.dbid = dbid;
        stallBoard->loadStall(std::move(sdS));

        if(!m_myStall.selling){
            auto invBoard = dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard"));
            fflassert(invBoard);

            invBoard->startInvOp({.invOp = INVOP_STALL});
            invBoard->setShow(true);
        }
    }
    else{
        // show what we already know, the query response fills the item list
        const auto stallName = [dbid, this]() -> std::string
        {
            if(auto heroPtr = dynamic_cast<Hero *>(findUID(uidf::getPlayerUID(dbid), false))){
                return heroPtr->getStallName();
            }
            return {};
        }();

        stallBoard->loadStall(SDStall
        {
            .dbid = dbid,
            .selling = true,
            .name = stallName,
        });
        requestQueryStall(dbid);
    }

    stallBoard->setShow(true);
    stallBoard->setFocus(true);
}

bool ProcessRun::processStallBarEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                break;
            }
        default:
            {
                return false;
            }
    }

    std::vector<Hero *> heroList;
    for(const auto &[uid, coPtr]: m_coList){
        if(auto heroPtr = dynamic_cast<Hero *>(coPtr.get()); heroPtr && heroPtr->isStall()){
            heroList.push_back(heroPtr);
        }
    }

    // stall drawn later is on top, it takes event first
    std::sort(heroList.begin(), heroList.end(), [](const Hero *lhs, const Hero *rhs)
    {
        return std::make_tuple(lhs->y(), lhs->x()) > std::make_tuple(rhs->y(), rhs->x());
    });

    bool tookEvent = false;
    for(auto heroPtr: heroList){
        tookEvent = heroPtr->processOverHeadEvent(event, !tookEvent, m_viewX, m_viewY) || tookEvent;
    }
    return tookEvent;
}

void ProcessRun::on_SM_STALL(const uint8_t *buf, size_t bufSize)
{
    m_myStall = cerealf::deserialize<SDStall>(buf, bufSize);
    m_myStall.dbid = to_u32(getMyHeroDBID());

    if(auto myHeroPtr = getMyHero()){
        myHeroPtr->setStall(m_myStall.selling);
        myHeroPtr->setStallName(m_myStall.selling ? m_myStall.name : std::string{});
    }

    if(auto stallBoard = dynamic_cast<StallBoard *>(getWidget("StallBoard")); stallBoard->stallDBID() == m_myStall.dbid){
        stallBoard->loadStall(m_myStall);
    }

    if(m_myStall.selling){
        if(auto invBoard = dynamic_cast<InventoryBoard *>(getWidget("InventoryBoard")); invBoard->getInvOp() == INVOP_STALL){
            invBoard->clearInvOp();
        }
    }
}

void ProcessRun::on_SM_STALLERROR(const uint8_t *buf, size_t bufSize)
{
    const auto smSE = ServerMsg::conv<SMStallError>(buf, bufSize);
    const char8_t *errStr = [&smSE]() -> const char8_t *
    {
        switch(smSE.error){
            case STALLERR_NOTALLOWED: return u8"当前无法摆摊";
            case STALLERR_BUSY      : return u8"交易中，无法摆摊";
            case STALLERR_DEAD      : return u8"死亡状态无法摆摊";
            case STALLERR_SELLING   : return u8"摊位已开张";
            case STALLERR_NOTSELLING: return u8"摊位未开张";
            case STALLERR_FULL      : return u8"摊位已满";
            case STALLERR_EMPTY     : return u8"摊位没有物品";
            case STALLERR_BADITEM   : return u8"无效的物品";
            case STALLERR_BADPRICE  : return u8"无效的价格";
            case STALLERR_BADNAME   : return u8"无效的摊位名称";
            case STALLERR_BADCOUNT  : return u8"无效的数量";
            case STALLERR_NOGOLD    : return u8"金币不足";
            case STALLERR_NOSPACE   : return u8"背包空间不足";
            case STALLERR_SELFBUY   : return u8"不能购买自己的物品";
            case STALLERR_STALE     : return u8"摊位物品已变化，请重新选择";
            case STALLERR_DBFAILED  : return u8"摆摊操作失败";
            default                 : return nullptr;
        }
    }();

    if(errStr){
        addCBLog(CBLOG_ERR, u8"%s", to_cstr(errStr));
    }
    else{
        addCBLog(CBLOG_ERR, u8"摆摊错误：%d", to_d(smSE.error));
    }
}

void ProcessRun::on_SM_PLAYERSTALL(const uint8_t *buf, size_t bufSize)
{
    const auto smPS = ServerMsg::conv<SMPlayerStall>(buf, bufSize);
    if(auto heroPtr = dynamic_cast<Hero *>(findUID(smPS.uid, false))){
        heroPtr->setStall(smPS.selling);
    }

    const auto dbid = uidf::getPlayerDBID(smPS.uid);
    if(dbid == getMyHeroDBID()){
        return; // handled by SM_STALL
    }

    if(!smPS.selling){
        if(auto stallBoard = dynamic_cast<StallBoard *>(getWidget("StallBoard")); stallBoard->stallDBID() == dbid){
            stallBoard->loadStall(SDStall{.dbid = dbid});
        }
    }
}

void ProcessRun::on_SM_QUERYSTALLOK(const uint8_t *buf, size_t bufSize)
{
    auto sdS = cerealf::deserialize<SDStall>(buf, bufSize);
    if(sdS.dbid == getMyHeroDBID()){
        return;
    }

    if(auto heroPtr = dynamic_cast<Hero *>(findUID(uidf::getPlayerUID(sdS.dbid), false))){
        heroPtr->setStall(sdS.selling);
        heroPtr->setStallName(sdS.selling ? sdS.name : std::string{});
    }

    if(auto stallBoard = dynamic_cast<StallBoard *>(getWidget("StallBoard")); stallBoard->stallDBID() == sdS.dbid){
        stallBoard->loadStall(std::move(sdS));
    }
}
