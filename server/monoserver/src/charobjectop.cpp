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

void CharObject::on_MPK_QUERYFRIENDTYPE(const MessagePack &rstMPK)
{
    AMQueryFriendType amQFT;
    std::memcpy(&amQFT, rstMPK.Data(), sizeof(amQFT));

    checkFriend(amQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->forward(rstMPK.from(), {MPK_FRIENDTYPE, amFT}, rstMPK.ID());
    });
}
