/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 06/02/2016 17:43:40
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
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

ProcessRun::ProcessRun()
    : Process()
    , m_ViewX(0)
    , m_ViewY(0)
{
}

void ProcessRun::Update(double)
{
}

// ProcessRun::Start()
//
//     LoadMap(stCMLS.szMapName);
//     delete m_MyHero;
//     m_MyHero = new MyHero(stCMLS.nSID, stCMLS.nUID, stCMLS.nGenTime);
//     m_MyHero->SetMap(stCMLS.nMapX, stCMLS.nMapY, &m_Map);
//     m_MyHero->SetDirection(stCMLS.nDirection);
//
//     int nStartX = (std::min)((std::max)(0, m_MyHero->X() - m_WindowW / 2), m_Map.W() * 48 - m_WindowW);
//     int nStartY = (std::min)((std::max)(0, m_MyHero->Y() - m_WindowH / 2), m_Map.H() * 32 - m_WindowH);
//
//     m_Map.SetViewPoint(nStartX, nStartY);
// }
//     m_WindowW = GetConfigurationManager()->GetInt("Root/Window/SizeW");
//     m_WindowH = GetConfigurationManager()->GetInt("Root/Window/SizeH");
// }
//
// ProcessRun::~ProcessRun()
// {}
//
// void ProcessRun::Enter()
// {
// 	Process::Enter();
//
//     // delete m_MyHero;
//     // m_MyHero = new MyHero(0, 0, 0);
//     // m_MyHero->SetPosition(800, 400);
// }
//
// void ProcessRun::Exit()
// {
// 	Process::Exit();
// }
//
// void ProcessRun::LoadMap(const char #<{(| szMapFileName |)}>#)
// {
//     // extern XMLConf *g_XMLConf;
//     // std::string szMapFullFileName = g_XMLConf->GetXMLNode("Root/Map/Path");
//     m_Map.Load("./DESC.BIN");
// }
//
//
// void ProcessRun::RollScreen()
// {
//     // what we should have
//     int nDX = m_MyHero->X() - m_WindowW / 2 - m_Map.ViewX();
//     int nDY = m_MyHero->Y() - m_WindowH / 2 - m_Map.ViewY();
//
//     int nCurrentViewX = m_Map.ViewX();
//     int nCurrentViewY = m_Map.ViewY();
//
//     if(std::abs(nDX) > 20){
//         nCurrentViewX += std::lround(std::copysign(1.0, nDX));
//     }
//     if(std::abs(nDY) > 20){
//         nCurrentViewY += std::lround(std::copysign(1.0, nDY));
//     }
//
//     int nStartX = (std::min)((std::max)(0, nCurrentViewX), m_Map.W() * 48 - m_WindowW);
//     int nStartY = (std::min)((std::max)(0, nCurrentViewY), m_Map.H() * 32 - m_WindowH);
//
//     m_Map.SetViewPoint(nStartX, nStartY);
// }
//
void ProcessRun::Draw()
{
    static auto fnDrawTile = [this](int nGX, int nGY, uint32_t nKey){
        extern PNGTexDBN *g_PNGTexDBN;
        // 1. retrieve texture
        auto pTexture = g_PNGTexDBN->Retrieve(nKey);
        if(!pTexture){ return; }

        // 2. get the start point
        int nPX = nGX * SYS_MAPGRIDXP - m_ViewX;
        int nPY = nGY * SYS_MAPGRIDYP - m_ViewY;

        // 3. draw it
        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->DrawTexture(pTexture, nPX, nPY);
    };

    static auto fnDrawObj = [this](int nGX, int nGY, uint32_t nKey){
        extern PNGTexDBN *g_PNGTexDBN;
        // 1. retrieve texture
        auto pTexture = g_PNGTexDBN->Retrieve(nKey);
        if(!pTexture){ return; }

        // 2. we need it's height
        int nH = 0;
        SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nH);

        // 3. get the start point
        int nPX = nGX * SYS_MAPGRIDXP - m_ViewX;
        int nPY = nGY * SYS_MAPGRIDYP - m_ViewY + SYS_MAPGRIDYP - nH;

        // 4. draw it
        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->DrawTexture(pTexture, nPX, nPY);
    };

    static auto fnDrawActor = [this](int nGX, int nGY){
        if(nGX >= 0 && nGY >= 0){
        }
    };

    static auto fnDrawExt = [this](int nGX, int nGY){
        if(nGX >= 0 && nGY >= 0){
        }
    };

    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->ClearScreen();

    if(m_Map.Valid()){
        extern SDLDevice *g_SDLDevice;
        m_Map.Draw(m_ViewX, m_ViewY,
                g_SDLDevice->WindowW(false), g_SDLDevice->WindowH(false),
                SYS_OBJMAXW, SYS_OBJMAXH, fnDrawTile, fnDrawObj, fnDrawActor, fnDrawExt);
    }

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
// 					pActor->SetMap(stTmpCM.nX, stTmpCM.nY, &m_Map);
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

// we get all needed initialization info for init the process run
void ProcessRun::Net_LoginOK(const uint8_t *pBuf, size_t nLen)
{
    if(!(pBuf && nLen && nLen == sizeof(SMLoginOK))){ return; }

    SMLoginOK stLoginOK;
    std::memcpy(&stLoginOK, pBuf, nLen);

    m_Map.Load("./DESC.BIN");
    // m_MyHero.SetGUID(stLoginOK.GUID);
    // m_MyHero.SetDirection(stLoginOK.Direction);
}

#include <cstdio>
void ProcessRun::Net_MotionState(const uint8_t *pBuf, size_t)
{
    SMMotionState stSMMS = *((const SMMotionState *)pBuf);
    if(stSMMS.MapID != m_ClientMap.ID()){ return; }

    auto pRecord = m_CreatureMap.find(((uint64_t)stSMMS.UID << 32) + stSMMS.AddTime);
    if(true
            && pRecord != m_CreatureMap.end()
            && pRecord.second
            && pRecord.second->Type(stSMMS.Type)){
        auto pCreature = pRecord.second;

        pCreature->ResetR(stSMMS.State);
        pCreature->ResetMotionState(stSMMS.State);
        pCreature->ResetLocation(stSMMS.MapID, stSMMS.X, stSMMS.Y);
    }
}

void ProcessRun::Net_MONSTERGINFO(const uint8_t *pBuf, size_t)
{
    auto *pInfo = (MonsterGInfo *)pBuf;
    Monster::CreateGInfoRecord(
            pInfo->MonsterID,
            pInfo->LookID[0],
            pInfo->LookID[1],
            pInfo->LookID[2],
            pInfo->LookID[3]); 
}
