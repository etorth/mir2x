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

    uint8_t *pCurDat = pRawData;
    LoadHead(pCurDat);
    LoadWalk(pCurDat);
    LoadLight(pCurDat);
    LoadTile(pCurDat);
    LoadObj1(pCurDat);
    LoadObj2(pCurDat);






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
    int nOff = nStartY * m_W + nStartX;
    m_CellDesc[nOff].Desc |= (uint8_t)(1 << (nSubGrid % 4));
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
    int nOff = nStartY * m_W + nStartX;
    m_CellDesc[nOff].Desc |= (uint8_t)(1 << (nSubGrid % 4));
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


void ClientMap::ParseWalk(int nStartX, int nStartY, int nSize, uint8_t *pMark, long &nMarkOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    if(nStartX < m_W && nStartY < m_H){
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid consists of four smallest subgrids divided by X-cross
                if(PickOneBit(pData, nBitOff++)){
                    // it's combined at last level
                    SetOneWalk(nStartX, nStartY, 0, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nStartX, nStartY, 1, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nStartX, nStartY, 2, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nStartX, nStartY, 3, PickOneBit(pData, nBitOff++));
                }else{
                    // it's not combined, and there is info, so it only can be all full-filled
                    SetWalk(nStartX, nStartY, 1, true);
                }
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
                    // there is data, and it's combined, need further parsing
                    ParseWalk(nStartX,             nStartY,             nSize / 2, pMark, nMarkOff);
                    ParseWalk(nStartX + nSize / 2, nStartY,             nSize / 2, pMark, nMarkOff);
                    ParseWalk(nStartX,             nStartY + nSize / 2, nSize / 2, pMark, nMarkOff);
                    ParseWalk(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2, pMark, nMarkOff);
                }else{
                    // there is information, but it's not combined, can only be full-filled
                    SetWalk(nStartX, nStartY, nSize, true);
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetWalk(nStartX, nStartY, nSize, false);
        }
    }
}

void ClientMap::ParseMarkData(
        int nStartX, int nStartY,
        int nSize,                      // current size
        int nLastSize,                  // smallest size consists a atomic element
        uint8_t *pMark, long &nMarkOff,
        uint8_t *pData, long &nDataOff,
        std::function<void()> fnDoSetBaseElement,
        std::function<void()> fnUnSetBaseElement,
        std::function<void()> fnUnSetElement,
        )
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    if(nStartX < m_W && nStartY < m_H){
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                SetOneData(nStartX, nStartY, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseLight(nStartX,             nStartY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX + nSize / 2, nStartY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX,             nStartY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    SetLight(nStartX, nStartY, nSize, pData, nDataOff);
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetLight(nStartX, nStartY, nSize, nullptr, nDataOff);
        }
    }
}

void ClientMap::ParseLight(int nStartX, int nStartY, int nSize,
        uint8_t *pMark, long &nMarkOff, uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    if(nStartX < m_W && nStartY < m_H){
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                SetOneLight(nStartX, nStartY, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseLight(nStartX,             nStartY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX + nSize / 2, nStartY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX,             nStartY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    SetLight(nStartX, nStartY, nSize, pData, nDataOff);
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetLight(nStartX, nStartY, nSize, nullptr, nDataOff);
        }
    }
}

void ClientMap::ParseGroundInfoStream(int nStartX, int nStartY, int nSize,
        uint32_t *pU32BitStream,  uint32_t &nU32BitStreamOff,
        uint32_t *pU32GroundInfo, uint32_t &nU32GroundInfoOff)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position
    if(BitPickOne(pU32BitStream, nU32BitStreamOff++) == 0){
        // here use 1 bit for null block
        // saving bits, since we assume most of them are null block

        // nothing, no matter nSize == 1 or not
        SetGroundInfoBlock(nStartX, nStartY, nSize, 0XFFFFFFFF);
    }else{
        // have something
        // maybe unique but walkable, or combined
        if(BitPickOne(pU32BitStream, nU32BitStreamOff++) == 0){
            // unique, walkable
            SetGroundInfoBlock(nStartX, nStartY, nSize,
                    pU32GroundInfo[nU32GroundInfoOff++]);
        }else{
            // combined
            if(nSize == 1){
                for(int nCnt = 0; nCnt < 4; ++nCnt){
                    if(BitPickOne(pU32BitStream, nU32BitStreamOff++)){
                        SetOneGroundInfoGrid(nStartX, nStartY, nCnt, 0XFFFFFFFF);
                    }else{
                        SetOneGroundInfoGrid(nStartX, nStartY, nCnt,
                                pU32GroundInfo[nU32GroundInfoOff++]);
                    }
                }
            }else{
                // recursively invoke
                ParseGroundInfoStream(nStartX, nStartY, nSize / 2,
                        pU32BitStream,  nU32BitStreamOff,
                        pU32GroundInfo, nU32GroundInfoOff);

                ParseGroundInfoStream(nStartX + nSize / 2, nStartY, nSize / 2,
                        pU32BitStream,  nU32BitStreamOff,
                        pU32GroundInfo, nU32GroundInfoOff);

                ParseGroundInfoStream(nStartX, nStartY + nSize / 2, nSize / 2,
                        pU32BitStream,  nU32BitStreamOff,
                        pU32GroundInfo, nU32GroundInfoOff);

                ParseGroundInfoStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                        pU32BitStream,  nU32BitStreamOff,
                        pU32GroundInfo, nU32GroundInfoOff);
            }
        }
    }
}


void ClientMap::LoadMarkData(uint8_t *pMark, uint8_t *pData)
{
    long nMarkOff = 0;
    long nByteOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseMarkData(nBlkX * 8, nBlkY * 8, pMark, pData);
        }
    }
}

void ClientMap::LoadLight(uint8_t *pLigthMark, uint8_t *pLightData)
{
    long nMarkOff = 0;
    long nByteOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseLight(nBlkX * 8, nBlkY * 8, pLigthMark, pLightData);
        }
    }
}

void ClientMap::LoadWalk(uint8_t * pWalkData)
{
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
        uint32_t *pU32BitStream,    uint32_t &nU32BitStreamOff,
        uint32_t *pU32BaseTileInfo, uint32_t &nU32BaseTileInfoOff)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position

    if(BitPickOne(pU32BitStream, nU32BitStreamOff++) == 0){
        // no tile in this block, no matter whether nSize == 1 or not
        SetBaseTileBlock(nStartX, nStartY, nSize, 0XFFFFFFFF);
    }else{
        if(nSize == 2){
            // currently there may be object and nSize == 1
            SetBaseTileBlock(nStartX, nStartY, nSize, pU32BaseTileInfo[nU32BaseTileInfoOff++]);
        }else{
            // currently there may be object and nSize > 1
            // recursively parse sub-block
            ParseBaseTileStream(nStartX, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pU32BaseTileInfo, nU32BaseTileInfoOff);

            ParseBaseTileStream(nStartX + nSize / 2, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pU32BaseTileInfo, nU32BaseTileInfoOff);

            ParseBaseTileStream(nStartX, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pU32BaseTileInfo, nU32BaseTileInfoOff);

            ParseBaseTileStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pU32BaseTileInfo, nU32BaseTileInfoOff);
        }
    }
}

bool ClientMap::LoadBaseTileInfo(
        uint32_t *pU32BitStream,    uint32_t nU32BitStreamLen,
        uint32_t *pU32BaseTileInfo, uint32_t nU32BaseTileInfoLen)
{

    uint32_t nU32BitStreamOff    = 0;
    uint32_t nU32BaseTileInfoOff = 0;
    for(int nBlkY = 0; nBlkY < m_H / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < m_W / 8; ++nBlkX){
            ParseBaseTileStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOff,
                    pU32BaseTileInfo, nU32BaseTileInfoOff);
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
        uint32_t *pU32BitStream, uint32_t &nU32BitStreamOff,
        CELLDESC *pCellDesc, uint32_t &nCellDescOff)
{
    // when getting inside, offset is at current position
    // when exited, offset is at next valid position

    if(BitPickOne(pU32BitStream, nU32BitStreamOff++) == 0){
        // no object in this block, no matter whether nSize == 1 or not
        SetCellDescBlock(nStartX, nStartY, nSize, {
                0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF});
    }else{
        if(nSize == 1){
            // currently there may be object and nSize == 1
            SetCellDescBlock(nStartX, nStartY, nSize, pCellDesc[nCellDescOff++]);
        }else{
            // currently there may be object and nSize > 1
            // recursively parse sub-block
            ParseCellDescStream(nStartX, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pCellDesc, nCellDescOff);

            ParseCellDescStream(nStartX + nSize / 2, nStartY, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pCellDesc, nCellDescOff);

            ParseCellDescStream(nStartX, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pCellDesc, nCellDescOff);

            ParseCellDescStream(nStartX + nSize / 2, nStartY + nSize / 2, nSize / 2,
                    pU32BitStream,  nU32BitStreamOff,
                    pCellDesc, nCellDescOff);
        }
    }
}

bool ClientMap::LoadCellDesc(
        uint32_t *pU32BitStream, uint32_t nU32BitStreamLen,
        CELLDESC *pCellDesc,     uint32_t nCellDescLen)
{

    uint32_t nU32BitStreamOff = 0;
    uint32_t nCellDescOff     = 0;
    for(int nBlkY = 0; nBlkY < m_H / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < m_W / 8; ++nBlkX){
            ParseCellDescStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOff,
                    pCellDesc, nCellDescOff);
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
{

}

void ClientMap::LoadHead(uint8_t * &pData)
{
    m_W = *((uint16_t *)pData); pData += 2;
    m_H = *((uint16_t *)pData); pData += 2;

    m_TileDesc = new TILEDESC[m_W * m_H / 4];
    m_CellDesc = new CELLDESC[m_W * m_H    ];
}

void ClientMap::LoadWalk(uint8_t * &pData)
{
    long nBitOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseWalk(nBlkX * 8, nBlkY * 8, pData + 4, nBitOff);
        }
    }
    pData += (4 + *((uint32_t *)pData));
}

void ClientMap::LoadLight(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4 + nMarkLen));

    long nMarkOff = 0;
    long nDataOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseLight(nBlkX * 8, nBlkY * 8, pData + 4, nMarkOff, pData + 8 + nMarkLen, nDataLen);
        }
    }
    pData += (8 + nMarkLen + nDataLen);
}
