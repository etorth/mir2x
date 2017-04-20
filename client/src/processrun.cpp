/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 04/19/2017 18:11:14
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
    , m_RollMap(false)
    , m_ControbBoard(0, 0, nullptr, false)
{
}

void ProcessRun::Update(double)
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

            m_ViewX += (int)(std::lround(std::copysign(std::min<int>(12, std::abs(nDViewX)), nDViewX)));
            m_ViewY += (int)(std::lround(std::copysign(std::min<int>( 8, std::abs(nDViewY)), nDViewY)));

            m_ViewX = std::max<int>(0, m_ViewX);
            m_ViewY = std::max<int>(0, m_ViewY);
        }

        // stop rolling the map when
        //   1. the hero is at the required position
        //   2. the hero is not moving
        if((nDViewX == 0) && (nDViewY == 0) && !m_MyHero->Moving()){ m_RollMap = false; }
    }

    for(auto pRecord: m_CreatureRecord){
        if(pRecord.second){
            pRecord.second->Update();
        }
    }
}

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

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG == 10)
        {
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
#endif

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

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG == 10)
                    {
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
#endif
                }

                // draw actors
                {
                    for(auto pCreature: m_CreatureRecord){
                        if(pCreature.second && (pCreature.second->X() == nX) && (pCreature.second->Y() == nY)){
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG == 10)
                            {
                                g_SDLDevice->PushColor(0, 0, 255, 30);
                                g_SDLDevice->FillRectangle(nX * SYS_MAPGRIDXP - m_ViewX, nY * SYS_MAPGRIDYP - m_ViewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                                g_SDLDevice->PopColor();
                            }
#endif
                            pCreature.second->Draw(m_ViewX, m_ViewY);
                        }
                    }
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
                            // in mir2ei how human moves
                            // 1. client send motion request to server
                            // 2. client put motion lock to human
                            // 3. server response with "+GOOD" or "+FAIL" to client
                            // 4. if "+GOOD" client will release the motion lock
                            // 5. if "+FAIL" client will use the backup position and direction

                            int nX = -1;
                            int nY = -1;
                            if(LocatePoint(rstEvent.button.x, rstEvent.button.y, &nX, &nY)){
                                m_MyHero->RequestMove(nX, nY);
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
        if(auto pMapName = SYS_MAPFILENAME(nMapID)){
            return m_Mir2xMapData.Load(pMapName);
        }
    }
    return -1;
}

bool ProcessRun::CanMove(bool bCheckCreature, int nX, int nY){
    if(m_Mir2xMapData.ValidC(nX, nY)
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X00800000)){
        if(bCheckCreature){
            for(auto pCreature: m_CreatureRecord){
                if(pCreature.second && (pCreature.second->X() == nX) && (pCreature.second->Y() == nY)){
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
    if(m_Mir2xMapData.ValidC(nX0, nY0)){
        if(m_Mir2xMapData.ValidC(nX1, nY1) && (std::abs(nX0 - nX1) <= 1) && (std::abs(nY0 - nY1) <= 1)){
            return CanMove(bCheckCreature, nX1, nY1);
        }
    }
    return false;
}

bool ProcessRun::LocatePoint(int nPX, int nPY, int *pX, int *pY)
{
    if(pX){ *pX = (nPX + m_ViewX) / SYS_MAPGRIDXP; }
    if(pY){ *pY = (nPY + m_ViewY) / SYS_MAPGRIDYP; }

    return true;
}
