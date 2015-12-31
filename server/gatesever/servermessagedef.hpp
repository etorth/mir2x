#pragma once
#include <cstdint>

const int SERVERMT_CONNECTSUCCEED = 0;
const int SERVERMT_LOGIN          = 1;
const int SERVERMT_LOGINFAIL      = 2;
const int SERVERMT_LOGINSUCCEED   = 3;
const int SERVERMT_MAPNAME        = 4;
const int SERVERMT_ACTORBASEINFO  = 5;
const int SERVERMT_BROADCAST      = 6;
const int SERVERMT_ADDHUMAN       = 7;
const int SERVERMT_ADDHUMANERROR  = 8;

#pragma pack(push, 1)
typedef struct{
    char szMapName[128];
}ServerMessageMapName;

typedef struct{
    uint32_t    nSID;
    uint32_t    nUID;
    uint32_t    nGenTime;
    uint32_t    nX;
    uint32_t    nY;
    uint32_t    nState;
    uint32_t    nDirection;
}ServerMessageActorBaseInfo;

typedef struct{
    uint32_t    nSID;  // id of listener, not sender
    uint32_t    nUID;
    uint32_t    nLen;
    uint8_t     Data[512];
}ServerMessageBroadcast;

typedef struct{
    uint32_t    nUID;
}ServerMessageAddHuman;
#pragma pack(pop)
