/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 03/27/2017 11:18:06
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
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

ProcessRun::ProcessRun()
    : Process()
    , m_MyHero(nullptr)
    , m_ViewX(0)
    , m_ViewY(0)
    , m_ControbBoard(0, 0, nullptr, false)
{
}

void ProcessRun::Update(double)
{
    m_ViewX = 300;
    m_ViewY = 300;
    // if(m_MyHero){
    //     extern SDLDevice *g_SDLDevice;
    //     m_ViewX = m_MyHero->X() - g_SDLDevice->WindowW(false) / 2;
    //     m_ViewY = m_MyHero->Y() - g_SDLDevice->WindowH(false) / 2;
    // }
    //
    // for(auto &pRecord: m_CreatureRecord){
    //     if(pRecord.second){
    //         pRecord.second->Update();
    //     }
    // }
}

// void ProcessRun::RollScreen()
// {
//     // what we should have
//     int nDX = m_MyHero->X() - m_WindowW / 2 - m_ClientMap.ViewX();
//     int nDY = m_MyHero->Y() - m_WindowH / 2 - m_ClientMap.ViewY();
//
//     int nCurrentViewX = m_ClientMap.ViewX();
//     int nCurrentViewY = m_ClientMap.ViewY();
//
//     if(std::abs(nDX) > 20){
//         nCurrentViewX += std::lround(std::copysign(1.0, nDX));
//     }
//     if(std::abs(nDY) > 20){
//         nCurrentViewY += std::lround(std::copysign(1.0, nDY));
//     }
//
//     int nStartX = (std::min)((std::max)(0, nCurrentViewX), m_ClientMap.W() * 48 - m_WindowW);
//     int nStartY = (std::min)((std::max)(0, nCurrentViewY), m_ClientMap.H() * 32 - m_WindowH);
//
//     m_ClientMap.SetViewPoint(nStartX, nStartY);
// }
//
void ProcessRun::Draw()
{
    extern PNGTexDBN *g_PNGTexDBN;
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
                        if(auto pTexture = g_PNGTexDBN->Retrieve(nParam& 0X00FFFFFF)){
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
                                if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
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
                                if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
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
                                if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
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
                                if(auto pTexture = g_PNGTexDBN->Retrieve(nParam & 0X00FFFFFF)){
                                    int nH = 0;
                                    if(!SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH)){
                                        g_SDLDevice->DrawTexture(pTexture, nX * SYS_MAPGRIDXP - m_ViewX, (nY + 1) * SYS_MAPGRIDYP - m_ViewY - nH);
                                    }
                                }
                            }
                        }
                    }
                }

                // draw actors
                {

                }

            }
        }
    }

    m_ControbBoard.Draw();
    g_SDLDevice->Present();
}

//
// void ProcessRun::UpdateActor()
// {
//     std::lock_guard<std::mutex> stGuard(m_ActorListMutex);
//     for(auto pActor: m_ActorList){
// 		pActor->Update();
//     }
// }
//
// void ProcessRun::Update()
// {
//     // TODO
//     {
//         std::lock_guard<std::mutex> stGuard(m_ActorListMutex);
//         for(auto pActor: m_ActorList){
//             if(pActor != m_MyHero){
//                 pActor->Update();
//             }
//         }
//         {
//             std::lock_guard<std::mutex> stGuard(m_MyHeroMutex);
//             m_MyHero->EstimateNextPosition();
//             int nNextX = m_MyHero->EstimateNextX();
//             int nNextY = m_MyHero->EstimateNextY();
//
//             for(auto pActor: m_ActorList){
//                 if(pActor != m_MyHero){
//                     pActor->Update();
//                 }
//             }
//         }
//
//     }
// 	UpdateMyHero();
//     UpdateActor();
//
// 	auto fnMessageHandler = [this](const Message &stMessage){
// 		HandleMessage(stMessage);
// 	};
//
// 	GetMessageManager()->BatchHandleMessage(fnMessageHandler);
// }
//
// void ProcessRun::UpdateMyHero()
// {
// 	std::lock_guard<std::mutex> stGuard(m_MyHeroMutex);
// 	m_MyHero->Update();
// }
//
// void ProcessRun::HandleMessage(const Message &stMessage)
// {
//     switch(stMessage.Index()){
//         case CLIENTMT_ACTORBASEINFO:
//             {
//                 ClientMessageActorBaseInfo stTmpCM;
//                 std::memcpy(&stTmpCM, stMessage.Body(), sizeof(stTmpCM));
//                 std::lock_guard<std::mutex> stGuard(m_ActorListMutex);
// 				bool bFind = false;
//                 for(auto pActor: m_ActorList){
//                     if(true
//                             && pActor->UID()     == stTmpCM.nUID
//                             && pActor->SID()     == stTmpCM.nSID
//                             && pActor->GenTime() == stTmpCM.nGenTime
//                       ){
// 						bFind = true;
//                         pActor->SetNextState(stTmpCM.nState);
//                         pActor->SetNextPosition(stTmpCM.nX, stTmpCM.nY);
//                         // pActor->SetHP(stTmpCM.nHP);
//                     }
//                 }
//
// 				if(!bFind){
// 					// can't find it, create a new one
// 					auto pActor = NewActor(stTmpCM.nSID, stTmpCM.nUID, stTmpCM.nGenTime);
// 					pActor->SetDirection(stTmpCM.nDirection);
// 					pActor->SetNextState(stTmpCM.nState);
// 					// pActor->SetNextPosition(stTmpCM.nX, stTmpCM.nY);
// 					pActor->SetMap(stTmpCM.nX, stTmpCM.nY, &m_ClientMap);
// 					m_ActorList.push_back(pActor);
// 					// pActor->SetHP(stTmpCM.nHP);
// 				}
//                 break;
//             }
//         default:
//             break;
//     }
// }
//
// Actor *ProcessRun::NewActor(int nSID, int nUID, int nGenTime)
// {
//     switch(nSID){
//         case 1000:
//         case 1001:
//         case 1002:
//         case 1003:
//         case 1004:
//         case 1005:
//             {
//                 return new Hero(nSID, nUID, nGenTime);
//             }
//         default:
//             {
//                 return new Monster(nSID, nUID, nGenTime);
//             }
//     }
// }
//

void ProcessRun::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(rstEvent.button.button){
                    case SDL_BUTTON_RIGHT:
                        {
                            // int nDX = pEvent->button.x - m_MyHero->ScreenX();
                            // int nDY = pEvent->button.y - m_MyHero->ScreenY();
                            // m_MyHero->DGoto(nDX, nDY);
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
                // m_MyHero->SetState(0);
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
        if(auto pMapName = SYS_MAPNAME(nMapID)){
            return m_Mir2xMapData.Load(pMapName);
        }
    }
    return -1;
}
