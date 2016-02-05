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

void Mir2xMap::ParseWalk(int nX, int nY, int nSize, uint8_t *pMark, long &nMarkOff)
{
    // 1: there is data in current grid
    // 0: no
    //
    // 1: current grid is combined, means it's filled partically
    // 0: no
    if(nX < m_W && nY < m_H){
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid consists of four smallest subgrids divided by X-cross
                if(PickOneBit(pData, nBitOff++)){
                    // it's combined at last level
                    SetOneWalk(nX, nY, 0, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nX, nY, 1, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nX, nY, 2, PickOneBit(pData, nBitOff++));
                    SetOneWalk(nX, nY, 3, PickOneBit(pData, nBitOff++));;
                }else{
                    // it's not combined, and there is info, so it only can be all full-filled
                    SetWalk(nX, nY, 1, true);
                }
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
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
        uint8_t *pMark, long &nMarkOff, uint8_t *pData, long &nDataOff)
{
    if(pMark && pData){
        CellDesc(nX, nY).Obj[nObjIndex].Desc       = pData[nDataOff++];
        CellDesc(nX, nY).Obj[nObjIndex].FileIndex  = pData[nDataOff++];
        CellDesc(nX, nY).Obj[nObjIndex].ImageIndex = *((uint16_t)(pData + nDataOff));;
        pData += 2;

        SetObjLayer(nX, nY, nObjIndex, true, PickOneBit(pMark, nMarkOff++));
    }else{
        SetObjLayer(nX, nY, nObjIndex, false, true);
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
        uint8_t *pMark, long &nMarkOff, uint8_t *pData, long &nDataOff)
{
    // full-fill current grid defined by parameters
    // obj has ground / wall layer, so need further parse mark data
    for(int nY = nY; nY < nY + nSize; ++nY){
        for(int nX = nX; nX < nX + nSize; ++nX){
            SetOneObj(nX, nY, nObjIndex, pMark, nMarkOff, pData, nDataOff);
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
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                SetLight(nX, nY, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseLight(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    // ask one more bit for light, here we use recursively defined data stream
                    // since most likely lights in full-filled grid are of the same attributes
                    if(PickOneBit(pData, nBitOff++)){
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

    for(int nY = nY; nY < nY + nSize; ++nY){
        for(int nX = nX; nX < nX + nSize; ++nX){
            if(pData){
                CellDesc(nX, nY).Desc |= 0X8000;
            }else{
                CellDesc(nX, nY).Desc &= 0X7FFF;
            }
            CellDesc(nX, nY).Light = nLightAttr;
        }
    }
}

void Mir2Map::DrawGround(int nViewX, int nViewY, int nViewW, int nViewH,
        std::function<void(int, int, uint32_t, int, int)> fnDrawFunc)
{
    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            if(!(nCellY % 2) && !(nCellX % 2) && TileValid(nCellX, nCellY)){
                fnDrawFunc(nCellX * 48, nCellY * 32, TileKey(nCellX, nCellY), 96, 64);
            }
        }
    }
}

void Mir2Map::DrawGroundObj(int nViewX, int nViewY, int nViewW, int nViewH, int nMaxObjH,
        std::function<void(int, int, uint32_t, int, int)> fnDrawFunc)
{
    int nStartCellX = nViewX / 96;
    int nStartCellY = nViewY / 64;

    int nStopCellX = (nViewX + nViewW) / 96;
    int nStopCellY = (nViewY + nViewH) / 64;

    for(int nCellY = nStartCellY; nCellY <= nStopCellY; ++nCellY){
        for(int nCellX = nStartCellX; nCellX <= nStopCellX; ++nCellX){
            if(CellObjOverGround(nCellX, nCellY, 0)){
                uint32_t nKey = nCellX, nCellY, 0CellObjKey(nCellX, nCellY, 0);
                int nStartX = fnTexStartX(nkey);
                int nStartY = fnTexStartY(nKey);

                fnDrawFunc()
            }
            if(ValidGroundObj(nCellX, nCellY, 1)){
                CellObjKey(nCellX, nCellY, )
                    int nStartX = fnTexStartX()
                    fnDrawFunc(nCellX * 48, nCellY * 32, TileKey(nCellX, nCellY), 96, 64);
            }
        }
    }
}

void Mir2xMap::Draw(
        int nView, int nViewY, int nViewW, int nViewH,      //
        int nMaxObjH,                                       // 
        )
{
    DrawGround(nView, nViewY, nViewW, nViewH, fnDrawFunc);

    DrawGroundObj(nView, nViewY, nViewW, nViewH, fnDrawFunc);




    void Mir2Map::DrawGroundObj(int nViewX, int nViewY, int nViewW, int nViewH,
            std::function<void(int, int, uint32_t, int, int)> fnDrawFunc)
}



void Mir2xMap::DrawBaseTile(
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

            for(int nIndex = 0; nIndex < 4; ++nIndex){
                if(!CanWalk(nCellX, nCellY, nIndex) && (PointInCircle(nBdX[nIndex], nBdY[nIndex], nBdX[(nIndex + 1) % 4], nBdy[(nIndex + 1) % 4], nCX, nCY, nR)){
                        return false;
                        }
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

                        void Mir2xMap::DrawObject(
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

void Mir2xMap::DrawGroundObject(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY)
{
    auto fnCheckFunc = [](int nW, int nH){
        return nW == 48 && nH == 32;
    };
    auto fnExtDrawFunc = [](int, int){};
    DrawObject(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnExtDrawFunc);
}

void Mir2xMap::DrawOverGroundObject(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY, 
        std::function<void(int, int)> fnExtDrawFunc)
{
    auto fnCheckFunc = [](int nW, int nH){
        return nW != 48 || nH != 32;
    };
    DrawObject(nStartCellX, nStartCellY, nStopCellX, nStopCellY, fnCheckFunc, fnExtDrawFunc);
}

uint32_t Mir2xMap::GetDoorImageIndex(int nX, int nY)
{
    uint32_t  nDoorIndex = 0;
    auto     &stCellDesc = m_CellDesc[nX + nY * m_W];
    if((stCellDesc.dwDesc & 0XFF000000) > 0){
        nDoorIndex += ((stCellDesc.dwDesc & 0X007F0000) >> 16);
        // printf("be careful: GetDoorImageIndex() returns non-zero value !!!\n");
    }
    return nDoorIndex;
}

void Mir2xMap::SetViewPoint(int nX, int nY)
{
    m_ViewX = nX;
    m_ViewY = nY;
}

int Mir2xMap::ViewX()
{
    return m_ViewX;
}

int Mir2xMap::ViewY()
{
    return m_ViewY;
}

bool Mir2xMap::ValidPosition()
{

}

void Mir2xMap::LoadHead(uint8_t * &pData)
{
    m_W = *((uint16_t *)pData); pData += 2;
    m_H = *((uint16_t *)pData); pData += 2;

    m_TileDesc = new TILEDESC[m_W * m_H / 4];
    m_CellDesc = new CELLDESC[m_W * m_H    ];
}

void Mir2xMap::LoadWalk(uint8_t * &pData)
{
    long nBitOff = 0;
    for(int nBlkY = 0; nBlkY < (m_H + 7) / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < (m_W + 7) / 8; ++nBlkX){
            ParseWalk(nBlkX * 8, nBlkY * 8, pData + 4, nBitOff);
        }
    }
    pData += (4 + *((uint32_t *)pData));
}

void Mir2xMap::LoadLight(uint8_t * &pData)
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

    if(pData[8 + nMarkLen + nDataLen] != 0){
        return false;
    }else{
        pData += (8 + nMarkLen + nDataLen + 1);
        return true;
    }
}


void Mir2xMap::ParseObj(int nX, int nY, int nSize, int nObjIndex
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
        if(PickOneBit(pData, nMarkOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                // need to read one more bit for ground / wall layer decision
                SetOneObj(nX, nY, nObjIndex, pMark, nMarkOff, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
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

    if(pData[8 + nMarkLen + nDataLen] != 0){
        return false;
    }else{
        pData += (8 + nMarkLen + nDataLen + 1);
        return true;
    }
}

void Mir2Map::ExtendBuf(long nSize)
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
        if(PickOneBit(pData, nBitOff++)){
            // there is information in current grid
            if(nSize == 1){
                // last level of grid, and there is data, so fill it directly
                SetLight(nX, nY, pData, nDataOff);
            }else{
                // not the last level of grid, and there is information in current gird
                if(PickOneBit(pData, nBitOff++)){
                    // there is data in current grid and it's combined, further parse it
                    ParseLight(nX,             nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY,             nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX,             nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                    ParseLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, pMark, nMarkOff, pData, nDataOff);
                }else{
                    // there is data and not combined, so full-filled the whole grid
                    // ask one more bit for light, here we use recursively defined data stream
                    // since most likely lights in full-filled grid are of the same attributes
                    if(PickOneBit(pData, nBitOff++)){
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

void Mir2xMap::LoadTile(uint8_t * &pData)
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

    if(pData[8 + nMarkLen + nDataLen] != 0){
        return false;
    }else{
        pData += (8 + nMarkLen + nDataLen + 1);
        return true;
    }
}
