/*
 * =====================================================================================
 *
 *       Filename: message.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 01/24/2016 19:41:31
 *
 *    Description: net message used by client and mono-server
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

#pragma once

// message without content
const std::pair<uint8_t, size_t> CM_RIDEHORSE   = {0, sizeof(ClientMessageRideHorse)};


// message with content
const uint8_t MESSAGE_NOCONTENT = 128;
inline bool MessageWithContent(uint8_t nMsgID)
{
    return nMsgID >= MESSAGE_NOCONTENT;
}

typedef struc{
    uint8_t  ID;
    uint32_t TimeStamp;
}ClientMessagePing;
const std::pair<uint8_t, size_t> CM_PING = {MESSAGE_NOCONTENT + 0, sizeof(ClientMessagePing)};
