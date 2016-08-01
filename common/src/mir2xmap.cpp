#include <memory.h>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>
#include "triangle.hpp"
#include "mir2xmap.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"

Mir2xMap::Mir2xMap()
    : m_TileDesc(nullptr)
    , m_CellDesc(nullptr)
    , m_Buf(nullptr)
    , m_BufSize(0)
    , m_BufMaxSize(0)
    , m_W(0)
    , m_H(0)
    , m_Valid(false)
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
    (void)(1 + fread(m_Buf, nSize, 1, pFile));
    fclose(pFile);

    uint8_t *pCurDat = m_Buf;
    m_Valid = LoadHead(pCurDat) && LoadGround(pCurDat) 
        && LoadLight(pCurDat) && LoadTile(pCurDat) && LoadObj(pCurDat, 0) && LoadObj(pCurDat, 1);

    return m_Valid;
}

void Mir2xMap::SetOneGround(int nX, int nY, int nIndex, bool bHasInfo, uint8_t nGroundInfo)
{
    uint16_t nMask = (uint16_t)(0X0001 << nIndex);
    if(bHasInfo){
        CellDesc(nX, nY).Desc |= nMask;
        CellDesc(nX, nY).Ground[nIndex] = nGroundInfo;
    }else{
        uint16_t nRMask = (uint16_t)(0XFFFF ^ nMask);
        CellDesc(nX, nY).Desc &= nRMask;
        CellDesc(nX, nY).Ground[nIndex] = 0;
    }
}

void Mir2xMap::SetGround(int nX, int nY, int nSize, bool bHasInfo, uint8_t nGroundInfo)
{
    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            if(!ValidC(nTX, nTY)){ continue; }
            SetOneGround(nTX, nTY, 0, bHasInfo, nGroundInfo);
            SetOneGround(nTX, nTY, 1, bHasInfo, nGroundInfo);
            SetOneGround(nTX, nTY, 2, bHasInfo, nGroundInfo);
            SetOneGround(nTX, nTY, 3, bHasInfo, nGroundInfo);
        }
    }
}

// TODO: need to deal with walkable grid attributes:
//       cave floor
//       indoor floor
//       grass
//       etc.
//
//       differ ocean/lake/pond so sound can be different for even one tile
//
//       this is for different sound effect and more
void Mir2xMap::ParseGround(int nX, int nY, int nSize,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    //
    // 1: ground info in full-filled grid are of different attributes
    // 0: no
    if(!ValidC(nX, nY)){ return; }

    if(PickOneBit(pMark, nMarkOff++)){
        // there is information in current grid
        if(nSize == 1){
            // last level of grid consists of four smallest subgrids divided by X-cross
            if(PickOneBit(pMark, nMarkOff++)){
                // it's a/0 combined or a/b combined at last level
                // should parse one by one
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(PickOneBit(pMark, nMarkOff++)){
                        SetOneGround(nX, nY, nIndex, true, pData[nDataOff++]);
                    }else{
                        SetOneGround(nX, nY, nIndex, false, 0);
                    }
                }
            }else{
                // it's not combined, and there is info, so it only can be all full-filled
                SetGround(nX, nY, 1, true, pData[nDataOff++]);
            }
        }else{
            // not the last level of grid, and there is information in current gird
            if(PickOneBit(pMark, nMarkOff++)){
                // there is data, and it's combined, need further parsing
                // maybe filled/empty, or fullfilled of different attributes
                ParseGround(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseGround(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseGround(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseGround(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
            }else{
                // there is information, but it's not combined, can only be full-filled
                SetGround(nX, nY, nSize, true, pData[nDataOff++]);
            }
        }
    }else{
        // no data here, always unset the desc field for the whole grid
        SetGround(nX, nY, nSize, false, 0);
    }
}

// light is the simplest
void Mir2xMap::ParseLight(int nX, int nY, int nSize,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    //
    // 1: light in full-filled grid are of different attributes
    // 0: no
    if(!ValidC(nX, nY)){ return; }

    if(PickOneBit(pMark, nMarkOff++)){
        // there is information in current grid
        if(nSize == 1){
            // there is info and it's last level
            // end of recursion
            SetLight(nX, nY, 1, pData, nDataOff);
        }else{
            // not the last level of grid, and there is information in current gird
            if(PickOneBit(pMark, nMarkOff++)){
                // there is info, and it's combined, need further parsing
                // maybe a/0 or a/b combination
                ParseLight(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseLight(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseLight(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
            }else{
                // there is info, but not a/0 or a/b combined
                // can only be a/a full-filled
                SetLight(nX, nY, nSize, pData, nDataOff);
            }
        }
    }else{
        // no data here, always unset the desc field for the whole grid
        SetLight(nX, nY, nSize, nullptr, nDataOff);
    }
}

void Mir2xMap::SetObj(int nX, int nY, int nObjIndex, int nSize,
        const uint8_t *pMark, long &nMarkOff, const uint8_t *pData, long &nDataOff)
{
    // we define two objects are *the same* in:
    //  1. animation desc
    //  2. file index
    //  3. image index
    //  4. layer
    //  5. alpha obj
    //
    OBJDESC stObjDesc  = {0, 0, 0};
    bool    bIsObj     = false;
    bool    bGroundObj = false;
    bool    bAlphaObj  = false;

    if(pMark && pData){
        stObjDesc.Desc       = pData[nDataOff++];
        stObjDesc.FileIndex  = pData[nDataOff++];
        stObjDesc.ImageIndex = *((uint16_t *)(pData + nDataOff));

        nDataOff += 2;
        bIsObj = true; // mark for validation

        bGroundObj = PickOneBit(pMark, nMarkOff++);
        bAlphaObj  = PickOneBit(pMark, nMarkOff++);
    }

    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            if(!ValidC(nTX, nTY)){ continue; }

            CellDesc(nTX, nTY).Obj[nObjIndex] = stObjDesc;
            if(bIsObj){
                // mark it as valid
                CellDesc(nTX, nTY).Desc |= ((nObjIndex == 0) ? 0X0100 : 0X1000);

                if(bGroundObj){
                    CellDesc(nTX, nTY).Desc |= ((nObjIndex == 0) ? 0X0200 : 0X2000);
                }else{
                    CellDesc(nTX, nTY).Desc &= ((nObjIndex == 0) ? 0XFDFF : 0XDFFF);
                }

                if(bAlphaObj){
                    CellDesc(nTX, nTY).Desc |= ((nObjIndex == 0) ? 0X0800 : 0X8000);
                }else{
                    CellDesc(nTX, nTY).Desc &= ((nObjIndex == 0) ? 0XF7FF : 0X7FFF);
                }
            }else{
                CellDesc(nTX, nTY).Desc &= ((nObjIndex == 0) ? 0XFEFF : 0XEFFF);
            }
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
            if(!ValidC(nTX, nTY)){ continue; }

            if(pData){
                CellDesc(nTX, nTY).Desc |= 0X0080;
            }else{
                CellDesc(nTX, nTY).Desc &= 0XFF7F;
            }
            CellDesc(nTX, nTY).Light = nLightAttr;
        }
    }
}

void Mir2xMap::Draw(int nViewX, int nViewY, int nViewW, int nViewH, // view region
        int nMaxObjW, int nMaxObjH,                                 // operation addition margin
        const std::function<void(int, int, uint32_t)> &fnDrawTile,  //
        const std::function<void(int, int, uint32_t)> &fnDrawObj,   //
        const std::function<void(int, int)> &fnDrawActor,           //
        const std::function<void(int, int)> &fnDrawExt)             //
{
    // to make it safe
    int nStartCellX = (nViewX - 2 * SYS_MAPGRIDXP - nMaxObjW         ) / SYS_MAPGRIDXP;
    int nStartCellY = (nViewY - 2 * SYS_MAPGRIDYP - nMaxObjH         ) / SYS_MAPGRIDYP;
    int nStopCellX  = (nViewX + 2 * SYS_MAPGRIDXP + nMaxObjW + nViewW) / SYS_MAPGRIDXP;
    int nStopCellY  = (nViewY + 2 * SYS_MAPGRIDYP + nMaxObjH + nViewH) / SYS_MAPGRIDYP;

    // 1. draw tile, this should be done seperately
    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            // 1. boundary check
            if(!ValidC(nCellX, nCellY)){ continue; }

            // 2. for tile
            if(!(nCellY % 2) && !(nCellX % 2) && TileValid(nCellX, nCellY)){
                fnDrawTile(nCellX, nCellY, Tile(nCellX, nCellY));
            }
        }
    }

    // 2. draw everything on ground
    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            // 1. validate the cell, draw overgournd objects here
            if(ValidC(nCellX, nCellY)){
                // 1-1. draw ground cell object
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(GroundObjectValid(nCellX, nCellY, nIndex)){
                        fnDrawObj(nCellX, nCellY, Object(nCellX, nCellY, nIndex));
                    }
                }

                // 1-2. draw actor
                fnDrawActor(nCellX, nCellY);

                // 1-3. draw over ground cell object
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(!GroundObjectValid(nCellX, nCellY, nIndex)){
                        fnDrawObj(nCellX, nCellY, Object(nCellX, nCellY, nIndex));
                    }
                }
            }

            // 2. draw ext, even the cell is not valid we need to draw it
            fnDrawExt(nCellX, nCellY);

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

bool Mir2xMap::LoadGround(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseGround(nBlkX * 8, nBlkY * 8, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0
            || (long)nMarkLen != (nMarkOff + 7) / 8
            || (long)nDataLen != nDataOff){
        return false;
    }else{
        pData++;
        return true;
    }
}

bool Mir2xMap::LoadLight(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseLight(nBlkX * 8, nBlkY * 8, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0
            || (long)nMarkLen != (nMarkOff + 7) / 8
            || (long)nDataLen != nDataOff){
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
    if(!ValidC(nX, nY)){ return; }

    if(PickOneBit(pMark, nMarkOff++)){
        // there is information in current grid
        if(nSize == 1){
            // last level of grid, and there is data, so fill it directly
            // need to read one more bit for ground / wall layer decision
            SetObj(nX, nY, nObjIndex, 1, pMark, nMarkOff, pData, nDataOff);
        }else{
            // not the last level of grid, and there is information in current gird
            if(PickOneBit(pMark, nMarkOff++)){
                // there is data in current grid and it's combined, further parse it
                ParseObj(nX,             nY,             nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                ParseObj(nX + nSize / 2, nY,             nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                ParseObj(nX,             nY + nSize / 2, nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
                ParseObj(nX + nSize / 2, nY + nSize / 2, nSize / 2, nObjIndex, pMark, nMarkOff, pData, nDataOff);
            }else{
                SetObj(nX, nY, nObjIndex, nSize, pMark, nMarkOff, pData, nDataOff);
            }
        }
    }else{
        // no data here, always unset the desc field for the whole grid
        SetObj(nX, nY, nObjIndex, nSize, nullptr, nMarkOff, nullptr, nDataOff);
    }
}

bool Mir2xMap::LoadObj(uint8_t * &pData, int nObjIndex)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseObj(nBlkX * 8, nBlkY * 8, 8, nObjIndex % 2,
                    pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0 ||
            (long)nMarkLen != (nMarkOff + 7) / 8 ||
            (long)nDataLen != nDataOff){
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
    if(!ValidC(nX, nY)){ return; }

    if(PickOneBit(pMark, nMarkOff++)){
        // there is information in current grid
        if(nSize == 2){
            // last level of grid, and there is data, so fill it directly
            SetTile(nX, nY, 2, pData, nDataOff);
        }else{
            // not the last level of grid, and there is information in current gird
            if(PickOneBit(pMark, nMarkOff++)){
                // there is data in current grid and it's combined, further parse it
                ParseTile(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseTile(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseTile(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                ParseTile(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
            }else{
                // full-filled grid with lights of the same attributes
                SetTile(nX, nY, nSize, pData, nDataOff);
            }
        }
    }else{
        // no data here, always unset the desc field for the whole grid
        SetTile(nX, nY, nSize, nullptr, nDataOff);
    }
}

bool Mir2xMap::LoadTile(uint8_t * &pData)
{
    uint32_t nMarkLen = *((uint32_t *)(pData + 0));
    uint32_t nDataLen = *((uint32_t *)(pData + 4));

    long nMarkOff = 0;
    long nDataOff = 0;

    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseTile(nBlkX * 8, nBlkY * 8, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff);
        }
    }
    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] != 0
            || (long)nMarkLen != (nMarkOff + 7) / 8
            || (long)nDataLen != nDataOff){
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
        // stTileDesc.Desc       = 0X01;
        stTileDesc.Desc       = (pData[nDataOff++] | 0X80);
        stTileDesc.FileIndex  = (pData[nDataOff++]       );
        stTileDesc.ImageIndex = *((uint16_t *)(pData + nDataOff));
        nDataOff += 2;

        // stTileDesc.Desc       = pData[nDataOff++];
        // stTileDesc.FileIndex  = pData[nDataOff++];
        // stTileDesc.ImageIndex = *((uint16_t *)(pData + nDataOff));
        // nDataOff += 2;
    }

    for(int nTY = nY; nTY < nY + nSize; ++nTY){
        for(int nTX = nX; nTX < nX + nSize; ++nTX){
            if(!ValidC(nTX, nTY)){ continue; }

            if(!(nTX % 2 || nTY % 2)){
                TileDesc(nTX, nTY) = stTileDesc;
            }
        }
    }
}

bool Mir2xMap::CanWalkP(int nPX, int nPY, int nPR)
{
    // 1. we make strict parameter check here
    assert(nRR >= 0);

    // 2. valid point?
    if(!ValidP(nPX, nPY)){ return false; }

    // 3. ok it's a real ``cover"
    if(nPR){
        int nPX0 = nPX - nPR;
        int nPX1 = nPX + nPR;
        int nPY0 = nPY - nPR;
        int nPY1 = nPY + nPR;

        // 3. the cover should fully in the map
        if(!RectangleInside(0, 0, W() * SYS_MAPGRIDXP, H() * SYS_MAPGRIDYP, nPX0, nPY0, nPX1, nPY1)){ return false; }

        // ok now it's in the map, we check each covered grid
        for(int nGY = nPY0 / SYS_MAPGRIDYP; nGY <= nPY1 / SYS_MAPGRIDYP; ++nGY){
            for(int nGX = nPX0 / SYS_MAPGRIDXP; nGX <= nPX1 / SYS_MAPGRIDXP; ++nGX){
                // 1. couldn't happen
                if(!ValidC(nGX, nGY)){ continue; }

                int nGPX0 = nGX * SYS_MAPGRIDXP;
                int nGPY0 = nGY * SYS_MAPGRIDYP;

                // 2. check if circle overlaps with this grid
                if(!CircleRectangleOverlap(nPX, nPY, nPR, nGPX0, nGPY0, SYS_MAPGRIDXP, SYS_MAPGRIDYP)){ continue; }

                int nGPX1 = nGX * SYS_MAPGRIDXP + SYS_MAPGRIDXP;
                int nGPY1 = nGY * SYS_MAPGRIDYP + SYS_MAPGRIDYP;

                int nGPMX = (nGPX0 + nGPX1) / 2;
                int nGPMY = (nGPY0 + nGPY1) / 2;

                if(CircleTriangleOverlap(nPX, nPY, nPR, nGPMX, nGPMY, nGPX0, nGPY0, nGPX1, nGPY0)){ if(!CanWalk(nGX, nGY, 0)){ return false; } }
                if(CircleTriangleOverlap(nPX, nPY, nPR, nGPMX, nGPMY, nGPX1, nGPY0, nGPX1, nGPY1)){ if(!CanWalk(nGX, nGY, 1)){ return false; } }
                if(CircleTriangleOverlap(nPX, nPY, nPR, nGPMX, nGPMY, nGPX1, nGPY1, nGPX0, nGPY1)){ if(!CanWalk(nGX, nGY, 2)){ return false; } }
                if(CircleTriangleOverlap(nPX, nPY, nPR, nGPMX, nGPMY, nGPX0, nGPY1, nGPX0, nGPY0)){ if(!CanWalk(nGX, nGY, 3)){ return false; } }
            }
        }

        // all overlapped grids pass the check
        return true;
    }

    // just a point
    int nDX = nPX % SYS_MAPGRIDXP;
    int nDY = nPY % SYS_MAPGRIDYP;

    int nBit0 = ((SYS_MAPGRIDXP * nDY <= SYS_MAPGRIDYP * nDX) ? 1 : 0); // or just take it as int...
    int nBit1 = ((SYS_MAPGRIDXP * nDY <= SYS_MAPGRIDYP * (SYS_MAPGRIDXP - nDX)) ? 1 : 0);

    static int knReginIndex[] = {
        2, // 0
        1, // 1
        3, // 2
        0  // 3
    };

    return CanWalk(nPX / SYS_MAPGRIDXP, nPY / SYS_MAPGRIDYP, knReginIndex[nBit1 * 2 + nBit0]);
}
