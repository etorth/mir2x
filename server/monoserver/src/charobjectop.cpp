/*
 * =====================================================================================
 *
 *       Filename: charobjectop.cpp
 *        Created: 03/26/2019 21:37:01
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

#include "charobject.hpp"
#include "messagepack.hpp"
#include "actormessage.hpp"

void CharObject::On_MPK_QUERYFRIENDTYPE(const MessagePack &rstMPK)
{
    AMQueryFriendType stAMQFT;
    std::memcpy(&stAMQFT, rstMPK.Data(), sizeof(stAMQFT));

    CheckFriendType(stAMQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType stAMFT;
        std::memset(&stAMFT, 0, sizeof(stAMFT));

        stAMFT.Type = nFriendType;
        m_actorPod->Forward(rstMPK.From(), {MPK_FRIENDTYPE, stAMFT}, rstMPK.ID());
    });
}
