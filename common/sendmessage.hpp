#pragma once
#include "mir2xasio.hpp"

template<typename MsgType>
void SendMessage(const MsgType & stMsg, std::function<void(uint8_t *, size_t)> fnSendBuf)
{
    std::static_assert(sizeof(stMsg) < 255 + 2);
    size_t nSize = sizeof(stMsg) - 2;
    uint8_t chMsgID1 = (uint8_t)((stMsg.ID & 0X00FF));
    uint8_t chMsgID2 = (uint8_t)((stMsg.ID & 0XFF00) >> 8);
    
    fnSendBuf(&chMsgID1, 1);
    if(chMsgID2 != 0){
        std::static_assert(sizeof(MsgType) < 255 + 2);
        fnSendBuf(sizeof(MsgType) - 2, 1);
        fnSendBuf()
    }
}
