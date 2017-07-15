/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46
 *  Last Modified: 07/14/2017 22:14:30
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

#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "clientenv.hpp"
#include "processrun.hpp"
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
    , m_LuaModule(this, 0)
    , m_ControbBoard(0, 0, nullptr, false)
    , m_CreatureRecord()
    , m_AttackUIDX(-1)
    , m_AttackUIDY(-1)
    , m_PointerPixlInfo(0, 0, "", 0, 15, 0, {0XFF, 0X00, 0X00, 0X00})
    , m_PointerTileInfo(0, 0, "", 0, 15, 0, {0XFF, 0X00, 0X00, 0X00})
    , m_SystemBoard(190, 490, false, false, false, -1, 0, 0, 0, 15, 0, {128, 128, 128, 128})
{
    m_FocusUIDV.fill(0);
    m_ControbBoard.Bind(this);
}

void ProcessRun::Update(double fTime)
{
    if(m_MyHero){
        extern SDLDevice *g_SDLDevice;
        int nViewX = m_MyHero->X() * SYS_MAPGRIDXP - g_SDLDevice->WindowW(false) / 2;
        int nViewY = m_MyHero->Y() * SYS_MAPGRIDYP - g_SDLDevice->WindowH(false) / 2;

        int nDViewX = nViewX - m_ViewX;
        int nDViewY = nViewY - m_ViewY;

        if(m_RollMap
                ||  (std::abs(nDViewX) > g_SDLDevice->WindowW(false) / 4)
                ||  (std::abs(nDViewY) > g_SDLDevice->WindowH(false) / 4)){

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

    for(auto pRecord = m_CreatureRecord.begin(); pRecord != m_CreatureRecord.end();){
        if(true
                && pRecord->second
                && pRecord->second->Active()){
            pRecord->second->Update();
            ++pRecord;
        }else{
            delete pRecord->second;
            pRecord = m_CreatureRecord.erase(pRecord);
        }
    }

    // re-calculate the focused UID
    // even mouse location doesn't change the creatures can move
    // we need to always highlight the creature under the mouse pointer
    FocusUID(FOCUS_MOUSE);

    if(m_FocusUIDV[FOCUS_ATTACK]){
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

    m_SystemBoard.Update(fTime);
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
                    // clean the last mouse-focused one
                    // each time we have to re-calculate current focus
                    if(auto pCreature = RetrieveUID(m_FocusUIDV[FOCUS_MOUSE])){
                        pCreature->Focus(FOCUS_MOUSE, false);
                    }
                    m_FocusUIDV[FOCUS_MOUSE] = 0;

                    // re-calculate the mouse focus
                    // need to do it outside of creatures since only one can be selected
                    int nPointX = -1;
                    int nPointY = -1;
                    SDL_GetMouseState(&nPointX, &nPointY);

                    Creature *pFocus = nullptr;
                    for(auto pRecord: m_CreatureRecord){
                        if(true
                                && pRecord.second != m_MyHero
                                && pRecord.second->CanFocus(m_ViewX + nPointX, m_ViewY + nPointY)){
                            if(false
                                    || !pFocus
                                    ||  pFocus->Y() < pRecord.second->Y()){
                                // 1. currently we have no candidate yet
                                // 2. we have candidate but it's not at more front location
                                pFocus = pRecord.second;
                            }
                        }
                    }

                    if(pFocus){
                        pFocus->Focus(FOCUS_MOUSE, true);
                        m_FocusUIDV[FOCUS_MOUSE] = pFocus->UID();
                    }
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
        int nX0 = (m_ViewX - 2 * SYS_MAPGRIDXP - SYS_OBJMAXW) / SYS_MAPGRIDXP;
        int nY0 = (m_ViewY - 2 * SYS_MAPGRIDYP - SYS_OBJMAXH) / SYS_MAPGRIDYP;
        int nX1 = (m_ViewX + 2 * SYS_MAPGRIDXP + SYS_OBJMAXW + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
        int nY1 = (m_ViewY + 2 * SYS_MAPGRIDYP + SYS_OBJMAXH + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

        // tiles
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY) && !(nX % 2) && !(nY % 2)){
                    auto nParam = m_Mir2xMapData.Tile(nX, nY).Param;
                    if(nParam & 0X80000000){
                        if(auto pTexture = g_MapDBN->Retrieve(nParam & 0X00FFFFFF)){
                            g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, nY * SYS_MAPGRIDYP - m_ViewY);
                        }
                    }
                }
            }
        }

        // ground objects
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY) && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)){
                    // for obj-0
                    {
                        auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                        if(nParam & 0X80000000){
                            auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                            if(nObjParam & ((uint32_t)(1) << 22)){
                                if(auto pTexture = g_MapDBN->Retrieve(nParam & 0X00FFFFFF)){
                                    int nH = 0;
                                    if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                        g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                    }
                                }
                            }
                        }
                    }

                    // for obj-1
                    {
                        auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                        if(nParam & 0X80000000){
                            auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                            if(nObjParam & ((uint32_t)(1) << 6)){
                                if(auto pTexture = g_MapDBN->Retrieve(nParam & 0X00FFFFFF)){
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
        }

        extern ClientEnv *g_ClientEnv;
        if(g_ClientEnv->MIR2X_DEBUG_SHOW_MAP_GRID){
            int nX0 = m_ViewX / SYS_MAPGRIDXP;
            int nY0 = m_ViewY / SYS_MAPGRIDYP;

            int nX1 = (m_ViewX + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
            int nY1 = (m_ViewY + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

            g_SDLDevice->PushColor(0, 255, 0, 128);
            for(int nX = nX0; nX <= nX1; ++nX){
                g_SDLDevice->DrawLine(nX * SYS_MAPGRIDXP - m_ViewX, 0, nX * SYS_MAPGRIDXP - m_ViewX, g_SDLDevice->WindowH(false));
            }
            for(int nY = nY0; nY <= nY1; ++nY){
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
                        pCreature.second->Draw(m_ViewX, m_ViewY);
                    }
                }
            }
        }

        // over-ground objects
        for(int nY = nY0; nY <= nY1; ++nY){
            for(int nX = nX0; nX <= nX1; ++nX){
                if(m_Mir2xMapData.ValidC(nX, nY) && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)){
                    // for obj-0
                    {
                        auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                        if(nParam & 0X80000000){
                            auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                            if(!(nObjParam & ((uint32_t)(1) << 22))){
                                if(auto pTexture = g_MapDBN->Retrieve(nParam & 0X00FFFFFF)){
                                    int nH = 0;
                                    if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                        g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                    }
                                }
                            }
                        }
                    }

                    // for obj-1
                    {
                        auto nParam = m_Mir2xMapData.Cell(nX, nY).Obj[0].Param;
                        if(nParam & 0X80000000){
                            auto nObjParam = m_Mir2xMapData.Cell(nX, nY).ObjParam;
                            if(!(nObjParam & ((uint32_t)(1) << 6))){
                                if(auto pTexture = g_MapDBN->Retrieve(nParam & 0X00FFFFFF)){
                                    int nH = 0;
                                    if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                        g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                    }
                                }
                            }
                        }
                    }

                    extern ClientEnv *g_ClientEnv;
                    if(g_ClientEnv->MIR2X_DEBUG_SHOW_MAP_GRID){
                        if(m_Mir2xMapData.Cell(nX, nY).Param & 0X00800000){
                            g_SDLDevice->PushColor(255, 0, 0, 128);
                            int nX0 = nX * SYS_MAPGRIDXP - m_ViewX;
                            int nY0 = nY * SYS_MAPGRIDYP - m_ViewY;
                            int nX1 = (nX + 1) * SYS_MAPGRIDXP - m_ViewX;
                            int nY1 = (nY + 1) * SYS_MAPGRIDYP - m_ViewY;
                            g_SDLDevice->DrawLine(nX0, nY0, nX1, nY0);
                            g_SDLDevice->DrawLine(nX1, nY0, nX1, nY1);
                            g_SDLDevice->DrawLine(nX1, nY1, nX0, nY1);
                            g_SDLDevice->DrawLine(nX0, nY1, nX0, nY0);
                            g_SDLDevice->DrawLine(nX0, nY0, nX1, nY1);
                            g_SDLDevice->DrawLine(nX1, nY0, nX0, nY1);
                            g_SDLDevice->PopColor();
                        }
                    }
                }

                // draw actors
                {
                    for(auto pCreature: m_CreatureRecord){
                        if(true
                                &&  (pCreature.second)
                                &&  (pCreature.second->X() == nX)
                                &&  (pCreature.second->Y() == nY)
                                && !(pCreature.second->StayDead())){
                            extern ClientEnv *g_ClientEnv;
                            if(g_ClientEnv->MIR2X_DEBUG_SHOW_CREATURE_COVER){
                                g_SDLDevice->PushColor(0, 0, 255, 30);
                                g_SDLDevice->FillRectangle(nX * SYS_MAPGRIDXP - m_ViewX, nY * SYS_MAPGRIDYP - m_ViewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                                g_SDLDevice->PopColor();
                            }
                            pCreature.second->Draw(m_ViewX, m_ViewY);
                        }
                    }
                }
            }
        }
    }

    // draw some GUI here instead of putting in ControlBoard
    // for all widget without state, I prefer using the texture directly

    // draw the creature face
    {
        extern SDLDevice *g_SDLDevice;
        extern PNGTexDBN *g_ProgUseDBN;

        g_SDLDevice->PushColor(0, 0, 0, 0);
        g_SDLDevice->FillRectangle(530, 485, 95, 105);
        g_SDLDevice->PopColor();

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
                                nFaceKey = 0X01000000 + (nLookID - (LID_NONE + 1));
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

        g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(nFaceKey), 534, 483);
    }

    m_ControbBoard.Draw();

    {
        m_SystemBoard.DrawEx(190, 490, 0, 0, 100, 100);

        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->PushColor(0X00, 0XFF, 0X00, 0XFF);
        g_SDLDevice->DrawRectangle(m_SystemBoard.X(), m_SystemBoard.Y(), 300, 200);
        g_SDLDevice->PopColor();
    }

    // draw HP and MP texture
    {
        extern SDLDevice *g_SDLDevice;
        extern PNGTexDBN *g_ProgUseDBN;

        auto pHP = g_ProgUseDBN->Retrieve(0X00000018);
        auto pMP = g_ProgUseDBN->Retrieve(0X00000019);

        if(pHP && pMP){ 

            // we need to call query
            // so need to validate two textures here

            int nHPH = -1;
            int nHPW = -1;
            int nMPH = -1;
            int nMPW = -1;

            SDL_QueryTexture(pHP, nullptr, nullptr, &nHPW, &nHPH);
            SDL_QueryTexture(pMP, nullptr, nullptr, &nMPW, &nMPH);

            double fLostHPRatio = (m_MyHero->HPMax() > 0) ? (1.0 - ((1.0 * m_MyHero->HP()) / m_MyHero->HPMax())) : 0.0;
            double fLostMPRatio = (m_MyHero->MPMax() > 0) ? (1.0 - ((1.0 * m_MyHero->MP()) / m_MyHero->MPMax())) : 0.0;

            fLostHPRatio = std::max<double>(std::min<double>(fLostHPRatio, 1.0), 0.0);
            fLostMPRatio = std::max<double>(std::min<double>(fLostMPRatio, 1.0), 0.0);

            auto nLostHPH = (int)(std::lround(nHPH * fLostHPRatio));
            auto nLostMPH = (int)(std::lround(nMPH * fLostMPRatio));

            g_SDLDevice->DrawTexture(pHP, 33, 474 + nLostHPH, 0, nLostHPH, nHPW, nHPH - nLostHPH);
            g_SDLDevice->DrawTexture(pMP, 73, 474 + nLostMPH, 0, nLostMPH, nMPW, nMPH - nLostMPH);
        }
    }

    // draw cursor location information on top-left
    extern ClientEnv *g_ClientEnv;
    if(g_ClientEnv->MIR2X_DEBUG_SHOW_LOCATION){
        extern SDLDevice *g_SDLDevice;
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
    if(m_ControbBoard.ProcessEvent(rstEvent, &bValid)){ return; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                // test
                m_SystemBoard.Add("test");

                switch(rstEvent.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                if(auto pCreature = RetrieveUID(nUID)){
                                    m_FocusUIDV[FOCUS_ATTACK] = nUID;
                                    m_MyHero->ParseNewAction({
                                            ACTION_ATTACK,
                                            0,
                                            100,
                                            DIR_NONE,
                                            m_MyHero->CurrMotion().EndX,
                                            m_MyHero->CurrMotion().EndY,
                                            pCreature->X(),
                                            pCreature->Y(),
                                            MapID()}, false);
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
                    default:
                        {
                            break;
                        }
                }
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
        if(auto pMapName = SYS_MAPFILENAME(nMapID)){
            return m_Mir2xMapData.Load(pMapName);
        }
    }
    return -1;
}

bool ProcessRun::CanMove(bool bCheckCreature, int nX, int nY)
{
    if(true
            && (m_Mir2xMapData.Valid())
            && (m_Mir2xMapData.ValidC(nX, nY))
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X00800000)){

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

bool ProcessRun::UserCommand(const char *)
{
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

bool ProcessRun::RegisterLuaExport(ClientLuaModule *pModule, int nOutPort)
{
    if(pModule){

        // initialization before registration
        pModule->script(R"()");

        // register command exitClient
        // exit client and free all related resource
        pModule->set_function("exitClient", [](){ std::exit(0); });

        // register command exit
        pModule->set_function("exit", [](){
            // reserve this command
            // don't find what to exit here
            std::exit(0);
        });

        // register command sleep
        // sleep current thread and return after the specified ms
        // can use posix.sleep(ms), but here use std::this_thread::sleep_for(x)
        pModule->set_function("sleep", [](int nSleepMS){
            if(nSleepMS > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
            }
        });

        // register command printLine
        // print one line to the out port allocated for the lua module
        // won't add message to log system, use addLog instead if needed
        pModule->set_function("printLine", [this, nOutPort](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo){
            // use sol::object to accept arguments
            // otherwise for follow code it throws exception for type unmatch
            //      lua["f"] = [](int a){ return a; };
            //      lua.script("print f(\"hello world\")")
            // program crashes with exception.what() : expecting int, string provided
            if(true
                    && stLogType.is<int>()
                    &&  stPrompt.is<std::string>()
                    && stLogInfo.is<std::string>()){
                AddOPLog(nOutPort, stLogType.as<int>(), stPrompt.as<std::string>().c_str(), stLogInfo.as<std::string>().c_str());
                return;
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "printLine(LogType: int, Prompt: string, LogInfo: string)");
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
            AddOPLog(nOutPort, 2, ">>> ", "addLog(LogType: int, LogInfo: string)");
        });

        // register command playerList
        // get a list for all active maps
        // return a table (userData) to lua for ipairs() check
        pModule->set_function("playerList", [this](sol::this_state stThisLua){
            return sol::make_object(sol::state_view(stThisLua), GetPlayerList());
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
        pModule->set_function("myHero_dress", [this](int nDress){
            if(nDress >= 0){
                m_MyHero->Dress((uint32_t)(nDress));
            }
        });

        // register command ``myHero.xxx"
        // I need to insert a table to micmic a instance myHero in the future
        pModule->set_function("myHero_weapon", [this](int nWeapon){
            if(nWeapon >= 0){
                m_MyHero->Weapon((uint32_t)(nWeapon));
            }
        });


        // registration done
        return true;
    }
    return false;
}

bool ProcessRun::AddOPLog(int nOutPort, int nLogType, const char *szPrompt, const char *szLogInfo)
{
    if(nOutPort & OUTPORT_LOG){
        extern Log *g_Log;
        switch(nLogType){
            case 0  : g_Log->AddLog(LOGTYPE_INFO   , "%s", (std::string(szPrompt ? szPrompt : "") + szLogInfo).c_str()); break;
            case 1  : g_Log->AddLog(LOGTYPE_WARNING, "%s", (std::string(szPrompt ? szPrompt : "") + szLogInfo).c_str()); break;
            default : g_Log->AddLog(LOGTYPE_FATAL  , "%s", (std::string(szPrompt ? szPrompt : "") + szLogInfo).c_str()); break;
        }
    }

    if(nOutPort & OUTPORT_SCREEN){
    }

    if(nOutPort & OUTPORT_CONTROLBOARD){
    }

    return true;
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
                        0,
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
