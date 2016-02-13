#pragma once
#include <cstdint>

// TODO Fucked me
// any change here, must copy to another folder

const int CLIENTMT_CONNECTSUCCEED = 0;
const int CLIENTMT_LOGIN          = 1;
const int CLIENTMT_LOGINFAIL      = 2;
const int CLIENTMT_LOGINSUCCEED   = 3;
const int CLIENTMT_ACTORBASEINFO  = 4;
const int CLIENTMT_MAPERROR       = 5;

#pragma pack(push, 1)

typedef struct{
    char        szID[128];
    char        szPWD[128];
}ClientMessageLogin;

typedef struct{
    char        szCharName[64];
    char        szMapName[64];
    uint32_t    nUID;
    uint32_t    nSID;
    uint32_t    nGenTime;
    uint32_t    nMapX;
    uint32_t    nMapY;
    uint32_t    nDirection;
}ClientMessageLoginSucceed;

typedef struct{
    uint32_t    nSID;
    uint32_t    nUID;
    uint32_t    nGenTime;
    uint32_t    nX;
    uint32_t    nY;
    uint32_t    nState;
    uint32_t    nDirection;
}ClientMessageActorBaseInfo;
#pragma pack(pop)
