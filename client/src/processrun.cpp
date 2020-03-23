/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
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
#include "dbcomid.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "mapbindb.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"
#include "pathfinder.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "clientluamodule.hpp"
#include "clientpathfinder.hpp"
#include "debugboard.hpp"
#include "notifyboard.hpp"

extern Log *g_Log;
extern Client *g_Client;
extern PNGTexDB *g_MapDB;
extern MapBinDB *g_MapBinDB;
extern SDLDevice *g_SDLDevice;
extern PNGTexDB *g_GroundItemDB;
extern NotifyBoard *g_NotifyBoard;
extern DebugBoard *g_DebugBoard;
extern ClientArgParser *g_ClientArgParser;

ProcessRun::ProcessRun()
    : Process()
    , m_MapID(0)
    , m_Mir2xMapData()
    , m_GroundItemList()
    , m_MyHeroUID(0)
    , m_FocusUIDTable()
    , m_ViewX(0)
    , m_ViewY(0)
    , m_RollMap(false)
    , m_LuaModule(this, OUTPORT_CONTROLBOARD)
    , m_ControbBoard(
            0,                  // x
            []() -> int         // y
            {
                return g_SDLDevice->WindowH(false) - 134;
            }(),
            []() -> int         // w
            {
                return g_SDLDevice->WindowW(false);
            }(),
            this,               // self-bind
            nullptr,            // independent widget
            false)              // 
    , m_InventoryBoard(0, 0, this)
    , m_CreatureList()
    , m_UIDPending()
    , m_MousePixlLoc(0, 0, "", 0, 15, 0, ColorFunc::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_MouseGridLoc(0, 0, "", 0, 15, 0, ColorFunc::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_AscendStrList()
{
    m_FocusUIDTable.fill(0);
    RegisterUserCommand();
}

void ProcessRun::ScrollMap()
{
    auto nShowWindowW = g_SDLDevice->WindowW(false);
    auto nShowWindowH = g_SDLDevice->WindowH(false) - 134;

    int nViewX = GetMyHero()->X() * SYS_MAPGRIDXP - nShowWindowW / 2;
    int nViewY = GetMyHero()->Y() * SYS_MAPGRIDYP - nShowWindowH / 2;

    int nDViewX = nViewX - m_ViewX;
    int nDViewY = nViewY - m_ViewY;

    if(m_RollMap
            ||  (std::abs(nDViewX) > nShowWindowW / 6)
            ||  (std::abs(nDViewY) > nShowWindowH / 6)){

        m_RollMap = true;

        m_ViewX += (int)(std::lround(std::copysign((std::min<int>)(3, std::abs(nDViewX)), nDViewX)));
        m_ViewY += (int)(std::lround(std::copysign((std::min<int>)(2, std::abs(nDViewY)), nDViewY)));

        m_ViewX = (std::max<int>)(0, m_ViewX);
        m_ViewY = (std::max<int>)(0, m_ViewY);
    }

    // stop rolling the map when
    //   1. the hero is at the required position
    //   2. the hero is not moving
    if((nDViewX == 0) && (nDViewY == 0) && !GetMyHero()->Moving()){
        m_RollMap = false;
    }
}

void ProcessRun::Update(double fUpdateTime)
{
    ScrollMap();
    m_ControbBoard.Update(fUpdateTime);

    for(auto p = m_CreatureList.begin(); p != m_CreatureList.end();){
        if(p->second->Visible()){
            if(p->second->LastActive() + 5000 < SDL_GetTicks() && p->second->LastQuerySelf() + 5000 < SDL_GetTicks()){
                p->second->QuerySelf();
            }
            p->second->Update(fUpdateTime);
            ++p;
        }else{
            p = m_CreatureList.erase(p);
        }
    }

    for(auto p = m_IndepMagicList.begin(); p != m_IndepMagicList.end();){
        if((*p)->Done()){
            p = m_IndepMagicList.erase(p);
        }else{
            (*p)->Update(fUpdateTime);
            ++p;
        }
    }

    for(auto p = m_AscendStrList.begin(); p != m_AscendStrList.end();){
        if((*p)->Ratio() < 1.00){
            (*p)->Update(fUpdateTime);
            ++p;
        }else{
            p = m_AscendStrList.erase(p);
        }
    }

    if(auto p = RetrieveUID(m_FocusUIDTable[FOCUS_ATTACK])){
        if(p->StayDead()){
            m_FocusUIDTable[FOCUS_ATTACK] = 0;
        }else{
            TrackAttack(false, m_FocusUIDTable[FOCUS_ATTACK]);
        }
    }else{
        m_FocusUIDTable[FOCUS_ATTACK] = 0;
    }
}

uint64_t ProcessRun::FocusUID(int nFocusType)
{
    if(nFocusType < (int)(m_FocusUIDTable.size())){
        switch(nFocusType){
            case FOCUS_NONE:
                {
                    return 0;
                }
            case FOCUS_MOUSE:
                {
                    // use the cached mouse focus first
                    // if can't get it then scan the whole creature list

                    auto fnCheckFocus = [this](uint64_t nUID, int nX, int nY) -> bool
                    {
                        if(auto pCreature = RetrieveUID(nUID)){
                            if(nUID != m_MyHeroUID){
                                if(pCreature->CanFocus(nX, nY)){
                                    return true;
                                }
                            }
                        }
                        return false;
                    };
                    

                    int nPointX = -1;
                    int nPointY = -1;
                    SDL_GetMouseState(&nPointX, &nPointY);

                    int nCheckPointX = nPointX + m_ViewX;
                    int nCheckPointY = nPointY + m_ViewY;

                    if(fnCheckFocus(m_FocusUIDTable[FOCUS_MOUSE], nCheckPointX, nCheckPointY)){
                        return m_FocusUIDTable[FOCUS_MOUSE];
                    }

                    Creature *pFocus = nullptr;
                    for(auto pRecord: m_CreatureList){
                        if(fnCheckFocus(pRecord.second->UID(), nCheckPointX, nCheckPointY)){
                            if(false
                                    || !pFocus
                                    ||  pFocus->Y() < pRecord.second->Y()){
                                // 1. currently we have no candidate yet
                                // 2. we have candidate but it's not at more front location
                                pFocus = pRecord.second.get();
                            }
                        }
                    }

                    m_FocusUIDTable[FOCUS_MOUSE] = pFocus ? pFocus->UID() : 0;
                    return m_FocusUIDTable[FOCUS_MOUSE];
                }
            default:
                {
                    return m_FocusUIDTable[nFocusType];
                }
        }
    }
    
    return 0;
}

void ProcessRun::Draw()
{
    g_SDLDevice->ClearScreen();

    // 1. draw map + object
    {
        int nX0 = -SYS_OBJMAXW + (m_ViewX - 2 * SYS_MAPGRIDXP) / SYS_MAPGRIDXP;
        int nY0 = -SYS_OBJMAXH + (m_ViewY - 2 * SYS_MAPGRIDYP) / SYS_MAPGRIDYP;
        int nX1 = +SYS_OBJMAXW + (m_ViewX + 2 * SYS_MAPGRIDXP + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
        int nY1 = +SYS_OBJMAXH + (m_ViewY + 2 * SYS_MAPGRIDYP + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

        // tiles
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY) && !(nX % 2) && !(nY % 2)){
                    auto &rstTile = m_Mir2xMapData.Tile(nX, nY);
                    if(rstTile.Valid()){
                        if(auto pTexture = g_MapDB->Retrieve(rstTile.Image())){
                            g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, nY * SYS_MAPGRIDYP - m_ViewY);
                        }
                    }
                }
            }
        }

        // ground objects
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY)){
                    for(int nIndex = 0; nIndex < 2; ++nIndex){
                        auto stArray = m_Mir2xMapData.Cell(nX, nY).ObjectArray(nIndex);
                        if(true
                                && (stArray[4] & 0X80)
                                && (stArray[4] & 0X01)){
                            uint32_t nImage = 0
                                | (((uint32_t)(stArray[2])) << 16)
                                | (((uint32_t)(stArray[1])) <<  8)
                                | (((uint32_t)(stArray[0])) <<  0);
                            if(auto pTexture = g_MapDB->Retrieve(nImage)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(g_ClientArgParser->EnableDrawMapGrid){
            int nGridX0 = m_ViewX / SYS_MAPGRIDXP;
            int nGridY0 = m_ViewY / SYS_MAPGRIDYP;

            int nGridX1 = (m_ViewX + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
            int nGridY1 = (m_ViewY + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

            g_SDLDevice->PushColor(0, 255, 0, 128);
            for(int nX = nGridX0; nX <= nGridX1; ++nX){
                g_SDLDevice->DrawLine(nX * SYS_MAPGRIDXP - m_ViewX, 0, nX * SYS_MAPGRIDXP - m_ViewX, g_SDLDevice->WindowH(false));
            }
            for(int nY = nGridY0; nY <= nGridY1; ++nY){
                g_SDLDevice->DrawLine(0, nY * SYS_MAPGRIDYP - m_ViewY, g_SDLDevice->WindowW(false), nY * SYS_MAPGRIDYP - m_ViewY);
            }
            g_SDLDevice->PopColor();
        }

        // draw dead actors
        // dead actors are shown before all active actors
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                for(auto pCreature: m_CreatureList){
                    if(true
                            && (pCreature.second)
                            && (pCreature.second->X() == nX)
                            && (pCreature.second->Y() == nY)
                            && (pCreature.second->StayDead())){
                        pCreature.second->Draw(m_ViewX, m_ViewY, 0);
                    }
                }
            }
        }

        // draw ground item
        // should be over dead actors
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                auto &rstGroundItemList = GetGroundItemListRef(nX, nY);
                for(auto rstGroundItem: rstGroundItemList){
                    if(auto &rstIR = DBCOM_ITEMRECORD(rstGroundItem.ID())){
                        if(rstIR.PkgGfxID >= 0){
                            if(auto pTexture = g_GroundItemDB->Retrieve(rstIR.PkgGfxID)){
                                int nW = -1;
                                int nH = -1;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH)){
                                    int nXt = nX * SYS_MAPGRIDXP - m_ViewX + SYS_MAPGRIDXP / 2 - nW / 2;
                                    int nYt = nY * SYS_MAPGRIDYP - m_ViewY + SYS_MAPGRIDYP / 2 - nH / 2;

                                    int nPointX = -1;
                                    int nPointY = -1;
                                    SDL_GetMouseState(&nPointX, &nPointY);

                                    int nCurrX = (nPointX + m_ViewX) / SYS_MAPGRIDXP;
                                    int nCurrY = (nPointY + m_ViewY) / SYS_MAPGRIDYP;

                                    bool bChoose = false;
                                    if(true
                                            && nCurrX == nX
                                            && nCurrY == nY){
                                        bChoose = true;
                                        SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_ADD);
                                    }else{
                                        SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
                                    }

                                    // 1. draw item shadow
                                    SDL_SetTextureColorMod(pTexture, 0, 0, 0);
                                    SDL_SetTextureAlphaMod(pTexture, 128);
                                    g_SDLDevice->DrawTexture(pTexture, nXt + 1, nYt - 1);

                                    // 2. draw item body
                                    SDL_SetTextureColorMod(pTexture, 255, 255, 255);
                                    SDL_SetTextureAlphaMod(pTexture, 255);
                                    g_SDLDevice->DrawTexture(pTexture, nXt, nYt);

                                    if(bChoose){
                                        LabelBoard stItemName(0, 0, rstIR.Name, 1, 12, 0, ColorFunc::RGBA(0XFF, 0XFF, 0X00, 0X00));
                                        int nLW = stItemName.W();
                                        int nLH = stItemName.H();

                                        int nLXt = nX * SYS_MAPGRIDXP - m_ViewX + SYS_MAPGRIDXP / 2 - nLW / 2;
                                        int nLYt = nY * SYS_MAPGRIDYP - m_ViewY + SYS_MAPGRIDYP / 2 - nLH / 2 - 20;

                                        stItemName.DrawEx(nLXt, nLYt, 0, 0, nLW, nLH);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // over ground objects
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY)){
                    for(int nIndex = 0; nIndex < 2; ++nIndex){
                        auto stArray = m_Mir2xMapData.Cell(nX, nY).ObjectArray(nIndex);
                        if(true
                                &&  (stArray[4] & 0X80)
                                && !(stArray[4] & 0X01)){
                            uint32_t nImage = 0
                                | (((uint32_t)(stArray[2])) << 16)
                                | (((uint32_t)(stArray[1])) <<  8)
                                | (((uint32_t)(stArray[0])) <<  0);
                            if(auto pTexture = g_MapDB->Retrieve(nImage)){
                                int nH = 0;
                                if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                    g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                }
                            }
                        }
                    }
                }
            }

            // draw alive actors
            for(int nX = nX0; nX <= nX1; ++nX){
                for(auto pCreature: m_CreatureList){
                    if(true
                            &&  (pCreature.second)
                            &&  (pCreature.second->X() == nX)
                            &&  (pCreature.second->Y() == nY)
                            && !(pCreature.second->StayDead())){

                        if(g_ClientArgParser->EnableDrawCreatureCover){
                            g_SDLDevice->PushColor(0, 0, 255, 128);
                            g_SDLDevice->PushBlendMode(SDL_BLENDMODE_BLEND);
                            g_SDLDevice->FillRectangle(nX * SYS_MAPGRIDXP - m_ViewX, nY * SYS_MAPGRIDYP - m_ViewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                            g_SDLDevice->PopBlendMode();
                            g_SDLDevice->PopColor();
                        }

                        int nFocusMask = 0;
                        for(auto nFocus = 0; nFocus < FOCUS_MAX; ++nFocus){
                            if(FocusUID(nFocus) == pCreature.second->UID()){
                                nFocusMask |= (1 << nFocus);
                            }
                        }
                        pCreature.second->Draw(m_ViewX, m_ViewY, nFocusMask);
                    }
                }
            }
        }

        // draw all rotating stars
        // to aware players there is somethig to check

        m_starRatio += 0.05;
        if(m_starRatio >= 2.50){
            m_starRatio = 0.00;
        }else if(m_starRatio >= 1.00){
            // do nothing
            // hide the star to avoid blinking too much
        }else{
            for(int nY = nY0; nY <= nY1; ++nY){
                for(int nX = nX0; nX <= nX1; ++nX){

                    if(!GetGroundItemListRef(nX, nY).empty()){
                        if(auto pTexture = g_GroundItemDB->Retrieve(0X01000000)){
                            int nW = -1;
                            int nH = -1;
                            if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH)){
                                if(auto nLt = (int)(std::lround(m_starRatio * nW / 2.50))){
                                    auto nXt = nX * SYS_MAPGRIDXP - m_ViewX + SYS_MAPGRIDXP / 2 - nLt / 2;
                                    auto nYt = nY * SYS_MAPGRIDYP - m_ViewY + SYS_MAPGRIDYP / 2 - nLt / 2;

                                    // to make this to be more informative
                                    // use different color of rotating star for different type

                                    SDL_SetTextureAlphaMod(pTexture, 128);
                                    g_SDLDevice->DrawTextureEx(pTexture,
                                            0,
                                            0,
                                            nW,
                                            nH,
                                            nXt,
                                            nYt,
                                            nLt,
                                            nLt,
                                            nLt / 2,
                                            nLt / 2,
                                            std::lround(m_starRatio * 360.0));
                                }
                            }
                        }
                    }
                }
            }
        }

        // draw magics
        for(auto p: m_IndepMagicList){
            if(!p->Done()){
                p->Draw(m_ViewX, m_ViewY);
            }
        }

        // put it here
        // any other should draw before GUI
    }

    // draw underlay at the bottom
    // there is one pixel transparent rectangle
    {
        auto nWindowW = g_SDLDevice->WindowW(false);
        auto nWindowH = g_SDLDevice->WindowH(false);

        g_SDLDevice->PushColor(0, 0, 0, 0);
        g_SDLDevice->FillRectangle(0, nWindowH - 4, nWindowW, 4);
        g_SDLDevice->PopColor();
    }

    for(auto p: m_AscendStrList){
        p->Draw(m_ViewX, m_ViewY);
    }

    m_ControbBoard  .Draw();
    m_InventoryBoard.Draw();

    // draw notifyBoard
    {
        const int w = g_NotifyBoard->W();
        const int h = g_NotifyBoard->H();
        g_NotifyBoard->DrawEx(0, 0, 0, 0, w, h);
    }

    // draw debugBoard
    {
        const int x = 0;
        const int y = 0;
        const int w = std::max<int>(g_DebugBoard->PW(), 200);
        const int h = g_DebugBoard->H();

        {
            SDLDevice::EnableDrawColor enableColor(ColorFunc::GREEN + 200);
            SDLDevice::EnableDrawBlendMode enableBlend(SDL_BLENDMODE_BLEND);
            g_SDLDevice->FillRectangle(x, y, w, h);
        }

        g_DebugBoard->DrawEx(x, y, 0, 0, w, h);
        {
            SDLDevice::EnableDrawColor enableColor(ColorFunc::BLUE + 100);
            g_SDLDevice->DrawRectangle(x, y, w, h);
        }
    }

    // draw cursor location information on top-left
    if(g_ClientArgParser->EnableDrawMouseLocation){
        g_SDLDevice->PushColor(0, 0, 0, 230);
        g_SDLDevice->PushBlendMode(SDL_BLENDMODE_BLEND);
        g_SDLDevice->FillRectangle(0, 0, 200, 60);
        g_SDLDevice->PopBlendMode();
        g_SDLDevice->PopColor();

        int nPointX = -1;
        int nPointY = -1;
        SDL_GetMouseState(&nPointX, &nPointY);

        m_MousePixlLoc.SetText("Pix_Loc: %3d, %3d", nPointX, nPointY);
        m_MouseGridLoc.SetText("Til_Loc: %3d, %3d", (nPointX + m_ViewX) / SYS_MAPGRIDXP, (nPointY + m_ViewY) / SYS_MAPGRIDYP);

        m_MouseGridLoc.DrawEx(10, 10, 0, 0, m_MouseGridLoc.W(), m_MouseGridLoc.H());
        m_MousePixlLoc.DrawEx(10, 30, 0, 0, m_MousePixlLoc.W(), m_MousePixlLoc.H());
    }

    g_SDLDevice->Present();
}

void ProcessRun::ProcessEvent(const SDL_Event &rstEvent)
{
    bool bValid = true;
    if(m_InventoryBoard.ProcessEvent(rstEvent, &bValid)){ return; }
    if(m_ControbBoard  .ProcessEvent(rstEvent, &bValid)){ return; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                int nMouseGridX = -1;
                int nMouseGridY = -1;
                ScreenPoint2Grid(rstEvent.button.x, rstEvent.button.y, &nMouseGridX, &nMouseGridY);

                switch(rstEvent.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDTable[FOCUS_ATTACK] = nUID;
                                TrackAttack(true, nUID);
                            }else{
                                auto &rstGroundItemList = GetGroundItemListRef(nMouseGridX, nMouseGridY);
                                if(!rstGroundItemList.empty()){
                                    GetMyHero()->EmplaceAction(ActionPickUp(nMouseGridX, nMouseGridY, rstGroundItemList.back().ID()));
                                }
                            }
                            break;
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            // in mir2ei how human moves
                            // 1. client send motion request to server
                            // 2. client put motion lock to human
                            // 3. server response with "+GOOD" or "+FAIL" to client
                            // 4. if "+GOOD" client will release the motion lock
                            // 5. if "+FAIL" client will use the backup position and direction

                            m_FocusUIDTable[FOCUS_ATTACK] = 0;
                            m_FocusUIDTable[FOCUS_FOLLOW] = 0;

                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDTable[FOCUS_FOLLOW] = nUID;
                            }else{
                                int nX = -1;
                                int nY = -1;
                                if(true
                                        && ScreenPoint2Grid(rstEvent.button.x, rstEvent.button.y, &nX, &nY)
                                        && MathFunc::LDistance2(GetMyHero()->CurrMotion().EndX, GetMyHero()->CurrMotion().EndY, nX, nY)){

                                    // we get a valid dst to go
                                    // provide myHero with new move action command

                                    // when post move action don't use X() and Y()
                                    // since if clicks during hero moving then X() may not equal to EndX

                                    GetMyHero()->EmplaceAction(ActionMove
                                    {
                                        GetMyHero()->CurrMotion().EndX,    // don't use X()
                                        GetMyHero()->CurrMotion().EndY,    // don't use Y()
                                        nX,
                                        nY,
                                        SYS_DEFSPEED,
                                        GetMyHero()->OnHorse() ? 1 : 0
                                    });
                                }
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                break;
            }
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_e:
                        {
                            std::exit(0);
                            break;
                        }
                    case SDLK_ESCAPE:
                        {
                            CenterMyHero();
                            break;
                        }
                    case SDLK_t:
                        {
                            if(auto nMouseFocusUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDTable[FOCUS_MAGIC] = nMouseFocusUID;
                            }else{
                                if(!RetrieveUID(m_FocusUIDTable[FOCUS_MAGIC])){
                                    m_FocusUIDTable[FOCUS_MAGIC] = 0;
                                }
                            }

                            if(auto nFocusUID = FocusUID(FOCUS_MAGIC)){
                                GetMyHero()->EmplaceAction(ActionSpell
                                {
                                    GetMyHero()->CurrMotion().EndX,
                                    GetMyHero()->CurrMotion().EndY,
                                    nFocusUID,
                                    DBCOM_MAGICID(u8"雷电术"),
                                });
                            }else{
                                int nAimX   = -1;
                                int nAimY   = -1;
                                int nMouseX = -1;
                                int nMouseY = -1;
                                SDL_GetMouseState(&nMouseX, &nMouseY);
                                ScreenPoint2Grid(nMouseX, nMouseY, &nAimX, &nAimY);
                                GetMyHero()->EmplaceAction(ActionSpell
                                {
                                    GetMyHero()->CurrMotion().EndX,
                                    GetMyHero()->CurrMotion().EndY,
                                    nAimX,
                                    nAimY,
                                    DBCOM_MAGICID(u8"雷电术"),
                                });
                            }
                            break;
                        }
                    case SDLK_p:
                        {
                            GetMyHero()->PickUp();
                            break;
                        }
                    case SDLK_y:
                        {
                            GetMyHero()->EmplaceAction(ActionSpell(GetMyHero()->X(), GetMyHero()->Y(), GetMyHero()->UID(), DBCOM_MAGICID(u8"魔法盾")));
                            break;
                        }
                    case SDLK_u:
                        {
                            GetMyHero()->EmplaceAction(ActionSpell(GetMyHero()->X(), GetMyHero()->Y(), GetMyHero()->UID(), DBCOM_MAGICID(u8"召唤骷髅")));
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SDL_TEXTEDITING:
            {
                break;
            }
        default:
            {
                break;
            }
    }
}

int ProcessRun::LoadMap(uint32_t nMapID)
{
    if(nMapID){
        if(auto pMapBin = g_MapBinDB->Retrieve(nMapID)){
            m_MapID        =  nMapID;
            m_Mir2xMapData = *pMapBin;

            auto nW = m_Mir2xMapData.W();
            auto nH = m_Mir2xMapData.H();

            m_GroundItemList.clear();
            m_GroundItemList.resize(nW);

            for(int nX = 0; nX < nW; ++nX){
                m_GroundItemList[nX].resize(nH);
            }

            return 0;
        }
    }

    m_MapID = 0;
    return -1;
}

bool ProcessRun::CanMove(bool bCheckGround, int nCheckCreature, int nX, int nY)
{
    switch(auto nGrid = CheckPathGrid(nX, nY)){
        case PathFind::FREE:
            {
                return true;
            }
        case PathFind::OBSTACLE:
        case PathFind::INVALID:
            {
                return bCheckGround ? false : true;
            }
        case PathFind::OCCUPIED:
        case PathFind::LOCKED:
            {
                switch(nCheckCreature){
                    case 0:
                    case 1:
                        {
                            return true;
                        }
                    case 2:
                        {
                            return false;
                        }
                    default:
                        {
                            g_Log->AddLog(LOGTYPE_FATAL, "Invalid CheckCreature provided: %d, should be (0, 1, 2)", nCheckCreature);
                            return false;
                        }
                }
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid grid provided: %d at (%d, %d)", nGrid, nX, nY);
                return false;
            }
    }
}

int ProcessRun::CheckPathGrid(int nX, int nY) const
{
    if(!m_Mir2xMapData.ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_Mir2xMapData.Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    // we should take EndX/EndY, not X()/Y() as occupied
    // because server only checks EndX/EndY, if we use X()/Y() to request move it just fails

    bool bLocked = false;
    for(auto pCreature: m_CreatureList){
        if(true
                && (pCreature.second)
                && (pCreature.second->CurrMotion().EndX == nX)
                && (pCreature.second->CurrMotion().EndY == nY)){
            return PathFind::OCCUPIED;
        }

        if(!bLocked
                && pCreature.second->X() == nX
                && pCreature.second->Y() == nY){
            bLocked = true;
        }
    }
    return bLocked ? PathFind::LOCKED : PathFind::FREE;
}

bool ProcessRun::CanMove(bool bCheckGround, int nCheckCreature, int nX0, int nY0, int nX1, int nY1)
{
    return OneStepCost(nullptr, bCheckGround, nCheckCreature, nX0, nY0, nX1, nY1) >= 0.00;
}

double ProcessRun::OneStepCost(const ClientPathFinder *pFinder, bool bCheckGround, int nCheckCreature, int nX0, int nY0, int nX1, int nY1) const
{
    switch(nCheckCreature){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid CheckCreature provided: %d, should be (0, 1, 2)", nCheckCreature);
                break;
            }
    }

    int nMaxIndex = -1;
    switch(MathFunc::LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                nMaxIndex = 0;
                break;
            }
        case 1:
        case 2:
            {
                nMaxIndex = 1;
                break;
            }
        case 4:
        case 8:
            {
                nMaxIndex = 2;
                break;
            }
        case  9:
        case 18:
            {
                nMaxIndex = 3;
                break;
            }
        default:
            {
                return -1.00;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fExtraPen = 0.00;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        int nCurrX = nX0 + nDX * nIndex;
        int nCurrY = nY0 + nDY * nIndex;
        switch(auto nGrid = pFinder ? pFinder->GetGrid(nCurrX, nCurrY) : this->CheckPathGrid(nCurrX, nCurrY)){
            case PathFind::FREE:
                {
                    break;
                }
            case PathFind::LOCKED:
            case PathFind::OCCUPIED:
                {
                    switch(nCheckCreature){
                        case 1:
                            {
                                fExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return -1.00;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            case PathFind::INVALID:
            case PathFind::OBSTACLE:
                {
                    if(bCheckGround){
                        return -1.00;
                    }

                    fExtraPen += 10000.00;
                    break;
                }
            default:
                {
                    g_Log->AddLog(LOGTYPE_FATAL, "Invalid grid provided: %d at (%d, %d)", nGrid, nCurrX, nCurrY);
                    break;
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
}

bool ProcessRun::ScreenPoint2Grid(int nPX, int nPY, int *pX, int *pY)
{
    if(pX){ *pX = (nPX + m_ViewX) / SYS_MAPGRIDXP; }
    if(pY){ *pY = (nPY + m_ViewY) / SYS_MAPGRIDYP; }

    return true;
}

bool ProcessRun::LuaCommand(const char *szLuaCommand)
{
    if(szLuaCommand){
        auto stCallResult = m_LuaModule.GetLuaState().script(szLuaCommand, [](lua_State *, sol::protected_function_result stResult){
            // default handler
            // do nothing and let the call site handle the errors
            return stResult;
        });

        if(stCallResult.valid()){
            // default nothing printed
            // we can put information here to show call succeeds
            // or we can unlock the input widget to allow next command
        }else{
            sol::error stError = stCallResult;

            g_Log->AddLog(LOGTYPE_WARNING, "%s", stError.what());
        }

        // always return true if command get evaluated
        // return false only if provided is unhandled
        return true;
    }
    return false;
}

bool ProcessRun::UserCommand(const char *szUserCommand)
{
    if(szUserCommand){
        // 1. split commands into an array
        std::vector<std::string> stvToken;
        {
            auto pBegin = szUserCommand;
            auto pEnd   = szUserCommand + std::strlen(szUserCommand);

            while(true){
                pBegin = std::find_if_not(pBegin, pEnd, [](char chByte){ return chByte == ' '; });
                if(pBegin != pEnd){
                    auto pDone = std::find(pBegin, pEnd, ' ');
                    stvToken.emplace_back(pBegin, pDone);
                    pBegin = pDone;
                }else{ break; }
            }

            if(stvToken.empty()){
                return true;
            }
        }

        // 2. to find the command
        //    alert and return if can't find it
        UserCommandEntry *pEntry = nullptr;
        {
            int nCount = 0;
            for(auto &rstEntry : m_UserCommandGroup){
                if(rstEntry.Command.substr(0, stvToken[0].size()) == stvToken[0]){
                    pEntry = &rstEntry;
                    nCount++;
                }
            }

            switch(nCount){
                case 0:
                    {
                        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Invalid command: %s", stvToken[0].c_str());
                        break;
                    }
                case 1:
                    {
                        if(pEntry->Callback){
                            pEntry->Callback(stvToken);
                        }
                        break;
                    }
                default:
                    {
                        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Ambiguous command: %s", stvToken[0].c_str());
                        for(auto &rstEntry : m_UserCommandGroup){
                            if(rstEntry.Command.substr(0, stvToken[0].size()) == stvToken[0]){
                                AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Possible command: %s", rstEntry.Command.c_str());
                            }
                        }
                        break;
                    }
            }
        }
    }
    return false;
}

std::vector<int> ProcessRun::GetPlayerList()
{
    std::vector<int> stRetV {};
    for(auto p = m_CreatureList.begin(); p != m_CreatureList.end();){
        if(p->second->Visible()){
            switch(p->second->Type()){
                case CREATURE_PLAYER:
                    {
                        stRetV.push_back(p->second->UID());
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
            ++p;
        }else{
            p = m_CreatureList.erase(p);
        }
    }
    return stRetV;
}

bool ProcessRun::RegisterUserCommand()
{
    // 1. register command: moveTo
    //    usage:
    //              moveTo        120 250
    //              moveTo 某地图 120 250
    auto fnMoveTo = [this](const std::vector<std::string> &rstvParam) -> int
    {
        switch(rstvParam.size()){
            case 1 + 2:
                {
                    int nX = std::atoi(rstvParam[1].c_str());
                    int nY = std::atoi(rstvParam[2].c_str());
                    RequestSpaceMove(MapID(), nX, nY);
                    return 0;
                }
            case 1 + 3:
                {
                    int nX = std::atoi(rstvParam[2].c_str());
                    int nY = std::atoi(rstvParam[3].c_str());
                    auto nMapID = DBCOM_MAPID(rstvParam[1].c_str());
                    RequestSpaceMove(nMapID, nX, nY);
                    return 0;
                }
            default:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Invalid argument to command: moveTo");
                    return 1;
                }
        }
    };
    m_UserCommandGroup.emplace_back("moveTo", fnMoveTo);

    auto fnLuaEditor = [this](const std::vector<std::string> &) -> int
    {
        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "LuaEditor not implemented yet");
        return 0;
    };
    m_UserCommandGroup.emplace_back("luaEditor", fnLuaEditor);

    auto fnMakeItem = [this](const std::vector<std::string> &rstvParam) -> int
    {
        switch(rstvParam.size()){
            case 0 + 1:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"@make 物品名字");
                    return 1;
                }
            case 1 + 1:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"获得%s", rstvParam[1].c_str());
                    return 0;
                }
            default:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"Invalid argument for @make");
                    return 1;
                }
        }
    };
    m_UserCommandGroup.emplace_back("makeItem", fnMakeItem);

    auto fnGetAttackUID = [this](const std::vector<std::string> &) -> int
    {
        AddOPLog(OUTPORT_CONTROLBOARD, 2, "", std::to_string(FocusUID(FOCUS_ATTACK)).c_str());
        return 0;
    };
    m_UserCommandGroup.emplace_back("getAttackUID", fnGetAttackUID);

    return true;
}

bool ProcessRun::RegisterLuaExport(ClientLuaModule *pModule, int nOutPort)
{
    if(pModule){

        // initialization before registration
        pModule->GetLuaState().script(R"()");

        // register command printLine
        // print one line to the out port allocated for the lua module
        pModule->GetLuaState().set_function("printLine", [this, nOutPort](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo)
        {
            if(true
                    && stLogType.is<int>()
                    &&  stPrompt.is<std::string>()
                    && stLogInfo.is<std::string>()){
                AddOPLog(nOutPort, stLogType.as<int>(), stPrompt.as<std::string>().c_str(), "%s", stLogInfo.as<std::string>().c_str());
                return;
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "%s", "printLine(LogType: int, Prompt: string, LogInfo: string)");
        });

        // register command addLog
        // add to client system log file only, same as g_Log->AddLog(LOGTYPE_XXXX, LogInfo)
        pModule->GetLuaState().set_function("addLog", [this, nOutPort](sol::object stLogType, sol::object stLogInfo)
        {
            if(true
                    && stLogType.is<int>()
                    && stLogInfo.is<std::string>()){

                switch(stLogType.as<int>()){
                    case 0  : g_Log->AddLog(LOGTYPE_INFO   , "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 1  : g_Log->AddLog(LOGTYPE_WARNING, "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 2  : g_Log->AddLog(LOGTYPE_FATAL  , "%s", stLogInfo.as<std::string>().c_str()); return;
                    default : g_Log->AddLog(LOGTYPE_DEBUG  , "%s", stLogInfo.as<std::string>().c_str()); return;
                }
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "%s", "addLog(LogType: int, LogInfo: string)");
        });

        // register command playerList
        // return a table (userData) to lua for ipairs() check
        pModule->GetLuaState().set_function("playerList", [this](sol::this_state stThisLua)
        {
            return sol::make_object(sol::state_view(stThisLua), GetPlayerList());
        });

        // register command moveTo(x, y)
        // wait for server to move player if possible
        pModule->GetLuaState().set_function("moveTo", [this, nOutPort](sol::object stLocX, sol::object stLocY)
        {
            if(true
                    && stLocX.is<int>()
                    && stLocY.is<int>()){

                int nX = stLocX.as<int>();
                int nY = stLocY.as<int>();

                RequestSpaceMove(MapID(), nX, nY);
                AddOPLog(nOutPort, 0, ">", "moveTo(%d, %d)", nX, nY);
            }
        });


        // register command ``listPlayerInfo"
        // this command call to get a player info table and print to out port
        pModule->GetLuaState().script(R"#(
            function listPlayerInfo ()
                for k, v in ipairs(playerList())
                do
                    printLine(0, "> ", tostring(v))
                end
            end
        )#");

        // register command ``help"
        // part-1: divide into two parts, part-1 create the table for help
        pModule->GetLuaState().script(R"#(
            helpInfoTable = {
                wear     = "put on different dress",
                moveTo   = "move to other position on current map",
                randMove = "random move on current map"
            }
        )#");

        // part-2: make up the function to print the table entry
        pModule->GetLuaState().script(R"#(
            function help (queryKey)
                if helpInfoTable[queryKey]
                then
                    printLine(0, "> ", helpInfoTable[queryKey])
                else
                    printLine(2, "> ", "No entry find for input")
                end
            end
        )#");

        // register command ``myHero.xxx"
        // I need to insert a table to micmic a instance myHero in the future
        pModule->GetLuaState().set_function("myHero_dress", [this](int nDress)
        {
            if(nDress >= 0){
                GetMyHero()->Dress((uint32_t)(nDress));
            }
        });

        // register command ``myHero.xxx"
        // I need to insert a table to micmic a instance myHero in the future
        pModule->GetLuaState().set_function("myHero_weapon", [this](int nWeapon)
        {
            if(nWeapon >= 0){
                GetMyHero()->Weapon((uint32_t)(nWeapon));
            }
        });


        // registration done
        return true;
    }
    return false;
}

void ProcessRun::AddOPLog(int nOutPort, int nLogType, const char *szPrompt, const char *szLogFormat, ...)
{
    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in ProcessRun::AddOPLog(\"%s\", ...): %s", szLogFormat, e.what());
        }

        va_end(ap);
    }

    if(bError){
        nLogType = Log::LOGTYPEV_WARNING;
    }

    if(nOutPort & OUTPORT_LOG){
        switch(nLogType){
            case Log::LOGTYPEV_INFO    : g_Log->AddLog(LOGTYPE_INFO   , u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            case Log::LOGTYPEV_WARNING : g_Log->AddLog(LOGTYPE_WARNING, u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            case Log::LOGTYPEV_DEBUG   : g_Log->AddLog(LOGTYPE_DEBUG,   u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            default                    : g_Log->AddLog(LOGTYPE_FATAL  , u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
        }
    }

    if(nOutPort & OUTPORT_SCREEN){
    }

    if(nOutPort & OUTPORT_CONTROLBOARD){
        m_ControbBoard.AddLog(nLogType, szLog.c_str());
    }
}

bool ProcessRun::OnMap(uint32_t nMapID, int nX, int nY) const
{
    return (MapID() == nMapID) && m_Mir2xMapData.ValidC(nX, nY);
}

Creature *ProcessRun::RetrieveUID(uint64_t nUID)
{
    if(!nUID){
        return nullptr;
    }

    if(auto p = m_CreatureList.find(nUID); p != m_CreatureList.end()){
        if(p->second->Visible()){
            if(p->second->UID() != nUID){
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid creature record: %p, UID = %" PRIu64, p->second, p->second->UID());
                return nullptr;
            }
            return p->second.get();
        }
        m_CreatureList.erase(p);
    }
    return nullptr;
}

bool ProcessRun::LocateUID(uint64_t nUID, int *pX, int *pY)
{
    if(auto pCreature = RetrieveUID(nUID)){
        if(pX){ *pX = pCreature->X(); }
        if(pY){ *pY = pCreature->Y(); }
        return true;
    }
    return false;
}

bool ProcessRun::TrackAttack(bool bForce, uint64_t nUID)
{
    if(RetrieveUID(nUID)){
        if(bForce || GetMyHero()->StayIdle()){
            auto nEndX = GetMyHero()->CurrMotion().EndX;
            auto nEndY = GetMyHero()->CurrMotion().EndY;
            return GetMyHero()->EmplaceAction(ActionAttack(nEndX, nEndY, DC_PHY_PLAIN, SYS_DEFSPEED, nUID));
        }
    }
    return false;
}

uint32_t ProcessRun::GetFocusFaceKey()
{
    uint32_t nFaceKey = 0X02000000;
    if(auto nUID = FocusUID(FOCUS_MOUSE)){
        if(auto pCreature = RetrieveUID(nUID)){
            switch(pCreature->Type()){
                case CREATURE_PLAYER:
                    {
                        nFaceKey = 0X02000000;
                        break;
                    }
                case CREATURE_MONSTER:
                    {
                        auto nLookID = ((Monster*)(pCreature))->LookID();
                        if(nLookID >= 0){
                            nFaceKey = 0X01000000 + (nLookID - LID_MIN);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }

    return nFaceKey;
}

void ProcessRun::AddAscendStr(int nType, int nValue, int nX, int nY)
{
    m_AscendStrList.emplace_back(std::make_shared<AscendStr>(nType, nValue, nX, nY));
}

bool ProcessRun::GetUIDLocation(uint64_t nUID, bool bDrawLoc, int *pX, int *pY)
{
    if(auto pCreature = RetrieveUID(nUID)){
        if(bDrawLoc){
        }else{
            if(pX){ *pX = pCreature->X(); }
            if(pY){ *pY = pCreature->Y(); }
        }
        return true;
    }
    return false;
}

void ProcessRun::CenterMyHero()
{
    auto nMotion     = GetMyHero()->CurrMotion().Motion;
    auto nDirection  = GetMyHero()->CurrMotion().Direction;
    auto nX          = GetMyHero()->CurrMotion().X;
    auto nY          = GetMyHero()->CurrMotion().Y;
    auto nFrame      = GetMyHero()->CurrMotion().Frame;
    auto nFrameCount = GetMyHero()->MotionFrameCount(nMotion, nDirection);

    if(nFrameCount <= 0){
        throw std::runtime_error(str_fflprintf("Current hero has invalid frame count: %d", nFrameCount));
    }

    auto fnSetOff = [this, nX, nY, nDirection, nFrame, nFrameCount](int nStepLen)
    {
        auto nShowWindowW = g_SDLDevice->WindowW(false);
        auto nShowWindowH = g_SDLDevice->WindowH(false);

        switch(nStepLen){
            case 0:
                {
                    m_ViewX = nX * SYS_MAPGRIDXP - nShowWindowW / 2;
                    m_ViewY = nY * SYS_MAPGRIDYP - nShowWindowH / 2;
                    return;
                }
            case 1:
            case 2:
            case 3:
                {
                    int nDX = -1;
                    int nDY = -1;
                    PathFind::GetFrontLocation(&nDX, &nDY, 0, 0, nDirection, nStepLen);

                    int nOffX = nDX * SYS_MAPGRIDXP * (nFrame + 1) / nFrameCount;
                    int nOffY = nDY * SYS_MAPGRIDYP * (nFrame + 1) / nFrameCount;

                    m_ViewX = nX * SYS_MAPGRIDXP + nOffX - nShowWindowW / 2;
                    m_ViewY = nY * SYS_MAPGRIDYP + nOffY - nShowWindowH / 2;
                    return;
                }
            default:
                {
                    return;
                }
        }
    };

    switch(nMotion){
        case MOTION_WALK:
        case MOTION_ONHORSEWALK:
            {
                fnSetOff(1);
                break;
            }
        case MOTION_RUN:
            {
                fnSetOff(2);
                break;
            }
        case MOTION_ONHORSERUN:
            {
                fnSetOff(3);
                break;
            }
        default:
            {
                fnSetOff(0);
                break;
            }
    }
}

bool ProcessRun::RequestSpaceMove(uint32_t nMapID, int nX, int nY)
{
    if(auto pMapBin = g_MapBinDB->Retrieve(nMapID)){
        if(pMapBin->ValidC(nX, nY) && pMapBin->Cell(nX, nY).CanThrough()){
            CMReqestSpaceMove stCMRSM;
            stCMRSM.MapID = nMapID;
            stCMRSM.X     = nX;
            stCMRSM.Y     = nY;
            g_Client->Send(CM_REQUESTSPACEMOVE, stCMRSM);
            return true;
        }
    }
    return false;
}

void ProcessRun::ClearCreature()
{
    m_CreatureList.clear();
}

void ProcessRun::QueryCORecord(uint64_t nUID) const
{
    CMQueryCORecord stCMQCOR;
    std::memset(&stCMQCOR, 0, sizeof(stCMQCOR));

    stCMQCOR.AimUID = nUID;
    g_Client->Send(CM_QUERYCORECORD, stCMQCOR);
}

void ProcessRun::OnActionSpawn(uint64_t nUID, const ActionNode &rstAction)
{
    condcheck(nUID);
    condcheck(rstAction.Action == ACTION_SPAWN);

    if(UIDFunc::GetUIDType(nUID) != UID_MON){
        QueryCORecord(nUID);
        return;
    }

    switch(UIDFunc::GetMonsterID(nUID)){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                // TODO how about make it as an action of skeleton
                // then we don't need to define the callback of a done magic

                AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"使用魔法: 召唤骷髅"), 
                m_IndepMagicList.emplace_back(std::make_shared<IndepMagic>
                (
                    rstAction.ActionParam,
                    DBCOM_MAGICID(u8"召唤骷髅"), 
                    0,
                    EGS_START,
                    rstAction.Direction,
                    rstAction.X,
                    rstAction.Y,
                    rstAction.X,
                    rstAction.Y,
                    nUID
                ));

                m_UIDPending.insert(nUID);
                m_IndepMagicList.back()->AddFunc([this, nUID, rstAction, pMagic = m_IndepMagicList.back()]() -> bool
                {
                    // if(!pMagic->Done()){
                    //     return false;
                    // }

                    if(pMagic->Frame() < 10){
                        return false;
                    }

                    ActionStand stActionStand
                    {
                        rstAction.X,
                        rstAction.Y,
                        DIR_DOWNLEFT,
                    };

                    if(auto pMonster = Monster::CreateMonster(nUID, this, stActionStand)){
                        m_CreatureList[nUID].reset(pMonster);
                    }

                    m_UIDPending.erase(nUID);
                    QueryCORecord(nUID);
                    return true;
                });

                return;
            }
        default:
            {
                ActionStand stActionStand
                {
                    rstAction.X,
                    rstAction.Y,
                    PathFind::ValidDir(rstAction.Direction) ? rstAction.Direction : DIR_UP,
                };

                if(auto pMonster = Monster::CreateMonster(nUID, this, stActionStand)){
                    m_CreatureList[nUID].reset(pMonster);
                }

                QueryCORecord(nUID);
                return;
            }
    }
}

Widget *ProcessRun::GetWidget(const char *szWidgetName)
{
    if(szWidgetName){
        if(!std::strcmp(szWidgetName, "InventoryBoard")){ return &m_InventoryBoard; }
    }
    return nullptr;
}
