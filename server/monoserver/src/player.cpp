/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/19/2016 17:06:19
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

#include "player.hpp"
#include "charobject.hpp"

Player::Player(uint32_t nUID, uint32_t nAddTime, uint32_t nGUID, uint32_t nSID)
    : CharObject(nUID, nAddTime)
    , m_SID(nSID)
    , m_GUID(nGUID)
{
    Install("CheckTime", fnCheckTime);
}

Player::~Player()
{
}

void Player::Operate(const MessagePack &, const Theron::Address &)
{
}

bool Player::Update()
{
    return true;
}

bool Player::Type(uint8_t nType)
{
    switch(nType){
        case OBJECT_ANIMAL: return true;
        default: return false;
    }
    return false;
}

bool Player::SetType(uint8_t nType, bool bThisType)
{
    m_TypeV[nType] = 1;
    return bThisType;
}

bool Player::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Player::SetState(uint8_t nState, bool bThisState)
{
    m_StateV[nState] = bThisState;
    return bThisState;
}

uint32_t Player::NameColor()
{
    return 0XFFFFFFFF;
}

const char *Player::CharName()
{
    return "hello";
}

int Player::Range(uint8_t)
{
    return 20;
}

void Player::OperateNet(uint8_t nType, const uint8_t * pData, size_t nDataLen)
{

}

void Player::OnSessionReadHC(uint8_t nType, Session *pSession)
{
    extern MonoServer *g_MonoServer;
    size_t nLen = g_MonoServer->MessageSize(nType);

    if(nLen > 0){
        extern MemoryChunkPN *g_MemoryChunkPN;
        auto pBuf = (uint8_t *)(g_MemoryChunkPN->Get(nLen));

        auto fnSendPKG = [](){
            AMNetPackage stAMNP;
            stAMNP.Type    = nType;
            stAMNP.Data    = pBuf;
            stAMNP.DataLen = nLen;

            pSession->Forward({MPK_NET, stAMNP}, pSession->TargetAddress(), nullptr);
        };
        pSession->Read(nLen, pBuf, fnSendPKG);
        return;
    }

    // zero-len, check whether its unfixed
    if(g_MonoServer->MessageFixedSize(nType)){
        AMNetPackage stAMNP;
        stAMNP.Type    = nType;
        stAMNP.Data    = nullptr;
        stAMNP.DataLen = 0;

        pSession->Forward({MPK_NET, stAMNP}, pSession->TargetAddress(), nullptr);
        return;
    }

    // unfixed-size message, hard

    auto fnReadLen = [](const uint8_t *pBuf, size_t nLen){
        size_t nStreamLen = *((size_t *)pBuf);

        auto fnReadStreamBody = [](){
            AMNetPackage stAMNP;
            stAMNP.Type    = nType;
            stAMNP.Data    = nullptr;
            stAMNP.DataLen = 0;

            pSession->Forward({MPK_NET, stAMNP}, pSession->TargetAddress(), nullptr);
            return;

        };

        auto pStreamBuf = g_MemoryChunkPN->Get(nStreamLen);
        pSession->Read(nStreamLen, pStreamBuf, fnReadStreamBody);
    };

    pSession->Read(sizeof(uint16_t), fnReadLen);
}

void Player::CheckTime()
{
    extern MonoServer *g_MonoServer;
    auto nNow = g_MonoServer->GetTickCount();

    if(nNow > m_TimeCheckTick){
        pSession->Send()
    }
}
