/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46
 *  Last Modified: 10/11/2017 23:30:17
 *
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
#include "mapbindbn.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "clientenv.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "clientluamodule.hpp"

ProcessRun::ProcessRun()
    : Process()
    , m_MapID(0)
    , m_Mir2xMapData()
    , m_MyHero(nullptr)
    , m_FocusUIDV()
    , m_ViewX(0)
    , m_ViewY(0)
    , m_RollMap(false)
    , m_LuaModule(this, OUTPORT_CONTROLBOARD)
    , m_ControbBoard(
            0,                  // x
            []() -> int         // y
            {
                extern SDLDevice *g_SDLDevice;
                return g_SDLDevice->WindowH(false) - 134;
            }(),
            []() -> int         // w
            {
                extern SDLDevice *g_SDLDevice;
                return g_SDLDevice->WindowW(false);
            }(),
            this,               // self-bind
            nullptr,            // independent widget
            false)              // 
    , m_InventoryBoard(0, 0, this)
    , m_GroundItemList()
    , m_CreatureRecord()
    , m_AttackUIDX(-1)
    , m_AttackUIDY(-1)
    , m_PointerPixlInfo(0, 0, "", 0, 15, 0, {0XFF, 0X00, 0X00, 0X00})
    , m_PointerTileInfo(0, 0, "", 0, 15, 0, {0XFF, 0X00, 0X00, 0X00})
    , m_AscendStrRecord()
{
    m_FocusUIDV.fill(0);
    RegisterUserCommand();
}

void ProcessRun::ScrollMap()
{
    extern SDLDevice *g_SDLDevice;
    auto nShowWindowW = g_SDLDevice->WindowW(false);
    auto nShowWindowH = g_SDLDevice->WindowH(false);

    if(m_MyHero){
        int nViewX = m_MyHero->X() * SYS_MAPGRIDXP - nShowWindowW / 2;
        int nViewY = m_MyHero->Y() * SYS_MAPGRIDYP - nShowWindowH / 2;

        int nDViewX = nViewX - m_ViewX;
        int nDViewY = nViewY - m_ViewY;

        if(m_RollMap
                ||  (std::abs(nDViewX) > nShowWindowW / 4)
                ||  (std::abs(nDViewY) > nShowWindowH / 4)){

            m_RollMap = true;

            m_ViewX += (int)(std::lround(std::copysign(std::min<int>(3, std::abs(nDViewX)), nDViewX)));
            m_ViewY += (int)(std::lround(std::copysign(std::min<int>(2, std::abs(nDViewY)), nDViewY)));

            m_ViewX = std::max<int>(0, m_ViewX);
            m_ViewY = std::max<int>(0, m_ViewY);
        }

        // stop rolling the map when
        //   1. the hero is at the required position
        //   2. the hero is not moving
        if((nDViewX == 0) && (nDViewY == 0) && !m_MyHero->Moving()){ m_RollMap = false; }
    }
}

void ProcessRun::Update(double fUpdateTime)
{
    ScrollMap();
    m_ControbBoard.Update(fUpdateTime);

    for(auto pRecord = m_CreatureRecord.begin(); pRecord != m_CreatureRecord.end();){
        if(true
                && pRecord->second
                && pRecord->second->Active()){
            pRecord->second->Update(fUpdateTime);
            ++pRecord;
        }else{
            delete pRecord->second;
            pRecord = m_CreatureRecord.erase(pRecord);
        }
    }

    for(size_t nIndex = 0; nIndex < m_IndepMagicList.size();){
        m_IndepMagicList[nIndex]->Update(fUpdateTime);
        if(m_IndepMagicList[nIndex]->Done()){
            std::swap(m_IndepMagicList[nIndex], m_IndepMagicList.back());
            m_IndepMagicList.pop_back();
        }else{
            nIndex++;
        }
    }

    for(auto pRecord = m_AscendStrRecord.begin(); pRecord != m_AscendStrRecord.end();){
        if((*pRecord)->Ratio() < 1.0){
            (*pRecord)->Update(fUpdateTime);
            ++pRecord;
        }else{
            delete (*pRecord);
            pRecord = m_AscendStrRecord.erase(pRecord);
        }
    }

    if(auto pCreature = RetrieveUID(m_FocusUIDV[FOCUS_ATTACK])){
        if(pCreature->StayDead()){
            m_FocusUIDV[FOCUS_ATTACK] = 0;
        }else{
            auto nX = pCreature->X();
            auto nY = pCreature->Y();

            bool bForce = false;
            if(false
                    || m_AttackUIDX != nX
                    || m_AttackUIDY != nY){

                bForce = true;
                m_AttackUIDX = nX;
                m_AttackUIDY = nY;
            }
            TrackAttack(bForce, m_FocusUIDV[FOCUS_ATTACK]);
        }
    }else{
        m_FocusUIDV[FOCUS_ATTACK] = 0;
    }
}

uint32_t ProcessRun::FocusUID(int nFocusType)
{
    if(nFocusType < (int)(m_FocusUIDV.size())){
        switch(nFocusType){
            case FOCUS_NONE:
                {
                    return 0;
                }
            case FOCUS_MOUSE:
                {
                    // use the cached mouse focus first
                    // if can't get it then scan the whole creature list

                    auto fnCheckFocus = [this](uint32_t nUID, int nX, int nY) -> bool
                    {
                        if(auto pCreature = RetrieveUID(nUID)){
                            if(pCreature != m_MyHero){
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

                    if(fnCheckFocus(m_FocusUIDV[FOCUS_MOUSE], nCheckPointX, nCheckPointY)){
                        return m_FocusUIDV[FOCUS_MOUSE];
                    }

                    Creature *pFocus = nullptr;
                    for(auto pRecord: m_CreatureRecord){
                        if(fnCheckFocus(pRecord.second->UID(), nCheckPointX, nCheckPointY)){
                            if(false
                                    || !pFocus
                                    ||  pFocus->Y() < pRecord.second->Y()){
                                // 1. currently we have no candidate yet
                                // 2. we have candidate but it's not at more front location
                                pFocus = pRecord.second;
                            }
                        }
                    }

                    m_FocusUIDV[FOCUS_MOUSE] = pFocus ? pFocus->UID() : 0;
                    return m_FocusUIDV[FOCUS_MOUSE];
                }
            default:
                {
                    return m_FocusUIDV[nFocusType];
                }
        }
    }
    
    return 0;
}

void ProcessRun::Draw()
{
    extern PNGTexDBN *g_MapDBN;
    extern SDLDevice *g_SDLDevice;
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
                        if(auto pTexture = g_MapDBN->Retrieve(rstTile.Image())){
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
                            if(auto pTexture = g_MapDBN->Retrieve(nImage)){
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

        extern ClientEnv *g_ClientEnv;
        if(g_ClientEnv->MIR2X_DEBUG_SHOW_MAP_GRID){
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
                for(auto pCreature: m_CreatureRecord){
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
                for(auto &rstGI: m_GroundItemList){
                    if(true
                            && rstGI.ID
                            && rstGI.X == nX
                            && rstGI.Y == nY){

                        // draw ground item
                        // only need information of item record

                        if(auto &rstIR = DBCOM_ITEMRECORD(rstGI.ID)){
                            if(rstIR.PkgGfxID >= 0){
                                extern SDLDevice *g_SDLDevice;
                                extern PNGTexDBN *g_GroundItemDBN;
                                if(auto pTexture = g_GroundItemDBN->Retrieve(rstIR.PkgGfxID)){
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

                                        g_SDLDevice->DrawTexture(pTexture, nXt, nYt);

                                        if(bChoose){
                                            LabelBoard stItemName(0, 0, rstIR.Name, 1, 12, 0, {0XFF, 0XFF, 0X00, 0X00});
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
                            if(auto pTexture = g_MapDBN->Retrieve(nImage)){
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
                for(auto pCreature: m_CreatureRecord){
                    if(true
                            &&  (pCreature.second)
                            &&  (pCreature.second->X() == nX)
                            &&  (pCreature.second->Y() == nY)
                            && !(pCreature.second->StayDead())){

                        extern ClientEnv *g_ClientEnv;
                        if(g_ClientEnv->MIR2X_DEBUG_SHOW_CREATURE_COVER){
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
        static double fRatio = 0.00;

        fRatio += 0.05;
        if(fRatio >= 2.50){
            fRatio = 0.00;
        }else if(fRatio >= 1.00){
            // do nothing
            // hide the star to avoid blinking too much
        }else{
            for(int nY = nY0; nY <= nY1; ++nY){
                for(int nX = nX0; nX <= nX1; ++nX){

                    bool bShowStar = false;
                    for(auto &rstGI: m_GroundItemList){
                        if(true
                                && rstGI.ID
                                && rstGI.X == nX
                                && rstGI.Y == nY){

                            bShowStar = true;
                            break;
                        }
                    }

                    if(bShowStar){
                        extern SDLDevice *g_SDLDevice;
                        extern PNGTexDBN *g_GroundItemDBN;

                        if(auto pTexture = g_GroundItemDBN->Retrieve(0X01000000)){
                            int nW = -1;
                            int nH = -1;
                            if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH)){
                                if(auto nLt = (int)(std::lround(fRatio * nW / 2.50))){
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
                                            std::lround(fRatio * 360.0));
                                }
                            }
                        }
                    }
                }
            }
        }

        // draw magics
        for(auto pMagic: m_IndepMagicList){
            if(true
                    &&  pMagic
                    && !pMagic->Done()){

                pMagic->Draw(m_ViewX, m_ViewY);
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

    for(auto pRecord: m_AscendStrRecord){
        pRecord->Draw(m_ViewX, m_ViewY);
    }

    m_InventoryBoard.Draw();

    m_ControbBoard.Draw();

    // draw cursor location information on top-left
    extern ClientEnv *g_ClientEnv;
    if(g_ClientEnv->MIR2X_DEBUG_SHOW_LOCATION){
        g_SDLDevice->PushColor(0, 0, 0, 230);
        g_SDLDevice->PushBlendMode(SDL_BLENDMODE_BLEND);
        g_SDLDevice->FillRectangle(0, 0, 200, 60);
        g_SDLDevice->PopBlendMode();
        g_SDLDevice->PopColor();

        int nPointX = -1;
        int nPointY = -1;
        SDL_GetMouseState(&nPointX, &nPointY);

        m_PointerPixlInfo.SetText("Pix_Loc: %3d, %3d", nPointX, nPointY);
        m_PointerTileInfo.SetText("Til_Loc: %3d, %3d", (nPointX + m_ViewX) / SYS_MAPGRIDXP, (nPointY + m_ViewY) / SYS_MAPGRIDYP);

        m_PointerTileInfo.DrawEx(10, 10, 0, 0, m_PointerTileInfo.W(), m_PointerTileInfo.H());
        m_PointerPixlInfo.DrawEx(10, 30, 0, 0, m_PointerPixlInfo.W(), m_PointerPixlInfo.H());
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
                switch(rstEvent.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDV[FOCUS_ATTACK] = nUID;
                                TrackAttack(true, nUID);
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

                            m_FocusUIDV[FOCUS_ATTACK] = 0;
                            m_FocusUIDV[FOCUS_FOLLOW] = 0;

                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDV[FOCUS_FOLLOW] = nUID;
                            }else{
                                int nX = -1;
                                int nY = -1;
                                if(true
                                        && LocatePoint(rstEvent.button.x, rstEvent.button.y, &nX, &nY)
                                        && LDistance2(m_MyHero->CurrMotion().EndX, m_MyHero->CurrMotion().EndY, nX, nY)){

                                    // we get a valid dst to go
                                    // provide myHero with new move action command

                                    m_MyHero->ParseNewAction({
                                            ACTION_MOVE,
                                            m_MyHero->OnHorse() ? 1 : 0,
                                            100,
                                            DIR_NONE,
                                            m_MyHero->CurrMotion().EndX,
                                            m_MyHero->CurrMotion().EndY,
                                            nX,
                                            nY,
                                            MapID()}, false);
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
                            extern SDLDevice *g_SDLDevice;
                            m_ViewX = std::max<int>(0, m_MyHero->X() - g_SDLDevice->WindowW(false) / 2 / SYS_MAPGRIDXP) * SYS_MAPGRIDXP;
                            m_ViewY = std::max<int>(0, m_MyHero->Y() - g_SDLDevice->WindowH(false) / 2 / SYS_MAPGRIDYP) * SYS_MAPGRIDYP;
                            break;
                        }
                    case SDLK_t:
                        {
                            if(auto nMouseFocusUID = FocusUID(FOCUS_MOUSE)){
                                m_FocusUIDV[FOCUS_MAGIC] = nMouseFocusUID;
                            }else{
                                if(!RetrieveUID(m_FocusUIDV[FOCUS_MAGIC])){
                                    m_FocusUIDV[FOCUS_MAGIC] = 0;
                                }
                            }

                            m_MyHero->ParseNewAction({
                                    ACTION_SPELL,
                                    DBCOM_MAGICID(u8"雷电术"),
                                    100,
                                    m_MyHero->CurrMotion().Direction,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    FocusUID(FOCUS_MAGIC),
                                    MapID()}, false);
                            break;
                        }
                    case SDLK_y:
                        {
                            m_MyHero->ParseNewAction({
                                    ACTION_SPELL,
                                    DBCOM_MAGICID(u8"魔法盾"),
                                    100,
                                    m_MyHero->CurrMotion().Direction,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    m_MyHero->UID(),
                                    MapID()}, false);
                            break;
                        }
                    case SDLK_u:
                        {
                            m_MyHero->ParseNewAction({
                                    ACTION_SPELL,
                                    DBCOM_MAGICID(u8"召唤骷髅"),
                                    100,
                                    m_MyHero->CurrMotion().Direction,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    m_MyHero->CurrMotion().EndX,
                                    m_MyHero->CurrMotion().EndY,
                                    m_MyHero->UID(),
                                    MapID()}, false);
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
        m_MapID = nMapID;
        extern MapBinDBN *g_MapBinDBN;
        if(auto pMapBin = g_MapBinDBN->Retrieve(nMapID)){
            m_Mir2xMapData = *pMapBin;
            return 0;
        }
    }

    m_MapID = 0;
    return -1;
}

bool ProcessRun::CanMove(bool bCheckCreature, int nX, int nY)
{
    if(true
            && m_Mir2xMapData.Valid()
            && m_Mir2xMapData.ValidC(nX, nY)
            && m_Mir2xMapData.Cell(nX, nY).CanThrough()){

        if(bCheckCreature){
            for(auto pCreature: m_CreatureRecord){
                if(true
                        && (pCreature.second)
                        && (pCreature.second->X() == nX)
                        && (pCreature.second->Y() == nY)){
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

bool ProcessRun::CanMove(bool bCheckCreature, int nX0, int nY0, int nX1, int nY1)
{
    int nMaxIndex = -1;
    switch(LDistance2(nX0, nY0, nX1, nY1)){
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
                return false;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        if(!CanMove(bCheckCreature, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
            return false;
        }
    }
    return true;
}

double ProcessRun::MoveCost(bool bCheckCreature, int nX0, int nY0, int nX1, int nY1)
{
    int nMaxIndex = -1;
    switch(LDistance2(nX0, nY0, nX1, nY1)){
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
                return false;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fMoveCost = 0.0;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        auto nCurrX = nX0 + nDX * nIndex;
        auto nCurrY = nY0 + nDY * nIndex;

        // validate crrent grid first
        // for client's part we allow motion parse with invalid grids

        // main purpose of this function:
        // if CanMove(checkCreature = true, srcLoc, dstLoc) failed
        // we need to give different cost for ground issue / creature occupation issue

        if(CanMove(false, nCurrX, nCurrY)){
            if(bCheckCreature){
                fMoveCost += (CanMove(true, nCurrX, nCurrY) ? 1.00 : 100.00);
            }else{
                // not check the creatures
                // then any valid grids get unique cost
                fMoveCost += 1.00;
            }
        }else{
            // since we allow motion into invalid grids
            // we accumulate the ``infinite" here if current grid is invalid
            fMoveCost += 10000.00;
        }
    }
    return fMoveCost;
}

bool ProcessRun::LocatePoint(int nPX, int nPY, int *pX, int *pY)
{
    if(pX){ *pX = (nPX + m_ViewX) / SYS_MAPGRIDXP; }
    if(pY){ *pY = (nPY + m_ViewY) / SYS_MAPGRIDYP; }

    return true;
}

bool ProcessRun::LuaCommand(const char *szLuaCommand)
{
    if(szLuaCommand){
        auto stCallResult = m_LuaModule.script(szLuaCommand, [](lua_State *, sol::protected_function_result stResult){
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

            extern Log *g_Log;
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
                        if(pEntry->Callback){ pEntry->Callback(stvToken); }
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
    for(auto pRecord = m_CreatureRecord.begin(); pRecord != m_CreatureRecord.end();){
        if(true
                && pRecord->second
                && pRecord->second->Active()){
            switch(pRecord->second->Type()){
                case CREATURE_PLAYER:
                    {
                        stRetV.push_back(pRecord->second->UID());
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
            ++pRecord;
        }else{
            delete pRecord->second;
            pRecord = m_CreatureRecord.erase(pRecord);
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
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", "@make 物品名字");
                    return 1;
                }
            case 1 + 1:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", "获得%s", rstvParam[1].c_str());
                    return 0;
                }
            default:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", "Invalid argument for @make");
                    return 1;
                }
        }
    };
    m_UserCommandGroup.emplace_back("makeItem", fnMakeItem);
    return true;
}

bool ProcessRun::RegisterLuaExport(ClientLuaModule *pModule, int nOutPort)
{
    if(pModule){

        // initialization before registration
        pModule->script(R"()");

        // register command exitClient
        // exit client and free all related resource
        pModule->set_function("exitClient", []()
        {
            std::exit(0);
        });

        // register command exit
        pModule->set_function("exit", []()
        {
            // reserve this command
            // don't find what to exit here
            std::exit(0);
        });

        // register command sleep
        // sleep current thread and return after the specified ms
        // can use posix.sleep(ms), but here use std::this_thread::sleep_for(x)
        pModule->set_function("sleep", [](int nSleepMS)
        {
            if(nSleepMS > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
            }
        });

        // register command printLine
        // print one line to the out port allocated for the lua module
        // won't add message to log system, use addLog instead if needed
        pModule->set_function("printLine", [this, nOutPort](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo)
        {
            // use sol::object to accept arguments
            // otherwise for follow code it throws exception for type unmatch
            //      lua["f"] = [](int a){ return a; };
            //      lua.script("print f(\"hello world\")")
            // program crashes with exception.what() : expecting int, string provided
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
        pModule->set_function("addLog", [this, nOutPort](sol::object stLogType, sol::object stLogInfo){
            if(true
                    && stLogType.is<int>()
                    && stLogInfo.is<std::string>()){
                extern Log *g_Log;
                switch(stLogType.as<int>()){
                    case 0  : g_Log->AddLog(LOGTYPE_INFO   , "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 1  : g_Log->AddLog(LOGTYPE_WARNING, "%s", stLogInfo.as<std::string>().c_str()); return;
                    default : g_Log->AddLog(LOGTYPE_FATAL  , "%s", stLogInfo.as<std::string>().c_str()); return;
                }
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "%s", "addLog(LogType: int, LogInfo: string)");
        });

        // register command playerList
        // get a list for all active maps
        // return a table (userData) to lua for ipairs() check
        pModule->set_function("playerList", [this](sol::this_state stThisLua)
        {
            return sol::make_object(sol::state_view(stThisLua), GetPlayerList());
        });

        // register command moveTo(x, y)
        // wait for server to move player if possible
        pModule->set_function("moveTo", [this, nOutPort](sol::object stLocX, sol::object stLocY)
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
        pModule->script(R"#(
            function listPlayerInfo ()
                for k, v in ipairs(playerList())
                do
                    printLine(0, "> ", tostring(v))
                end
            end
        )#");

        // register command ``help"
        // part-1: divide into two parts, part-1 create the table for help
        pModule->script(R"#(
            helpInfoTable = {
                wear     = "put on different dress",
                moveTo   = "move to other position on current map",
                randMove = "random move on current map"
            }
        )#");

        // part-2: make up the function to print the table entry
        pModule->script(R"#(
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
        pModule->set_function("myHero_dress", [this](int nDress)
        {
            if(nDress >= 0){
                m_MyHero->Dress((uint32_t)(nDress));
            }
        });

        // register command ``myHero.xxx"
        // I need to insert a table to micmic a instance myHero in the future
        pModule->set_function("myHero_weapon", [this](int nWeapon)
        {
            if(nWeapon >= 0){
                m_MyHero->Weapon((uint32_t)(nWeapon));
            }
        });


        // registration done
        return true;
    }
    return false;
}

bool ProcessRun::AddOPLog(int nOutPort, int nLogType, const char *szPrompt, const char *szLogFormat, ...)
{
    auto fnRecordLog = [this](int nOutPort, int nLogType, const char *szPrompt, const char *szLogInfo)
    {
        if(nOutPort & OUTPORT_LOG){
            extern Log *g_Log;
            switch(nLogType){
                case 0  : g_Log->AddLog(LOGTYPE_INFO   , "%s%s", szPrompt ? szPrompt : "", szLogInfo); break;
                case 1  : g_Log->AddLog(LOGTYPE_WARNING, "%s%s", szPrompt ? szPrompt : "", szLogInfo); break;
                default : g_Log->AddLog(LOGTYPE_FATAL  , "%s%s", szPrompt ? szPrompt : "", szLogInfo); break;
            }
        }

        if(nOutPort & OUTPORT_SCREEN){
        }

        if(nOutPort & OUTPORT_CONTROLBOARD){
            m_ControbBoard.AddLog(nLogType, szLogInfo);
        }
    };

    int nLogSize = 0;

    // 1. try static buffer
    //    give an enough size so we can hopefully stop here
    {
        char szSBuf[1024];

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(szSBuf, std::extent<decltype(szSBuf)>::value, szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < std::extent<decltype(szSBuf)>::value){
                fnRecordLog(nOutPort, nLogType, szPrompt, szSBuf);
                return true;
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            fnRecordLog(nOutPort, 0, "", (std::string("Parse log info failed: ") + szLogFormat).c_str());
            return false;
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory
    while(true){
        std::vector<char> szDBuf(nLogSize + 1 + 64);

        va_list ap;
        va_start(ap, szLogFormat);
        nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szLogFormat, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < szDBuf.size()){
                fnRecordLog(nOutPort, nLogType, szPrompt, &(szDBuf[0]));
                return true;
            }else{
                szDBuf.resize(nLogSize + 1 + 64);
            }
        }else{
            fnRecordLog(nOutPort, 0, "", (std::string("Parse log info failed: ") + szLogFormat).c_str());
            return false;
        }
    }
}

bool ProcessRun::OnMap(uint32_t nMapID, int nX, int nY) const
{
    return (MapID() == nMapID) && m_Mir2xMapData.ValidC(nX, nY);
}

Creature *ProcessRun::RetrieveUID(uint32_t nUID)
{
    if(nUID){
        auto pRecord = m_CreatureRecord.find(nUID);
        if(pRecord != m_CreatureRecord.end()){
            if(true
                    && pRecord->second
                    && pRecord->second->Active()
                    && pRecord->second->UID() == nUID){
                // here return the naked pointer
                // OK since we force to use single thread
                return pRecord->second;
            }

            // invalid record found
            // delete it as garbage collector
            delete pRecord->second;
            m_CreatureRecord.erase(pRecord);
        }
    }
    return nullptr;
}

bool ProcessRun::LocateUID(uint32_t nUID, int *pX, int *pY)
{
    if(auto pCreature = RetrieveUID(nUID)){
        if(pX){ *pX = pCreature->X(); }
        if(pY){ *pY = pCreature->Y(); }
        return true;
    }
    return false;
}

bool ProcessRun::TrackAttack(bool bForce, uint32_t nUID)
{
    if(nUID){
        if(auto pCreature = RetrieveUID(nUID)){
            if(bForce || m_MyHero->StayIdle()){
                return m_MyHero->ParseNewAction({
                        ACTION_ATTACK,
                        DC_PHY_PLAIN,
                        100,
                        DIR_NONE,
                        m_MyHero->CurrMotion().EndX,
                        m_MyHero->CurrMotion().EndY,
                        pCreature->X(),
                        pCreature->Y(),
                        MapID()}, false);
            }
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
    m_AscendStrRecord.emplace_back(new AscendStr(nType, nValue, nX, nY));
}

bool ProcessRun::GetUIDLocation(uint32_t nUID, bool bDrawLoc, int *pX, int *pY)
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
    if(m_MyHero){
        auto nMotion     = m_MyHero->CurrMotion().Motion;
        auto nDirection  = m_MyHero->CurrMotion().Direction;
        auto nX          = m_MyHero->CurrMotion().X;
        auto nY          = m_MyHero->CurrMotion().Y;
        auto nFrame      = m_MyHero->CurrMotion().Frame;
        auto nFrameCount = m_MyHero->MotionFrameCount(nMotion, nDirection);

        if(nFrameCount > 0){
            auto fnSetOff = [this, nX, nY, nDirection, nFrame, nFrameCount](int nStepLen)
            {
                extern SDLDevice *g_SDLDevice;
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
                            if(PathFind::GetFrontLocation(&nDX, &nDY, 0, 0, nDirection, nStepLen)){
                                int nOffX = nDX * SYS_MAPGRIDXP * (nFrame + 1) / nFrameCount;
                                int nOffY = nDY * SYS_MAPGRIDYP * (nFrame + 1) / nFrameCount;

                                m_ViewX = nX * SYS_MAPGRIDXP + nOffX - nShowWindowW / 2;
                                m_ViewY = nY * SYS_MAPGRIDYP + nOffY - nShowWindowH / 2;
                            }
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
    }
}

bool ProcessRun::RequestSpaceMove(uint32_t nMapID, int nX, int nY)
{
    extern MapBinDBN *g_MapBinDBN;
    if(auto pMapBin = g_MapBinDBN->Retrieve(nMapID)){
        if(pMapBin->ValidC(nX, nY) && pMapBin->Cell(nX, nY).CanThrough()){
            extern Game *g_Game;
            CMReqestSpaceMove stCMRSM;
            stCMRSM.MapID = nMapID;
            stCMRSM.X     = nX;
            stCMRSM.Y     = nY;
            g_Game->Send(CM_REQUESTSPACEMOVE, stCMRSM);
            return true;
        }
    }
    return false;
}

void ProcessRun::ClearCreature()
{
    m_MyHero = nullptr;

    for(auto pRecord: m_CreatureRecord){
        delete pRecord.second;
    }
    m_CreatureRecord.clear();
}
