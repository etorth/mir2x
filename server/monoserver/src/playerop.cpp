/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 05/11/2016 17:47:33
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

void Player::On_MPK_NETMESSAGE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMNetMessageContext stAMNMC;
    std::memcpy(&stAMNMC, rstMPK.Data(), sizeof(AMNetMessageContext));

    // TODO
    // here we use the buffer in class Session, this is dirty since we are
    // using shared memory, we can copy the net message body in the MPK but
    // I am for performance.
    //
    // to make it safe, we should
    // 1. class Session shouldn't read a new message unless notified by class Player
    // 2. never copy the data pointer stAMNMC.Data
    //
    // aha so smelly here
    OperateNet(stAMNMC.Type, stAMNMC.Data, stAMNMC.DataLen);

    // notify the session that I have consumed this data
    // then session can do everything to the buffer, maybe release it or put new stuff
    // inside
    m_ActorPod->Forward(MPK_OK, rstFromAddr);
}
