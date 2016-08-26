/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 08/26/2016 12:59:18
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
    if(m_MyHero){
        extern SDLDevice *g_SDLDevice;
        m_ViewX = m_MyHero->X() - g_SDLDevice->WindowW(false) / 2;
        m_ViewY = m_MyHero->Y() - g_SDLDevice->WindowH(false) / 2;
    }

    for(auto &pRecord: m_CreatureMap){
        if(pRecord.second){
            pRecord.second->Update();
        }
    }
}

// ProcessRun::Start()
//
//     LoadMap(stCMLS.szMapName);
//     delete m_MyHero;
//     m_MyHero = new MyHero(stCMLS.nSID, stCMLS.nUID, stCMLS.nGenTime);
//     m_MyHero->SetMap(stCMLS.nMapX, stCMLS.nMapY, &m_ClientMap);
//     m_MyHero->SetDirection(stCMLS.nDirection);
//
//     int nStartX = (std::min)((std::max)(0, m_MyHero->X() - m_WindowW / 2), m_ClientMap.W() * 48 - m_WindowW);
//     int nStartY = (std::min)((std::max)(0, m_MyHero->Y() - m_WindowH / 2), m_ClientMap.H() * 32 - m_WindowH);
//
//     m_ClientMap.SetViewPoint(nStartX, nStartY);
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
//     m_ClientMap.Load("./DESC.BIN");
// }
//
//
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
            for(auto &pRecord: m_CreatureMap){
                if(pRecord.second
                        && ((pRecord.second->X() / SYS_MAPGRIDXP) == nGX)
                        && ((pRecord.second->Y() / SYS_MAPGRIDYP) == nGY)){
                    pRecord.second->Draw(m_ViewX, m_ViewY);
                }
            }
        }
    };

    static auto fnDrawExt = [this](int nGX, int nGY){
        if(nGX >= 0 && nGY >= 0){
        }
    };

    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->ClearScreen();

    if(m_ClientMap.Valid()){
        extern SDLDevice *g_SDLDevice;
        m_ClientMap.Draw(m_ViewX, m_ViewY,
                g_SDLDevice->WindowW(false), g_SDLDevice->WindowH(false),
                SYS_OBJMAXW, SYS_OBJMAXH, fnDrawTile, fnDrawObj, fnDrawActor, fnDrawExt);
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

// we get all needed initialization info for init the process run
void ProcessRun::Net_LOGINOK(const uint8_t *pBuf, size_t nLen)
{
    if(!(pBuf && nLen && nLen == sizeof(SMLoginOK))){ return; }

    SMLoginOK stSMLOK;
    std::memcpy(&stSMLOK, pBuf, nLen);

    m_ClientMap.Load(stSMLOK.MapID);
    m_MyHero = new MyHero(stSMLOK.GUID, stSMLOK.UID, stSMLOK.AddTime, (bool)(stSMLOK.Male));

    m_MyHero->ResetLocation(stSMLOK.MapID, (int)(stSMLOK.X), (int)(stSMLOK.Y));
    m_MyHero->ResetDirection((int)(stSMLOK.Direction));
    m_MyHero->ResetLevel((int)(stSMLOK.Level));
    m_MyHero->ResetJob((int)(stSMLOK.JobID));
}

void ProcessRun::Net_ACTIONSTATE(const uint8_t *pBuf, size_t)
{
    SMActionState stSMAS = *((const SMActionState *)pBuf);
    if(stSMAS.MapID != m_ClientMap.ID()){ return; }

    auto pRecord = m_CreatureMap.find(((uint64_t)stSMAS.UID << 32) + stSMAS.AddTime);
    if(true
            && pRecord != m_CreatureMap.end()
            && pRecord->second
            && pRecord->second->MapID() == stSMAS.MapID){
        auto pCreature = pRecord->second;

        pCreature->ResetR((int)stSMAS.R);
        pCreature->ResetLocation(stSMAS.MapID, stSMAS.X, stSMAS.Y);

        pCreature->ResetAction((int)stSMAS.Action);
        pCreature->ResetDirection((int)stSMAS.Direction);

        pCreature->ResetSpeed((int)stSMAS.Speed);
    }
}

void ProcessRun::Net_MONSTERGINFO(const uint8_t *pBuf, size_t)
{
    auto *pInfo = (SMMonsterGInfo *)pBuf;
    Monster::GetGInfoRecord(pInfo->MonsterID).ResetLookID(pInfo->LookIDN, pInfo->LookID, pInfo->R);
}

void ProcessRun::Net_CORECORD(const uint8_t *pBuf, size_t)
{
    SMCORecord stSMCOR;
    std::memcpy(&stSMCOR, pBuf, sizeof(stSMCOR));

    Creature *pCreature = nullptr;
    uint64_t nCOKey = ((((uint64_t)stSMCOR.Common.UID) << 32) + stSMCOR.Common.AddTime);

    auto pRecord = m_CreatureMap.find(nCOKey);
    if(pRecord != m_CreatureMap.end()){
        pCreature = pRecord->second;
    }else{
        switch(stSMCOR.Type){
            case CREATURE_MONSTER:
                {
                    pCreature = new Monster(stSMCOR.Monster.MonsterID, stSMCOR.Common.UID, stSMCOR.Common.AddTime);
                    break;
                }
            case CREATURE_PLAYER:
                {
                    break;
                }
            default:
                {
                    break;
                }
        }
    }

    if(pCreature){
        pCreature->ResetR((int)stSMCOR.Common.R);
        pCreature->ResetLocation(stSMCOR.Common.MapID, (int)stSMCOR.Common.MapX, (int)stSMCOR.Common.MapY);

        pCreature->ResetAction((int)stSMCOR.Common.Action);
        pCreature->ResetDirection((int)stSMCOR.Common.Direction);

        pCreature->ResetSpeed((int)stSMCOR.Common.Speed);

        m_CreatureMap[nCOKey] = pCreature;
    }
}
