#include <SDL.h>
#include <memory.h>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <vector>
#include "triangle.hpp"
#include "mir2clientmap.hpp"
#include "devicemanager.hpp"
#include "texturemanager.hpp"
#include "directiverectcover.hpp"

ClientMap::ClientMap()
    : m_GroundInfo(nullptr)
    , m_BaseTileInfo(nullptr)
    , m_CellDesc(nullptr)
    , m_Valid(false)
    , m_W(0)
    , m_H(0)
    , m_ViewX(0)
    , m_ViewY(0)
{}

ClientMap::~ClientMap()
{
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;
}

bool ClientMap::Load(const char *szFullName)
{
	delete []m_TileDesc; m_TileDesc = nullptr;
    delete []m_CellDesc; m_CellDesc = nullptr;

    FILE *pFile = fopen(szFullName, "rb");
    if(pFile == nullptr){
        return false;
    }

    // file is already aligned to 64 byte
    fseek(pFile, 0, SEEK_END);
    long nSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    uint8_t *pRawData = new uint8_t[nSize];
    fread(pRawData, 1, nSize, pFile);

    //             +--+--+----+-......-+----+---------------------------------------------------
    // byte width: | 2  2  4       x     4
    // zone label: | 1  2  3       4
    //             +----------------------------------------------------------------------------
    //
    //  1: map width, 32 * 24
    //  2: map height
    //  3: size in byte of bit stream for walkable info
    //  4: bit stream for walkable info, recursively defined, length defined in 3


    // read map info
    uint8_t *pMapHead = pRawData;
    m_W = *((uint16_t *)(pMapHead + 0));
    m_H = *((uint16_t *)(pMapHead + 2));

    m_TileDesc = new TILEDESC[m_W * m_H / 4];
    m_CellDesc = new CELLDESC[m_W * m_H    ];

    // read walkable info
    uint8_t  *pWalkInfo    = pMapHead + 4;
    uint32_t  nWalkInfoLen = *((uint32_t *)pWalkInfo);
    uint8_t  *pWalkInfoDat = pWalkInfo + 4;

    LoadWalk(pWalkData);


    {
        uint8_t *pWalkInfoStart


        uint32_t *pU32    = (uint32_t *)(pRawData + 4);
        uint32_t  nBitLen = 0;
        uint32_t  nU32Len = 0;

        nBitLen = *pU32++;
        // now pU32 point to the start of the groundinfo bit stream
        // nBitLen is the length ( without align ) of *bit*
        //
        // now it's start from 64
        // each chunk is aligned by 32
        // but the stream is end of 64 * K

        // for align of 32
        nU32Len = (nBitLen + 31) / 32;

        if(nU32Len% 2 == 0){
            LoadGroundInfo(pU32, nBitLen, pU32 + nU32Len + 2, *(pU32 + nU32Len + 1));
            pU32 += (nU32Len + 1);
        }else{
            LoadGroundInfo(pU32, nBitLen, pU32 + nU32Len + 1, *(pU32 + nU32Len));
            pU32 += nU32Len;
        }

        // this is ground info count
        // we need to skip again
        // if(*pU32 % 2 == 0){
        //     pU32 += (*pU32 + 1);
        // }else{
        //     pU32 += (*pU32);
        // }

        pU32   += (1 + *pU32 + (1 - (*pU32 % 2)));
        nBitLen = *pU32++;

        // for align of 32
        if(nBitLen % 32){
            nU32Len = (nBitLen / 32 + 1);
        }else{
            nU32Len = nBitLen / 32;
        }

        if(nU32Len% 2 == 0){
            LoadBaseTileInfo(pU32, nBitLen, pU32 + nU32Len + 2, *(pU32 + nU32Len + 1));
            pU32 += (nU32Len + 1);
        }else{
            LoadBaseTileInfo(pU32, nBitLen, pU32 + nU32Len + 1, *(pU32 + nU32Len));
            pU32 += nU32Len;
        }

        pU32   += (1 + *pU32 + (1 - (*pU32 % 2)));
        nBitLen = *pU32++;

        // for align of 32
        if(nBitLen % 32){
            nU32Len = (nBitLen / 32 + 1);
        }else{
            nU32Len = nBitLen / 32;
        }

        if(nU32Len% 2 == 0){
            LoadCellDesc(pU32, nBitLen, (CELLDESC *)(pU32 + nU32Len + 2), *(pU32 + nU32Len + 1));
            pU32 += (nU32Len + 1);
        }else{
            LoadCellDesc(pU32, nBitLen, (CELLDESC *)(pU32 + nU32Len + 1), *(pU32 + nU32Len));
            pU32 += nU32Len;
        }
    }

    m_Valid = true;
    return true;
}

int ClientMap::W()
{
    return m_W;
}

int ClientMap::H()
{
    return m_H;
}

bool ClientMap::Valid()
{
    return m_Valid;
}

void ClientMap::SetOneWalk(int nStartX, int nStartY, int nSubGrid, bool bAttr)
{
    int nOffset = nStartY * m_W + nStartX;
    m_CellDesc[nOffset].Desc |= (uint8_t)(1 << (nSubGrid % 4));
}

void ClientMap::SetWalk(int nStartX, int nStartY, int nSize, bool bAttr)
{
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            SetOneWalk(nX, nY, 0, bAttr);
            SetOneWalk(nX, nY, 1, bAttr);
            SetOneWalk(nX, nY, 2, bAttr);
            SetOneWalk(nX, nY, 3, bAttr);
        }
    }
}


void ClientMap::SetOneWalk(int nStartX, int nStartY, int nSubGrid, bool bAttr)
{
    int nOffset = nStartY * m_W + nStartX;
    m_CellDesc[nOffset].Desc |= (uint8_t)(1 << (nSubGrid % 4));
}

void ClientMap::SetWalk(int nStartX, int nStartY, int nSize, bool bAttr)
{
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            SetOneWalk(nX, nY, 0, bAttr);
            SetOneWalk(nX, nY, 1, bAttr);
            SetOneWalk(nX, nY, 2, bAttr);
            SetOneWalk(nX, nY, 3, bAttr);
        }
    }
}

void ClientMap::ParseWalk(int nStartX, int nStartY, uint8_t *pData, long &nBitOffset, int nSize)
{
    if(nStartX < m_W && nStartY < m_H){
        if(PickOneBit(pData, nBitOffset++)){
            // get a combined grid
            if(nSize == 1){
                // last level of grid, four smallest subgrid with X-cross division
                if(PickOneBit(pData, nBitOffset++)){
                    // still combined at last level
                    SetOneWalk(nStartX, nStartY, 0, PickOneBit(pData, nBitOffset++));
                    SetOneWalk(nStartX, nStartY, 1, PickOneBit(pData, nBitOffset++));
                    SetOneWalk(nStartX, nStartY, 2, PickOneBit(pData, nBitOffset++));
                    SetOneWalk(nStartX, nStartY, 3, PickOneBit(pData, nBitOffset++));
                }else{
                    // last level of grid is empty
                    SetWalk(nStartX, nStartY, 1, 0);
                }
            }else{
                // combined but not the last level, then recursively check
                ParseWalk(nStartX,             nStartY,             pData, nBitOffset, nSize / 2);
                ParseWalk(nStartX + nSize / 2, nStartY,             pData, nBitOffset, nSize / 2);
                ParseWalk(nStartX,             nStartY + nSize / 2, pData, nBitOffset, nSize / 2);
                ParseWalk(nStartX + nSize / 2, nStartY + nSize / 2, pData, nBitOffset, nSize / 2);
            }
        }else{
            SetWalk(nStartX, nStartY, nSize, 0);
        }
    }
}

void ClientMap::ParseGroundInfoStream(int nStartX, int nStartY, int nSize,
        uint32_t *pU32BitStream,  uint32_t &nU32BitStreamOffset,
        uint32_t *pU32GroundInfo, uint32_t &nU32GroundInfoOffset)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position
    if(BitPickOne(pU32BitStream, nU32BitStreamOffset++) == 0){
        // here use 1 bit for null block
        // saving bits, since we assume most of them are null block

        // nothing, no matter nSize == 1 or not
        SetGroundInfoBlock(nStartX, nStartY, nSize, 0XFFFFFFFF);
    }else{
        // have something
        // maybe unique but walkable, or combined
        if(BitPickOne(pU32BitStream, nU32BitStreamOffset++) == 0){
            // unique, walkable
            SetGroundInfoBlock(nStartX, nStartY, nSize,
                    pU32GroundInfo[nU32GroundInfoOffset++]);
        }else{
            // combined
            if(nSize == 1){
                for(int nCnt = 0; nCnt < 4; ++nCnt){
                    if(BitPickOne(pU32BitStream, nU32BitStreamOffset++)){
                        SetOneGroundInfoGrid(nStartX, nStartY, nCnt, 0XFFFFFFFF);
                    }else{
                        SetOneGroundInfoGrid(nStartX, nStartY, nCnt,
                                pU32GroundInfo[nU32GroundInfoOffset++]);
                    }
                }
            }else{
                // recursively invoke
                ParseGroundInfoStream(nStartX, nStartY, nSize / 2,
                        pU32BitStream,  nU32BitStreamOffset,
                        pU32GroundInfo, nU32GroundInfoOffset);

                ParseGroundInfoStream(nStartX + nSize / 2, nStartY, nSize / 2,
                        pU32BitStream,  nU32BitStreamOffset,
                        pU32GroundInfo, nU32GroundInfoOffset);

                ParseGroundInfoStream(nStartX, nStartY + nSize / 2, nSize / 2,
                        pU32BitStream,  nU32BitStreamOffset,
                        pU32GroundInfo, nU32GroundInfoOffset);

                ParseGroundInfoStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                        pU32BitStream,  nU32BitStreamOffset,
                        pU32GroundInfo, nU32GroundInfoOffset);
            }
        }
    }
}


void ClientMap::LoadWalk(uint8_t * pWalkData)
{
    long nBitOffset = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseWalk(nBlkX * 8, nBlkY * 8, pWalkData, nBitOffset);
        }
    }
}

void ClientMap::SetBaseTileBlock(
        int nStartX, int nStartY, int nSize, uint32_t nAttr)
{
    for(int nY = nStartY; nY < nStartY + nSize; nY += 2){
        for(int nX = nStartX; nX < nStartX + nSize; nX += 2){
            m_BaseTileInfo[(nY / 2) * (m_W / 2) + (nX / 2)] = nAttr;
        }
    }
}

void ClientMap::ParseBaseTileStream(int nStartX, int nStartY, int nSize,
        uint32_t *pU32BitStream,    uint32_t &nU32BitStreamOffset,
        uint32_t *pU32BaseTileInfo, uint32_t &nU32BaseTileInfoOffset)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position

    if(BitPickOne(pU32BitStream, nU32BitStreamOffset++) == 0){
        // no tile in this block, no matter whether nSize == 1 or not
        SetBaseTileBlock(nStartX, nStartY, nSize, 0XFFFFFFFF);
    }else{
        if(nSize == 2){
            // currently there may be object and nSize == 1
            SetBaseTileBlock(nStartX, nStartY, nSize, pU32BaseTileInfo[nU32BaseTileInfoOffset++]);
        }else{
            // currently there may be object and nSize > 1
            // recursively parse sub-block
            ParseBaseTileStream(nStartX, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);

            ParseBaseTileStream(nStartX + nSize / 2, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);

            ParseBaseTileStream(nStartX, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);

            ParseBaseTileStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);
        }
    }
}

bool ClientMap::LoadBaseTileInfo(
        uint32_t *pU32BitStream,    uint32_t nU32BitStreamLen,
        uint32_t *pU32BaseTileInfo, uint32_t nU32BaseTileInfoLen)
{

    uint32_t nU32BitStreamOffset    = 0;
    uint32_t nU32BaseTileInfoOffset = 0;
    for(int nBlkY = 0; nBlkY < m_H / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < m_W / 8; ++nBlkX){
            ParseBaseTileStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);
        }
    }
    return true;
}

void ClientMap::SetCellDescBlock(
        int nStartX, int nStartY, int nSize, const CELLDESC & stCellDesc)
{
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            m_CellDesc[nY * m_W + nX] = stCellDesc;
        }
    }
}

void ClientMap::ParseCellDescStream(int nStartX, int nStartY, int nSize,
        uint32_t *pU32BitStream, uint32_t &nU32BitStreamOffset,
        CELLDESC *pCellDesc, uint32_t &nCellDescOffset)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position

    if(BitPickOne(pU32BitStream, nU32BitStreamOffset++) == 0){
        // no object in this block, no matter whether nSize == 1 or not
        SetCellDescBlock(nStartX, nStartY, nSize, {
                0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF});
    }else{
        if(nSize == 1){
            // currently there may be object and nSize == 1
            SetCellDescBlock(nStartX, nStartY, nSize, pCellDesc[nCellDescOffset++]);
        }else{
            // currently there may be object and nSize > 1
            // recursively parse sub-block
            ParseCellDescStream(nStartX, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);

            ParseCellDescStream(nStartX + nSize / 2, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);

            ParseCellDescStream(nStartX, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);

            ParseCellDescStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);
        }
    }
}

bool ClientMap::LoadCellDesc(
        uint32_t *pU32BitStream, uint32_t nU32BitStreamLen,
        CELLDESC *pCellDesc,     uint32_t nCellDescLen)
{

    uint32_t nU32BitStreamOffset = 0;
    uint32_t nCellDescOffset     = 0;
    for(int nBlkY = 0; nBlkY < m_H / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < m_W / 8; ++nBlkX){
            ParseCellDescStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);
        }
    }
    return true;
}

bool ClientMap::ValidPosition(int nX, int nY, Monster *pMonster)
{
    return true;
}

uint32_t ClientMap::BaseTileInfo(int nX, int nY)
{
    return m_BaseTileInfo[(nX / 2) + (nY / 2) * (m_W / 2)];
}


void ClientMap::DrawBaseTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY)
{
    for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
        for(int nX = nStartCellX; nX < nStopCellX; ++nX){
            if(nX % 2 || nY % 2){
                continue;
            }

            uint32_t nBaseTileInfo = BaseTileInfo(nX, nY);
            if(nBaseTileInfo == 0XFFFFFFFF){
                continue;
            }

            SDL_Texture *pTexture = GetTextureManager()->RetrieveTexture(nBaseTileInfo);
            if(pTexture){
                int nW, nH;
                SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);
                SDL_Rect stRectDst = { nX * 48 - m_ViewX, nY * 32 - m_ViewY, nW, nH};
                SDL_RenderCopy(GetDeviceManager()->GetRenderer(), pTexture, nullptr, &stRectDst);
            }
        }
    }
}

bool ClientMap::Overlap(const DirectiveRectCover &stDRC)
{
    Triangle stTri1(
            stDRC.Point(0).first, stDRC.Point(0).second,
            stDRC.Point(1).first, stDRC.Point(1).second,
            stDRC.Point(2).first, stDRC.Point(2).second);
    Triangle stTri2(
            stDRC.Point(0).first, stDRC.Point(0).second,
            stDRC.Point(3).first, stDRC.Point(3).second,
            stDRC.Point(2).first, stDRC.Point(2).second);
    return Overlap(stTri1) || Overlap(stTri2);
}

bool ClientMap::Overlap(const Triangle &stTri)
{
    double fMinX = stTri.MinX();
    double fMinY = stTri.MinY();
    double fMaxX = stTri.MaxX();
    double fMaxY = stTri.MaxY();

    int nCellStartX = (int)(std::lround(fMinX) / 48);
    int nCellStartY = (int)(std::lround(fMinY) / 32);
    int nCellStopX  = (int)(std::lround(fMaxX) / 48);
    int nCellStopY  = (int)(std::lround(fMaxY) / 32);

    for(int nX = nCellStartX; nX <= nCellStopX; ++nX){
        for(int nY = nCellStartY; nY <= nCellStopY; ++nY){
            double fX[4] = {(nX + 0) * 48.0, (nX + 1) * 48.0, (nX + 1) * 48.0, (nX + 0) * 48.0};
            double fY[4] = {(nY + 0) * 32.0, (nY + 0) * 32.0, (nY + 1) * 32.0, (nY + 1) * 32.0};
            double fMidX = (nX + 0.5) * 48.0;
            double fMidY = (nY + 0.5) * 48.0;

            for(int nIndex = 0; nIndex < 4; ++nIndex){
                if(m_GroundInfo[(nX + nY * m_W) * 4 + nIndex] == 0XFFFFFFFF){
                    if(stTri.Overlap(Triangle(fMidX, fMidY, fX[nIndex], fY[nIndex],
                                    fX[(nIndex + 1) % 4], fY[(nIndex + 1) % 4]))){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ClientMap::DrawObject(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<bool(int, int)> fnCheckFunc,
        std::function<void(int, int)> fnExtDrawFunc)
{
    if(m_Valid){
        for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
            for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
                auto &stCellDesc = m_CellDesc[nXCnt + nYCnt * m_W];
                if(true
                        && stCellDesc.dwDesc    == 0XFFFFFFFF
                        && stCellDesc.dwObject1 == 0XFFFFFFFF
                        && stCellDesc.dwObject2 == 0XFFFFFFFF
                        && stCellDesc.dwLight   == 0XFFFFFFFF
                  ){
                    goto __Mir2ClientMap_DrawObject_ExtDrawFunc;
                    // continue;
                }

                uint32_t nPrecodeV[2] = {
                    (((stCellDesc.dwObject1 & 0XFC000000) >> 26)),
                    (((stCellDesc.dwObject2 & 0XFC000000) >> 26))};
                uint32_t nFileIndexV[2] = {
                    (((stCellDesc.dwObject1 & 0X03FF0000) >> 16)),
                    (((stCellDesc.dwObject2 & 0X03FF0000) >> 16))};
                uint32_t nImageIndexV[2] = {
                    (((stCellDesc.dwObject1 & 0X0000FFFF))),
                    (((stCellDesc.dwObject2 & 0X0000FFFF)))};
                uint32_t nObjectDescV[2] = {
                    (((stCellDesc.dwDesc & 0X000000FF))),
                    (((stCellDesc.dwDesc & 0X0000FF00) >> 8))};

                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    uint32_t nPrecode    = nPrecodeV[nIndex];
                    uint32_t nFileIndex  = nFileIndexV[nIndex];
                    uint32_t nImageIndex = nImageIndexV[nIndex];
                    uint32_t nObjectDesc = nObjectDescV[nIndex];

                    if(nFileIndex != 255 && nImageIndex != 65535){
                        nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                        if(nObjectDesc != 255){
                            uint8_t bTickType = (nObjectDesc & 0X70) >> 4;
                            int16_t shAniCnt  = (nObjectDesc & 0X0F);

                            nImageIndex += m_bAniTileFrame[bTickType][shAniCnt];
                        }

                        SDL_Texture *pTexture = 
                            GetTextureManager()->RetrieveTexture(nPrecode, nFileIndex, nImageIndex);

                        { // internal draw
                            int nW, nH;
                            SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);
                            if(fnCheckFunc && !fnCheckFunc(nW, nH)){
                                goto __Mir2ClientMap_DrawObject_ExtDrawFunc;
                                // continue;
                            }

                            SDL_Rect stDst = {
                                nXCnt * 48 - m_ViewX, nYCnt * 32 + 32 - nH - m_ViewY, nW, nH};
                            SDL_RenderCopy(
                                    GetDeviceManager()->GetRenderer(), pTexture, nullptr, &stDst);
                        }

                    }
                }
__Mir2ClientMap_DrawObject_ExtDrawFunc: ;
                                        if(fnExtDrawFunc){
                                            fnExtDrawFunc(nXCnt, nYCnt);
                                        }
            }
        }
    }
}

void ClientMap::DrawGroundObject(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY)
{
    auto fnCheckFunc = [](int nW, int nH){
        return nW == 48 && nH == 32;
    };
    auto fnExtDrawFunc = [](int, int){};
    DrawObject(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnExtDrawFunc);
}

void ClientMap::DrawOverGroundObject(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY, 
        std::function<void(int, int)> fnExtDrawFunc)
{
    auto fnCheckFunc = [](int nW, int nH){
        return nW != 48 || nH != 32;
    };
    DrawObject(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnExtDrawFunc);
}

uint32_t ClientMap::GetDoorImageIndex(int nX, int nY)
{
    uint32_t  nDoorIndex = 0;
    auto     &stCellDesc = m_CellDesc[nX + nY * m_W];
    if((stCellDesc.dwDesc & 0XFF000000) > 0){
        nDoorIndex += ((stCellDesc.dwDesc & 0X007F0000) >> 16);
        // printf("be careful: GetDoorImageIndex() returns non-zero value !!!\n");
    }
    return nDoorIndex;
}

void ClientMap::SetViewPoint(int nX, int nY)
{
    m_ViewX = nX;
    m_ViewY = nY;
}

int ClientMap::ViewX()
{
    return m_ViewX;
}

int ClientMap::ViewY()
{
    return m_ViewY;
}

bool ClientMap::ValidPosition()
