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
#include "actormsgpack.hpp"
#include "actormsg.hpp"

void CharObject::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &mpk)
{
    const auto amQFT = mpk.conv<AMQueryFriendType>();
    checkFriend(amQFT.UID, [this, mpk](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->forward(mpk.from(), {AM_FRIENDTYPE, amFT}, mpk.seqID());
    });
}
