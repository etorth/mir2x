#include <memory.h>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <vector>
#include "triangle.hpp"
#include "mir2xmap.hpp"
#include "mathfunc.hpp"

Mir2xMap::Mir2xMap()
    : m_TileDesc(nullptr)
    , m_CellDesc(nullptr)
    , m_Buf(nullptr)
    , m_BufSize(0)
    , m_BufMaxSize(0)
    , m_W(0)
    , m_H(0)
{}

Mir2xMap::~Mir2xMap()
{
    delete []m_TileDesc; m_TileDesc = nullptr;
    delete []m_CellDesc; m_CellDesc = nullptr;
}

bool Mir2xMap::Load(const char *szFullName)
{
	delete []m_TileDesc; m_TileDesc = nullptr;
    delete []m_CellDesc; m_CellDesc = nullptr;

    FILE *pFile = fopen(szFullName, "rb");
    if(pFile == nullptr){
        return false;
    }

    fseek(pFile, 0, SEEK_END);
    long nSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    ExtendBuf(nSize);
    fread(m_Buf, 1, nSize, pFile);

    uint8_t *pCurDat = m_Buf;
    return LoadHead(pCurDat) && LoadWalk(pCurDat) 
        && LoadLight(pCurDat) && LoadTile(pCurDat) && LoadObj(pCurDat, 0) && LoadObj(pCurDat, 1);
}

void Mir2xMap::SetOneWalk(int nX, int nY, int nSubGrid, bool bCanWalk)
{
    uint16_t nMask = (uint16_t)(0X0001 << (nSubGrid % 4));
    if(bCanWalk){
        CellDesc(nX, nY).Desc |= nMask;
    }else{
        uint16_t nRMask = (uint16_t)(0XFFFF ^ nMask);
        CellDesc(nX, nY).Desc &= nRMask;
    }
}

void Mir2xMap::SetWalk(int nX, int nY, int nSize, bool bCanWalk)
{
    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            SetOneWalk(nTX, nTY, 0, bCanWalk);
            SetOneWalk(nTX, nTY, 1, bCanWalk);
            SetOneWalk(nTX, nTY, 2, bCanWalk);
            SetOneWalk(nTX, nTY, 3, bCanWalk);
        }
    }
}

void Mir2xMap::ParseWalk(int nX, int nY, int nSize, const uint8_t *pMark, long &nMarkOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    if(nX < m_W && nY < m_H){
        if(PickOneBit(pMark, nMarkOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid consists of four smallest subgrids divided by X-cross
                if(PickOneBit(pMark, nMarkOff++)){
                    // it's combined at last level
                    SetOneWalk(nX, nY, 0, PickOneBit(pMark, nMarkOff++));
                    SetOneWalk(nX, nY, 1, PickOneBit(pMark, nMarkOff++));
                    SetOneWalk(nX, nY, 2, PickOneBit(pMark, nMarkOff++));
                    SetOneWalk(nX, nY, 3, PickOneBit(pMark, nMarkOff++));;
                }else{
                    // it's not combined, and there is info, so it only can be all full-filled
                    SetWalk(nX, nY, 1, true);
                }
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pMark, nMarkOff++)){
                    // there is data, and it's combined, need further parsing
                    ParseWalk(nX,             nY,             nSize / 2, pMark, nMarkOff);
                    ParseWalk(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff);
                    ParseWalk(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff);
                    ParseWalk(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff);
                }else{
                    // there is information, but it's not combined, can only be full-filled
                    SetWalk(nX, nY, nSize, true);
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetWalk(nX, nY, nSize, false);
        }
    }
}

void Mir2xMap::SetOneObj(int nX, int nY, int nObjIndex,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    if(pMark && pData){
        CellDesc(nX, nY).Obj[nObjIndex].Desc       = pData[nDataOff++];
        CellDesc(nX, nY).Obj[nObjIndex].FileIndex  = pData[nDataOff++];
        CellDesc(nX, nY).Obj[nObjIndex].ImageIndex = *((uint16_t *)(pData + nDataOff));
        pData += 2;

        SetOneObjMask(nX, nY, nObjIndex, true, PickOneBit(pMark, nMarkOff++));
    }else{
        SetOneObjMask(nX, nY, nObjIndex, false, true);
    }
}

void Mir2xMap::SetOneObjMask(int nX, int nY, int nObjIndex, bool bIsObj, bool bWallLayer)
{
    uint16_t nMask = 0;

    if(bIsObj){
        nMask = ((bWallLayer ? 0X0300 : 0X0200) << (nObjIndex * 2));
    }

    CellDesc(nX, nY).Desc |= nMask;
}

void Mir2xMap::SetObj(int nX, int nY, int nSize, int nObjIndex,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // full-fill current grid defined by parameters
    // obj has ground / wall layer, so need further parse mark data
    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            SetOneObj(nTX, nTY, nObjIndex, pMark, nMarkOff, pData, nDataOff);
        }
    }
}

void Mir2xMap::ParseLight(int nX, int nY, int nSize,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    //
    // 1: lights in full-filled grid are of different attributes
    // 0: no
    if(ValidC(nX, nY)){
        if(PickOneBit(pMark, nMarkOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                SetLight(nX, nY, 1, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pMark, nMarkOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseLight(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    // ask one more bit for light, here we use recursively defined data stream
                    // since most likely lights in full-filled grid are of the same attributes
                    if(PickOneBit(pMark, nMarkOff++)){
                        // full-filled grid with lights of different attributes
                        // this rarely happens
                        ParseLight(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseLight(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseLight(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    }else{
                        // full-filled grid with lights of the same attributes
                        SetLight(nX, nY, nSize, pData, nDataOff);
                    }
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetLight(nX, nY, nSize, nullptr, nDataOff);
        }
    }
}

void Mir2xMap::SetLight(int nX, int nY, int nSize, const uint8_t *pData, long &nDataOff)
{
    // full-filled current grid defined by the same attributes
    uint16_t nLightAttr = 0;

    if(pData){
        nLightAttr = *((uint16_t *)(pData + nDataOff));
        nDataOff += 2;
    }

    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            if(pData){
                CellDesc(nTX, nTY).Desc |= 0X8000;
            }else{
                CellDesc(nTX, nTY).Desc &= 0X7FFF;
            }
            CellDesc(nTX, nTY).Light = nLightAttr;
        }
    }
}

void Mir2xMap::DrawOverGroundObj(int nViewX, int nViewY, int nViewW, int nViewH, int nMaxObjH,
        std::function<void(int, int, uint32_t)> fnDrawObjFunc, std::function<void(int, int)> fnDrawActorFunc)
{
    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            if(OverGroundObjValid(nCellX, nCellY, 0)){
                fnDrawObjFunc(nCellX * 48, nCellY * 32, CellObjKey(nCellX, nCellY, 0));
            }

            if(OverGroundObjValid(nCellX, nCellY, 1)){
                fnDrawObjFunc(nCellX * 48, nCellY * 32, CellObjKey(nCellX, nCellY, 1));
            }
            fnDrawActorFunc(nCellX, nCellY);
        }
    }
}


void Mir2xMap::DrawGroundObj(int nViewX, int nViewY, int nViewW, int nViewH, int nMaxObjH,
        std::function<void(int, int, uint32_t)> fnDrawFunc)
{
    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;


    // typical fnDrawFunc should be
    // auto fnDrawFunc = [this](int nX, int nY, uint32_t nKey){
    //     auto stItor = m_ObjTexCache.find(nKey);
    //     if(stItor != m_ObjTexCache.end()){
    //         int nW, nH;
    //         SDL_QueryTexture(stItor->second, &nW, &nH);
    //         SDL_Rect stDst = {nX - nViewX, nY + 32 - nH - m_ViewY, nW, nH};
    //         SDL_RenderCopy(m_Renderer, stItor->second, &stDst, nullptr);
    //     }
    // };

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            if(GroundObjValid(nCellX, nCellY, 0)){
                fnDrawFunc(nCellX * 48, nCellY * 32, CellObjKey(nCellX, nCellY, 0));
            }

            if(GroundObjValid(nCellX, nCellY, 1)){
                fnDrawFunc(nCellX * 48, nCellY * 32, CellObjKey(nCellX, nCellY, 1));
            }
        }
    }
}

void Mir2xMap::Draw(
        int nViewX, int nViewY, int nViewW, int nViewH,      //
        int nMaxObjH,                                       // 
        std::function<void(int, int, uint32_t)> fnDrawTileFunc,
        std::function<void(int, int, uint32_t)> fnDrawObjFunc,
        std::function<void(int, int)> fnDrawActorFunc,
        std::function<void(int, int)> fnDrawExtFunc)
{
    DrawGround(nViewX, nViewY, nViewW, nViewH, fnDrawTileFunc);
    DrawGroundObj(nViewX, nViewY, nViewW, nViewH, nMaxObjH, fnDrawObjFunc);
    DrawOverGroundObj(nViewX, nViewY, nViewW, nViewH, nMaxObjH, fnDrawObjFunc, fnDrawActorFunc);
    DrawExt(nViewX, nViewY, nViewW, nViewH, fnDrawExtFunc);
}


void Mir2xMap::DrawExt(int nViewX, int nViewY, int nViewW, int nViewH,
        std::function<void(int, int)> fnDrawExtFunc)
{
    // a typical fnDrawFunc should be
    // auto fnDrawFunc = [this](int nX, int nY, uint32_t nKey){
    //     auto stItor = m_TileTexCache.find(nKey);
    //     if(stItor != m_TileTexCache.end()){
    //         m_DeviceManager->Render(stItor.second, nX, nY, 96, 64, 0, 0, 96, 64);
    //     }
    // };

    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            fnDrawExtFunc(nCellX, nCellY);
        }
    }
}

void Mir2xMap::DrawGround(int nViewX, int nViewY, int nViewW, int nViewH,
        std::function<void(int, int, uint32_t)> fnDrawFunc)
{
    // a typical fnDrawFunc should be
    // auto fnDrawFunc = [this](int nX, int nY, uint32_t nKey){
    //     auto stItor = m_TileTexCache.find(nKey);
    //     if(stItor != m_TileTexCache.end()){
    //         m_DeviceManager->Render(stItor.second, nX, nY, 96, 64, 0, 0, 96, 64);
    //     }
    // };

    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            if(!(nCellY % 2) && !(nCellX % 2) && TileValid(nCellX, nCellY)){
                fnDrawFunc(nCellX * 48 - nViewX, nCellY * 32 - nViewY, TileKey(nCellX, nCellY));
            }
        }
    }
}

bool Mir2xMap::Overlap(int nX, int nY, int nCX, int nCY, int nR)
{
    if(ValidP(nX, nY) && nR > 0){
    }

    int nMinX = nX - nR;
    int nMinY = nY - nR;
    int nMaxX = nX + nR;
    int nMaxY = nY + nR;

    int nCellStartX = nMinX / 48;
    int nCellStartY = nMinY / 32;
    int nCellStopX  = nMaxX / 32;
    int nCellStopY  = nMaxY / 32;

    for(int nCellX = nCellStartX; nCellX <= nCellStopX; ++nCellX){
        for(int nCellY = nCellStartY; nCellY <= nCellStopY; ++nCellY){
            int nBdX[4] = {(nCellX + 0) * 48, (nCellX + 1) * 48, (nCellX + 1) * 48, (nCellX + 0) * 48};
            int nBdY[4] = {(nCellY + 0) * 32, (nCellY + 0) * 32, (nCellY + 1) * 32, (nCellY + 1) * 32};
            int nMidX = nX * 48 + 24;
            int nMidY = nY * 32 + 16;

            bool bMidIn = PointInCircle(nMidX, nMidY, nCX, nCY, nR);
            for(int nIndex = 0; nIndex < 4; ++nIndex){
                if(!CanWalk(nCellX, nCellY, nIndex)
                        && (bMidIn || PointInCircle(nBdX[nIndex], nBdY[nIndex], nCX, nCY, nR)
                            || PointInCircle(nBdX[(nIndex + 1) % 4], nBdY[(nIndex + 1) % 4], nCX, nCY, nR))){
                    return true;
                }
            }
        }
    }
    return false;
}

bool Mir2xMap::LoadHead(uint8_t * &pData)
{
    m_W = *((uint16_t *)pData); pData += 2;
    m_H = *((uint16_t *)pData); pData += 2;

    m_TileDesc = new TILEDESC[m_W * m_H / 4];
    m_CellDesc = new CELLDESC[m_W * m_H    ];

    if(pData[0] != 0){
        return false;
    }else{
        pData++;
        return true;
    }
}

bool Mir2xMap::LoadWalk(uint8_t * &pData)
{
    long nBitOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseWalk(nBlkX * 8, nBlkY * 8, 8, pData + 4, nBitOff);
        }
    }

    pData += (4 + *((uint32_t *)pData));
    if(pData[0] != 0){
        return false;
    }else{
        pData++;
        return true;
    }
}

bool Mir2xMap::LoadLight(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4 + nMarkLen));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseLight(nBlkX * 8, nBlkY * 8, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0){
        return false;
    }else{
        pData++;
        return true;
    }
}


void Mir2xMap::ParseObj(int nX, int nY, int nSize, int nObjIndex,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    //
    // 1: current obj is for wall layer
    // 0: for ground layer
    //
    if(ValidC(nX, nY)){
        if(PickOneBit(pMark, nMarkOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                // need to read one more bit for ground / wall layer decision
                SetOneObj(nX, nY, nObjIndex, pMark, nMarkOff, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pMark, nMarkOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseObj(nX,             nY,             nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                    ParseObj(nX + nSize / 2, nY,             nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                    ParseObj(nX,             nY + nSize / 2, nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                    ParseObj(nX + nSize / 2, nY + nSize / 2, nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    // for object, store data sequentially when there is full-filled grid with subgrid
                    // because for object it's hard to repeat in close area
                    SetObj(nX, nY, nSize, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetObj(nX, nY, nSize, nObjIndex, nullptr, nMarkOff, nullptr, nDataOff);
        }
    }
}

bool Mir2xMap::LoadObj(uint8_t * &pData, int nObjIndex)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4 + nMarkLen));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseObj(nBlkX * 8, nBlkY * 8, 8, nObjIndex % 2,
                    pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0){
        return false;
    }else{
        pData++;
        return true;
    }

}

void Mir2xMap::ExtendBuf(size_t nSize)
{
    if(nSize > m_BufMaxSize){
        delete[] m_Buf;
        m_Buf        = new uint8_t[nSize];
        m_BufSize    = nSize;
        m_BufMaxSize = nSize;
    }else{
        m_BufSize = nSize;
    }
}

void Mir2xMap::ParseTile(int nX, int nY, int nSize,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    //
    // 1: lights in full-filled grid are of different attributes
    // 0: no
    if(ValidC(nX, nY)){
        if(PickOneBit(pMark, nMarkOff++)){
            // there is information in current grid
            if(nSize == 2){
                // last level of grid, and there is data, so fill it directly
                SetTile(nX, nY, 2, pMark, nMarkOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pMark, nMarkOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseTile(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseTile(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseTile(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseTile(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    // ask one more bit for light, here we use recursively defined data stream
                    // since most likely lights/tiles in full-filled grid are of the same attributes
                    if(PickOneBit(pMark, nMarkOff++)){
                        // full-filled grid with lights of different attributes
                        // this rarely happens
                        ParseTile(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseTile(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseTile(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                        ParseTile(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    }else{
                        // full-filled grid with lights of the same attributes
                        SetTile(nX, nY, nSize, pData, nDataOff);
                    }
                }
            }
        }else{
            // no data here, always unset the desc field for the whole grid
            SetTile(nX, nY, nSize, nullptr, nDataOff);
        }
    }
}

bool Mir2xMap::LoadTile(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4 + nMarkLen));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseTile(nBlkX * 8, nBlkY * 8, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }
    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0){
        return false;
    }else{
        pData++;
        return true;
    }
}

void Mir2xMap::SetTile(int nX, int nY, int nSize, const uint8_t *pData, long &nDataOff)
{
    // full-filled current grid defined by the same attributes
    TILEDESC stTileDesc = {0, 0, 0};

    if(pData){
        stTileDesc.Desc       = 0X01;
        stTileDesc.FileIndex  = pData[nDataOff++];
        stTileDesc.ImageIndex = *((uint16_t *)(pData + nDataOff));
        nDataOff += 2;
    }

    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            if(!(nTX % 2 || nTY %2)){
                TileDesc(nTX, nTY) = stTileDesc;
            }
        }
    }
}
