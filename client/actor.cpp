/*
 * =====================================================================================
 *
 *       Filename: actor.cpp
 *        Created: 8/31/2015 10:45:48 PM
 *  Last Modified: 09/09/2015 7:51:44 PM
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

#include <SDL.h>
#include <string>
#include "actor.hpp"
#include <tinyxml2.h>
#include "mir2clientmap.hpp"
#include "devicemanager.hpp"
#include "texturemanager.hpp"
#include "configurationmanager.hpp"

// global offset info vector
// std::vector<std::vector<std::vector<std::vector<std::array<std::array<int, 2>, 2>>>>> g_GlobalActorOffset;
// static std::vector<std::vector<std::vector<std::vector<std::array<std::array<int, 2>, 2>>>>>
// g_GlobalActorOffset = std::vector<std::vector<std::vector<std::vector<std::array<std::array<int, 2>, 2>>>>>(1024);

// g_GlobalActorOffset[nPrecode][nLID][state][direction][frame][shadow/body][x/y]
static std::array<std::array<std::vector<std::vector<std::vector<std::array<std::array<int,
    2>, 2>>>>, 1024>, 8> g_GlobalActorOffset;

static std::vector<std::tuple<int, int, DirectiveRectCover>> g_SIDStateDRCTupleCacheV;

Actor::Actor(int nSID, int nUID, int nGenTime)
    : m_SID(nSID)
    , m_UID(nUID)
    , m_GenTime(nGenTime)
    , m_FrameIndex(0)
    , m_Direction(0)
    , m_State(0)
    , m_NextState(-1)
    , m_Map(nullptr)
    , m_FrameUpdateDelay(120)
    , m_UpdateTime(SDL_GetTicks())
    , m_Speed(100)
{
}

Actor::~Actor()
{}

void Actor::SetHP(int nHP)
{
    m_HP = nHP;
}

int Actor::SID()
{
    return m_SID;
}

int Actor::UID()
{
    return m_UID;
}

int Actor::ScreenX()
{
    return X() - m_Map->ViewX();
}

int Actor::ScreenY()
{
    return Y() - m_Map->ViewY();
}

int Actor::X()
{
    return m_X;
}

int Actor::Y()
{
    return m_Y;
}

void Actor::OnMessage(const Message &)
{}

void Actor::SetNextPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Actor::SetNextState(int nState)
{
    m_State = nState;
}

void Actor::SetState(int nState){
    m_State = nState;
}

void Actor::SetPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Actor::SetDirection(int nDir)
{
    m_Direction = nDir;
}

int Actor::GenTime()
{
    return m_GenTime;
}

void Actor::SetMap(int nX, int nY, Mir2ClientMap *pMap)
{
    m_X   = nX;
    m_Y   = nY;
    m_Map = pMap;
}

void Actor::LoadGlobalOffset(int nPrecode, int nLookID)
{
    nPrecode %= 8;
    nLookID  %= 1024;

    // take as a 2d array, so
    // g_GlobalActorOffset[nPrecode][nLookID] is always ok

    if(g_GlobalActorOffset[nPrecode][nLookID].size() == 0){
        std::string szOffsetFullFileName;
        szOffsetFullFileName += GetConfigurationManager()->GetString("Root/Actor/LID/Path");
        szOffsetFullFileName += "/";
        szOffsetFullFileName += std::to_string(nPrecode);
        szOffsetFullFileName += "/";
        szOffsetFullFileName += std::to_string(nLookID);
        szOffsetFullFileName += "/desc.xml";

        tinyxml2::XMLDocument XMLDoc;
        XMLDoc.LoadFile(szOffsetFullFileName.c_str());
        auto pRoot = XMLDoc.RootElement();
        if(pRoot == nullptr){
            return;
        }

        auto pSet = pRoot->FirstChildElement("ActionSet");
        while(pSet != nullptr){
            int nState     = std::atoi(pSet->Attribute("State"))     % 50;
            int nDirection = std::atoi(pSet->Attribute("Direction")) %  8;
            if(nState >= g_GlobalActorOffset[nPrecode][nLookID].size()){
                g_GlobalActorOffset[nPrecode][nLookID].resize(nState + 1);
            }
            if(nDirection >= g_GlobalActorOffset[nPrecode][nLookID][nState].size()){
                g_GlobalActorOffset[nPrecode][nLookID][nState].resize(nDirection + 1);
            }

            auto pFrame = pSet->FirstChildElement("Frame");
            while(pFrame != nullptr){
                auto pShadow = pFrame->FirstChildElement("Shadow");
                auto pBody   = pFrame->FirstChildElement("Body");
                auto pSDX    = pShadow->FirstChildElement("DX");
                auto pSDY    = pShadow->FirstChildElement("DY");
                auto pBDX    = pBody->FirstChildElement("DX");
                auto pBDY    = pBody->FirstChildElement("DY");
                std::array<std::array<int, 2>, 2> stTmpV22;
                stTmpV22[0][0] = std::atoi(pBDX->GetText());
                stTmpV22[0][1] = std::atoi(pBDY->GetText());
                stTmpV22[1][0] = std::atoi(pSDX->GetText());
                stTmpV22[1][1] = std::atoi(pSDY->GetText());
                g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection].push_back(stTmpV22);

                pFrame = pFrame->NextSiblingElement("Frame");
            }
            pSet = pSet->NextSiblingElement("ActionSet");
        }
    }
}

int Actor::FrameCount(int nPrecode, int nLID)
{
    if(g_GlobalActorOffset[nPrecode][nLID].size() == 0){
        LoadGlobalOffset(nPrecode, nLID);
    }
    return g_GlobalActorOffset[nPrecode][nLID][m_State][m_Direction].size();
}

void Actor::InnDraw(
        int nPrecode, int nLookID,
        int nState, int nDirection, int nFrameIndex,
        int nX, int nY)
{
    // nPrecode:
    //      0. occupied for map
    //      1. monster body
    //      2. monster shadow
    //      3. human body
    //      4. human shadow
    // we really use is 1 and 3
	nPrecode %= 8;
	nLookID  %= 1024;

    if(g_GlobalActorOffset[nPrecode][nLookID].size() == 0){
        LoadGlobalOffset(nPrecode, nLookID);
    }

    if(false
            || g_GlobalActorOffset[nPrecode][nLookID][nState].size()             == 0
            || g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection].size() == 0
      ){
        return;
    }

    uint32_t nFileIndex  = nLookID;
    uint32_t nImageIndex = nState * 1000 + nDirection * 100 + nFrameIndex;
    { // draw shadow
        SDL_Texture *pTexture = 
            GetTextureManager()->RetrieveTexture(nPrecode + 1, nFileIndex, nImageIndex);
        int nW, nH;
        SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);
        SDL_Rect stDst = {
            nX - m_Map->ViewX() + 
                g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection][nFrameIndex][1][0],
            nY - m_Map->ViewY() +
                g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection][nFrameIndex][1][1],
            nW, nH
        };

        SDL_RenderCopy(GetDeviceManager()->GetRenderer(), pTexture, nullptr, &stDst);
    }

    { // draw body
        SDL_Texture *pTexture =
            GetTextureManager()->RetrieveTexture(nPrecode, nFileIndex, nImageIndex);
        int nW, nH;
        SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);
        SDL_Rect stDst = {
            nX - m_Map->ViewX()
                + g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection][nFrameIndex][0][0],
            nY - m_Map->ViewY()
                + g_GlobalActorOffset[nPrecode][nLookID][nState][nDirection][nFrameIndex][0][1],
            nW, nH
        };

        SDL_RenderCopy(GetDeviceManager()->GetRenderer(), pTexture, nullptr, &stDst);
    }
}

void Actor::UpdateCurrentState()
{
    m_FrameIndex = ((m_FrameIndex + 1) % FrameCount());
}

void Actor::UpdateWithNewState()
{
}

int Actor::CalculateDirection(int nDX, int nDY)
{
    int nDirection = 0;
    if(nDX == 0){
        if(nDY > 0){
            nDirection = 4;
        }else{
            nDirection = 0;
        }
    }else{
        double dATan = std::atan(1.0 * nDY / nDX);
        if(nDX > 0){
            nDirection = std::lround(2.0 + dATan * 4.0 / 3.1416) % 8;
        }else{
            nDirection = std::lround(6.0 + dATan * 4.0 / 3.1416) % 8;
        }
    }
    return nDirection;
}

void Actor::EstimateNextPosition(int nDistance)
{
    double dDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double dDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    m_EstimateNextX = m_X + std::lround(dDX[m_Direction] * nDistance);
    m_EstimateNextY = m_Y + std::lround(dDY[m_Direction] * nDistance);
}

int Actor::EstimateNextX()
{
    return m_EstimateNextX;
}

int Actor::EstimateNextY()
{
    return m_EstimateNextY;
}

void Actor::Goto(int nX, int nY)
{
    EstimateNextPosition();
    m_NextX = nX;
    m_NextY = nY;
}

bool Actor::TryStepMove(std::function<bool()> fnExtCheck)
{
    EstimateNextPosition();
    return m_Map->ValidPosition(m_EstimateX, m_EstimateY, this);
}

bool Actor::SetGlobalCoverInfoCache(int nSID, int nState, int nDir, int nW, int nH)
{
    m_SIDStateDRCTupleCacheV.emplace_back(
            nSID, nState, DirectiveRectCover(nDir, 0.0, 0.0, (double)nW, (double)nH));
    return true;
}

int Actor::GlobalCoverInfoCacheIndex(int nSID, int nState, int nDirection)
{
    switch(((uint32_t)nSID & 0XFFFFFC00) >> 10){
        case 1:
            { // monster
                nSID = nSID;
                break;
            }
        case 3:
            { // human
                nSID = (3 << 10);
                break;
            }
        default:
            {
                nSID = nSID;
                break;
            }
    }

    for(int nCnt = 0; nCnt < m_SIDStateDRCTupleV.size(); ++nCnt){
        if(true
                && std::get<0>(m_SIDStateDRCTupleCacheV[nCnt]) == nSID
                && std::get<1>(m_SIDStateDRCTupleCacheV[nCnt]) == m_State
                && std::get<2>(m_SIDStateDRCTupleCacheV[nCnt]).Direction() == m_Direction
          ){
            return nCnt;
        }
    }

    return -1;
}

bool Actor::GlobalCoverInfoCacheValid(int nSID, int nState, int nDirection)
{
    return GlobalCoverInfoCacheIndex(nSID, nState, nDirection) >= 0;
}

bool Actor::SetGlobalCoverInfoCache(int nSID, int nState, int nDir, int nW, int nH)
{
    int nIndex = GlobalCoverInfoCacheIndex(nSID, nState, nDir);
    if(nIndex >= 0){
        g_SIDStateDRCTupleCacheV[nIndex].Reset(nDir, 0.0, 0.0, nW * 1.0, nH * 1.0);
    }else{
        m_SIDStateDRCTupleCacheV.emplace_back(
            nSID, nState, DirectiveRectCover(nDir, 0.0, 0.0, (double)nW, (double)nH));
    }
    return true;
}
