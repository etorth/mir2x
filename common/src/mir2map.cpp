#include "mir2map.hpp"
#include "wilimagepackage.hpp"
#include <memory.h>
#include "assert.h"
#include <cstring>
#include <functional>
#include <cstdint>
#include <algorithm>
#include <vector>
#include "savepng.hpp"
#include "filesys.hpp"

Mir2Map::Mir2Map()
    : m_Valid(false)
    , m_DoorCount(0)
    , m_LightCount(0)
    , m_AlphaBlend(0)
    , m_pstTileInfo(nullptr)
    , m_pstCellInfo(nullptr)
    , m_BaseTileInfo(nullptr)
    , m_GroundInfo(nullptr)
    , m_CellDesc(nullptr)
    , m_SelectedGrid()
{
    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));
    std::memset(m_bAniTileFrame, 0, sizeof(uint8_t) * 8 * 16);
    std::memset(m_dwAniSaveTime, 0, sizeof(uint32_t) * 8);
}

Mir2Map::~Mir2Map()
{
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;
}

void Mir2Map::LoadMapImage(WilImagePackage *pWilImagePackage)
{
    m_pxTileImage = pWilImagePackage;
}

bool Mir2Map::LoadMap(const char *szMapFileName)
{
    m_Valid = false;

    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;

    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));

    auto hFile = fopen(szMapFileName, "rb");
    if(hFile == nullptr){
        return false;
    }

    if(fread(&m_stMapFileHeader, sizeof(MAPFILEHEADER), 1, hFile) != 1){
        // printf("pos  = %d\n", ftell(hFile));
        fclose(hFile);
        return false;
    }

    size_t nMapLoadSize = m_stMapFileHeader.shWidth * m_stMapFileHeader.shHeight;

    m_pstTileInfo = new TILEINFO[nMapLoadSize / 4];
    if(fread(m_pstTileInfo, sizeof(TILEINFO), nMapLoadSize / 4, hFile) != nMapLoadSize / 4){
        delete m_pstTileInfo; m_pstTileInfo = nullptr;
        fclose(hFile);
        return false;
    }

    m_pstCellInfo = new CELLINFO[nMapLoadSize];
    if(fread(m_pstCellInfo, sizeof(CELLINFO), nMapLoadSize, hFile) != nMapLoadSize){
        delete m_pstCellInfo; m_pstCellInfo = nullptr;
        fclose(hFile);
        return false;
    }

    fclose(hFile);

    m_Valid = true;

    SetMapInfo();

    m_SelectedGrid = std::vector<std::vector<std::array<bool, 4>>>(
            Width(), std::vector<std::array<bool, 4>>(
                Height(), {false, false, false, false}));

    return m_Valid;
}

void Mir2Map::ExtractOneBaseTile(
            std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc,
            int nXCnt, int nYCnt)
{
    if(!Valid() || !ValidC(nXCnt, nYCnt) || (nXCnt % 2) || (nYCnt % 2)){ return; }

    if(m_Mir2xMap.Valid()){
        // decode with new map structure
        if(!m_Mir2xMap.TileValid(nXCnt, nYCnt)){ return; }

        // no alpha blend for tile 
        uint32_t nKey   = m_Mir2xMap.TileKey(nXCnt, nYCnt);
        int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
        int nImageIndex = ((nKey & 0X0000FFFF));

        // actually we don't need to check here
        // just put it here
        if(nFileIndex == 255 || nImageIndex == 65535){
            return;
        }

        if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                m_pxTileImage[nFileIndex].CurrentImageValid()){
            int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
            int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;

            uint32_t *pBuff = nullptr;
            if(nW * nH > 0){
                pBuff = new uint32_t[nW * nH];
            }else{
                return;
            }
            // no alpha blending for tiles
            m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
            fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
            delete pBuff;
        }
    }else{
        // decode with old mir2 map
        int nFileIndex  = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].bFileIndex;
        int nImageIndex = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].wTileIndex;

        if(nFileIndex != 255 && nImageIndex != 65535){
            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                    m_pxTileImage[nFileIndex].CurrentImageValid()){
                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                uint32_t *pBuff = nullptr;
                if(nW * nH > 0){
                    pBuff = new uint32_t[nW * nH];
                }else{
                    return;
                }
                // no alpha blending
                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                delete pBuff;
            }
        }
    }
}

void Mir2Map::ExtractBaseTile(std::function<bool(uint32_t, uint32_t)> fnCheckExistFunc,
        std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc)
{
    if(!Valid()){ return; }

    int nFileIndex  = 0;
    int nImageIndex = 0;

    uint32_t *pBuff    = nullptr;
    int       nBuffLen = 0;

    if(m_Mir2xMap.Valid()){
        for(int nXCnt = 0; nXCnt < m_Mir2xMap.W(); nXCnt++){
            for(int nYCnt = 0; nYCnt < m_Mir2xMap.H(); ++nYCnt){
                if(!(nXCnt % 2) && !(nYCnt % 2)){
                    if(!m_Mir2xMap.TileValid(nXCnt, nYCnt)){ continue; }

                    // no alpha blend for tile 
                    uint32_t nKey   = m_Mir2xMap.TileKey(nXCnt, nYCnt);
                    int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                    int nImageIndex = ((nKey & 0X0000FFFF));

                    // actually we don't need to check here
                    // just put it here
                    if(nFileIndex == 255 || nImageIndex == 65535){
                        continue;
                    }

                    if(fnCheckExistFunc(nFileIndex, nImageIndex)){
                        continue;
                    }

                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                            m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;

                        if(nW * nH > 0){
                            if(nW * nH > nBuffLen){
                                delete pBuff;
                                pBuff = new uint32_t[nW * nH];
                                nBuffLen = nW * nH;
                            }

                            // no alpha blending for tiles
                            m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                            fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
    }else{
        for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; nXCnt++){
            for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){
                if(!(nXCnt % 2) && !(nYCnt % 2)){
                    int nFileIndex = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].bFileIndex;
                    int nImageIndex = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].wTileIndex;

                    if(nFileIndex != 255 && nImageIndex != 65535) {
                        if(fnCheckExistFunc(nFileIndex, nImageIndex)){
                            continue;
                        }
                        if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                                m_pxTileImage[nFileIndex].CurrentImageValid()){

                            int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                            int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;

                            if(nW * nH > 0){
                                if(nW * nH > nBuffLen){
                                    delete pBuff;
                                    pBuff = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                            }
                        }
                    }
                }
            }
        }
    }
    delete pBuff;
}

int Mir2Map::Width()
{
    return (int)(m_stMapFileHeader.shWidth);
}

int Mir2Map::Height()
{
    return (int)(m_stMapFileHeader.shHeight);
}

const CELLINFO &Mir2Map::CellInfo(int nX, int nY)
{
    return m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight];
}

const TILEINFO &Mir2Map::BaseTileInfo(int nX, int nY)
{
    return m_pstTileInfo[(nX / 2) * m_stMapFileHeader.shHeight / 2 + nY / 2];
}

uint32_t Mir2Map::GetDoorImageIndex(int nX, int nY)
{
    uint32_t nDoorIndex = 0;
    if(Valid()){
        if(m_Mir2xMap.Valid()){
            return 0;
        }else{
            // seems bDoorOffset & 0X80 shows open or close
            //       bDoorIndex  & 0X80 shows whether there is a door
            if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorOffset & 0X80) > 0){
                if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorIndex & 0X7F) > 0){
                    nDoorIndex += (m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorOffset & 0X7F);
                }
            }
        }
    }
    return nDoorIndex;
}

void Mir2Map::DrawBaseTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<void(uint32_t, uint32_t, int, int)> fnDrawTileFunc)
{
    if(!Valid()){ return; }

    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, Width() - 1);
    nStopCellY  = (std::min)(nStopCellY, Height() - 1);

    for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
        for(int nX = nStartCellX; nX <= nStopCellX; ++nX){
            if(nX % 2 || nY % 2){
                continue;
            }

            if(m_Mir2xMap.Valid()){
                if(!m_Mir2xMap.TileValid(nX, nY)){ continue; }
                uint32_t nKey = m_Mir2xMap.TileKey(nX, nY);
                int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                int nImageIndex = ((nKey & 0X0000FFFF));

                if(nFileIndex != 255 && nImageIndex != 65535){
                    fnDrawTileFunc(nFileIndex, nImageIndex, nX, nY);
                }
                continue;
            }

            if(m_pstTileInfo){
                auto stInfo = BaseTileInfo(nX, nY);
                if(stInfo.bFileIndex != 255 && stInfo.wTileIndex != 65535){
                    fnDrawTileFunc(stInfo.bFileIndex, stInfo.wTileIndex, nX, nY);
                }
                continue;
            }
        }
    }
}

void Mir2Map::ExtractOneObjectTile(std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc, int nXCnt, int nYCnt)
{
    if(!Valid() || !ValidC(nX, nY)){ return; }

    if(m_Mir2xMap.Valid()){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nIndex = 0; nIndex < 2; ++nIndex){
            if(m_Mir2xMap.ObjectValid(nX, nY, nIndex)){
                uint32_t nKey = m_Mir2xMap.ObjectBaseKey(nX, nY, nIndex);

                int nImageDesc  = ((nKey & 0XFF000000) >> 24);
                int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                int nImageIndex = ((nKey & 0X0000FFFF));
                int nFrameCount = m_Mir2xMap.ObjectFrameCount(nX, nY, nIndex);

                uint32_t nImageColor = ((nImageDesc & 0X01) ? 0X80FFFFFF : 0XFFFFFFFF);

                // no door informaiton, ignore it
                if(nFileIndex != 255 && nImageIndex != 65535 && nFrameCount != 0){
                    for(int nFrame = 0; nFrame < nFrameCount; ++nFrame){
                        if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                                && m_pxTileImage[nFileIndex].CurrentImageValid()){
                            int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                            int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                            if(nBuffLen < nW * nH){
                                delete pBuff;
                                pBuff    = new uint32_t[nW * nH];
                                nBuffLen = nW * nH;
                            }
                            m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                            fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
        delete pBuff;
    }else{

        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;

        {// first layer:
            int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
            if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj1 + GetDoorImageIndex(nXCnt, nYCnt);
                uint32_t nFrameCount = 0;

                bool bBlend = false;

                if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                    bBlend = ((m_pstCellInfo[nArrayNum].bObj1Ani & 0X80) != 0);
                    // didn't use bTickType for decoding
                    // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F);        // for frame count

                    nFrameCount += shAniCnt;
                }

                uint32_t nImageColor = (bBlend ? 0X80FFFFFF : 0XFFFFFFFF);
                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            delete pBuff;
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                        fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                    }
                }
            }
        }

        {// second layer:
            int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
            if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj2 + GetDoorImageIndex(nXCnt, nYCnt);
                uint32_t nFrameCount = 0;

                bool bBlend = false;

                if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                    bBlend = ((m_pstCellInfo[nArrayNum].bObj2Ani & 0X80) != 0);
                    // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F);        // for frame count

                    nFrameCount += shAniCnt;
                }

                uint32_t nImageColor = (bBlend ? 0X80FFFFFF : 0XFFFFFFFF);
                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            delete pBuff;
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                        fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                    }
                }
            }
        }
        delete pBuff;
    }

}

void Mir2Map::ExtractObjectTile(std::function<bool(uint32_t, uint32_t)> fnCheckExistFunc,
        std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc)
{
    if(!Valid()){ return; }

    if(m_Mir2xMap.Valid()){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nYCnt = 0; nYCnt < m_Mir2xMap.H(); ++nYCnt){
            for(int nXCnt = 0; nXCnt < m_Mir2xMap.W(); ++nXCnt){
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(m_Mir2xMap.ObjectValid(nX, nY, nIndex)){
                        uint32_t nKey = m_Mir2xMap.ObjectBaseKey(nX, nY, nIndex);

                        int nImageDesc  = ((nKey & 0XFF000000) >> 24);
                        int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                        int nImageIndex = ((nKey & 0X0000FFFF));
                        int nFrameCount = m_Mir2xMap.ObjectFrameCount(nX, nY, nIndex);

                        uint32_t nImageColor = ((nImageDesc & 0X01) ? 0X80FFFFFF : 0XFFFFFFFF);

                        // no door informaiton, ignore it
                        if(nFileIndex != 255 && nImageIndex != 65535 && nFrameCount != 0){
                            for(int nFrame = 0; nFrame < nFrameCount; ++nFrame){
                                if(fnCheckExistFunc(nFileIndex, nImageIndex + nFrame)){
                                    continue;
                                }
                                if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                                        && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                    int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                    int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                    if(nBuffLen < nW * nH){
                                        delete pBuff;
                                        pBuff    = new uint32_t[nW * nH];
                                        nBuffLen = nW * nH;
                                    }
                                    m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                                    fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                                }
                            }
                        }
                    }
                }
            }
        }
        delete pBuff;
    }else{

        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){
            for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; ++nXCnt){
                int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;

                {// first layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                        uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj1 + GetDoorImageIndex(nXCnt, nYCnt);
                        uint32_t nFrameCount = 0;

                        bool bBlend = false;

                        if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                            bBlend = ((m_pstCellInfo[nArrayNum].bObj1Ani & 0X80) != 0);
                            // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F);        // for frame count

                            nFrameCount += shAniCnt;
                        }

                        uint32_t nImageColor = ( bBlend ? 0X80FFFFFF : 0XFFFFFFFF);
                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(fnCheckExistFunc(nFileIndex, nImageIndex + nFrame)){
                                continue;
                            }
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    delete pBuff;
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                            }
                        }
                    }
                }

                {// second layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                        uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj2 + GetDoorImageIndex(nXCnt, nYCnt);
                        uint32_t nFrameCount = 0;

                        bool bBlend = false;

                        if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                            bBlend = ((m_pstCellInfo[nArrayNum].bObj1Ani & 0X80) != 0);
                            // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F);        // for frame count

                            nFrameCount += shAniCnt;
                        }

                        uint32_t nImageColor = ( bBlend ? 0X80FFFFFF : 0XFFFFFFFF);
                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(fnCheckExistFunc(nFileIndex, nImageIndex + nFrame)){
                                continue;
                            }
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    delete pBuff;
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, nImageColor, 0XFFFFFFFF, 0XFFFFFFFF);
                                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                            }
                        }
                    }
                }
            }
        }
        delete pBuff;
    }
}

void Mir2Map::DrawObjectTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<bool(uint32_t, uint32_t, Fl_Shared_Image * &, int, int, int)> fnCheckFunc,
        std::function<void(uint32_t, uint32_t, Fl_Shared_Image *, int, int, int)> fnDrawObjFunc)
{
    if(!Valid()){ return; }

    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, Width() - 1);
    nStopCellY  = (std::min)(nStopCellY, Height() - 1);

    if(m_Mir2xMap.Valid()){
        for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
            for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(m_Mir2xMap.ObjectValid(nXCnt, nYCnt, nIndex)){
                        uint32_t nKey = m_Mir2xMap.ObjectKey(nXCnt, nYCnt, nIndex);
                        int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                        int nImageIndex = ((nKey & 0X0000FFFF));
                        if(nFileIndex != 255 && nImageIndex != 65535){
                            Fl_Shared_Image *p = nullptr;
                            if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, nIndex)){
                                fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, nIndex);
                            }
                        }
                    }
                }
            }
        }
    }else{
        for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
            for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
                int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;

                {// first layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                        uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj1 + GetDoorImageIndex(nXCnt, nYCnt);
                        if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                            uint8_t bTickType = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4;
                            int16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F);

                            nImageIndex += m_bAniTileFrame[bTickType][shAniCnt];
                        }

                        Fl_Shared_Image *p = nullptr;
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, 0)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, 0);
                        }
                    }
                }

                {// second layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                        uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj2 + GetDoorImageIndex(nXCnt, nYCnt);
                        if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                            uint8_t bTickType = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4;
                            int16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F);

                            nImageIndex += m_bAniTileFrame[bTickType][shAniCnt];
                        }

                        Fl_Shared_Image *p = nullptr;
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, 1)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt, 1);
                        }
                    }
                }
            }
        }
    }
}

void Mir2Map::ExtractGroundInfo(std::function<void(uint32_t, int, int, int)> fnSetGroundInfoFunc)
{
    if(!Valid()){ return; }

    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            if(m_Mir2xMap.Valid()){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(m_Mir2xMap.GroundValid(nX, nY, nIndex)){
                        uint32_t nGrondInfo = 0X80000000 + m_Mir2xMap.GroundInfo(nX, nY, nIndex);
                        fnSetGroundInfoFunc(nGroundInfo, nX, nY, nIndex);
                    }
                }
                continue;
            }
            if(m_pstCellInfo){
                uint32_t nRawGroundInfo = (CellInfo(nX, nY).bFlag & 0X01) ? 0X80000000 : 0X00000000;
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 0);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 1);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 2);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 3);
                continue;
            }

            fnSetGroundInfoFunc(0X00000000, nX, nY, 0);
            fnSetGroundInfoFunc(0X00000000, nX, nY, 1);
            fnSetGroundInfoFunc(0X00000000, nX, nY, 2);
            fnSetGroundInfoFunc(0X00000000, nX, nY, 3);
        }
    }
}

void Mir2Map::SetAniTileFrame(int nLoopTime)
{
    // m_bAniTileFrame[i][j]:
    //     i: denotes how fast the animation is.
    //     j: denotes how many frames the animation has.

    if(!ValidC()){ return; }
    if(m_Mir2xMap.Valid()){
        m_Mir2xMap.UpdateFrame(nLoopTime);
        return;
    }

    uint32_t dwDelayMS[] = {150, 200, 250, 300, 350, 400, 420, 450};

    for(int nCnt = 0; nCnt < 8; ++nCnt){
        m_dwAniSaveTime[nCnt] += nLoopTime;
        if(m_dwAniSaveTime[nCnt] > dwDelayMS[nCnt]){
            for(int nFrame = 0; nFrame < 16; ++nFrame){
                m_bAniTileFrame[nCnt][nFrame]++;
                if(m_bAniTileFrame[nCnt][nFrame] >= nFrame){
                    m_bAniTileFrame[nCnt][nFrame] = 0;
                }
            }
            m_dwAniSaveTime[nCnt] = 0;
        }
    }
}

bool Mir2Map::CropSize(int nStartX, int nStartY, int nW, int nH)
{
    // for mir2map, we only support 2M * 2N crop
    // since tile is of size 2*2 as cell
    // and then we also need to expand for 8*8
    if(!Valid()){ return false; }

    if(m_Mir2xMap.Valid()){
        // mir2x map is read-only
        return false;
    }

    nStartX = (std::max)(0, nStartX);
    nStartY = (std::max)(0, nStartY);

    if(nStartX % 2){ nStartX--; nW++; }
    if(nStartY % 2){ nStartY--; nH++; }
    if(nW % 2){ nW++; }
    if(nH % 2){ nH++; }

    nW = (std::min)(nW, Width()  - nStartX);
    nH = (std::min)(nH, Height() - nStartY);

    if(true
            && nStartX == 0
            && nStartY == 0
            && nW == Width()
            && nH == Height())
    {
        // no need to crop actually
        return true;
    }

    int  nMapLoadSize = nW * nH;
    auto pNewTileInfo = new TILEINFO[nMapLoadSize / 4];
    auto pNewCellInfo = new CELLINFO[nMapLoadSize];

    for(int nX = 0; nX < nW; ++nX){
        for(int nY = 0; nY < nH; ++nY){
            pNewTileInfo[nY / 2 + (nX / 2) * nH / 2] = m_pstTileInfo[(nY + nStartY) / 2 
                + ((nX + nStartX) / 2) * m_stMapFileHeader.shHeight / 2];

            pNewCellInfo[nY + nX * nH] = 
                m_pstCellInfo[(nY + nStartY) + (nX + nStartX) * m_stMapFileHeader.shHeight];
        }
    }

    delete [] m_pstTileInfo;
    delete [] m_pstCellInfo;

    m_pstTileInfo = pNewTileInfo;
    m_pstCellInfo = pNewCellInfo;

    m_stMapFileHeader.shWidth  = nW;
    m_stMapFileHeader.shHeight = nH;

    return true;
}

bool Mir2Map::NewValid()
{
    return m_Mir2xMap.Valid();
}

bool Mir2Map::Valid()
{
    if(m_Mir2xMap.Valid() || m_Valid){
        return true;
    }
    return false;
}

bool Mir2Map::ValidC(int nX, int nY)
{
    // assume valid map
    if(m_Mir2xMap.Valid()){
        return m_Mir2xMap.ValidC(nX, nY);
    }else{
        return true
            && nX >= 0
            && nX <  m_stMapFileHeader.shWidth
            && nY >= 0
            && nY <  m_stMapFileHeader.shHeight;
    }
}

bool Mir2Map::ValidP(int nX, int nY)
{
    // assume valid map
    if(m_Mir2xMap.Valid()){
        return m_Mir2xMap.ValidP(nX, nY);
    }else{
        return true
            && nX >= 0
            && nX <  48 * m_stMapFileHeader.shWidth
            && nY >= 0
            && nY <  32 * m_stMapFileHeader.shHeight;
    }
}

bool Mir2Map::EmptyGroundInfo(int nStartX, int nStartY, int nIndex)
{
    if(m_Mir2xMap.Valid()){
        return m_Mir2xMap.EmptyGroundInfo(nStartX, nStartY, nIndex);
    }else{
        extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;
        return (g_GroundInfo[nStartX][nStartY][nIndex] & 0X80000000) == 0;
    }
}

uint8_t Mir2Map::GroundInfo(int nStartX, int nStartY, int nIndex)
{
    if(m_Mir2xMap.Valid()){
        return m_Mir2xMap.GroundInfo(nStartX, nStartY, nIndex);
    }else{
        extern std::vector<std::vector<std::array<uint32_t, 4>>> g_GroundInfo;
        return (uint8_t)(g_GroundInfo[nStartX][nStartY][nIndex] & 0X000000FF);
    }
}

// get block type, assume valid parameters, parameters:
//
//      nStartX                 : ..
//      nStartY                 : ..
//      nIndex                  : ignore it when nSize != 0
//      nSize                   :
//
//      return                  : define block type ->
//                                      0: no info in this block
//                                      1: there is full-filled unified info in this block
//                                      2: there is full-filled different info in this block
//                                      3: there is empty/filled combined info in this block
int Mir2Map::GroundInfoBlockType(int nStartX, int nStartY, int nIndex, int nSize)
{
    // assume valid map, valid parameters
    if(nSize == 0){
        return EmptyGroundInfo(nStartX, nStartY, nIndex) ? 0 : 1;
    }else{
        bool bFindEmpty = false;
        bool bFindFill  = false;
        bool bFindDiff  = false;

        bool bInited = false;
        uint8_t nGroundInfoSample = 0;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(EmptyGroundInfo(nX, nY, nIndex)){
                        bFindEmpty = true;
                    }else{
                        bFindFill = true;
                        if(bInited){
                            if(nGroundInfoSample != GroundInfo(nX, nY, nIndex)){
                                bFindDiff = true;
                            }
                        }else{
                            nGroundInfoSample = GroundInfo(nX, nY, nIndex);
                        }
                    }
                }
            }
        }

        if(bFindFill == false){
            // no information at all
            return 0;
        }else{
            // do have information
            if(bFindEmpty == false){
                // all filled
                if(bFindDiff == false){
                    // all filled and no difference exists
                    return 1;
                }else{
                    // all filled and there is difference
                    return 2;
                }
            }else{
                // there are filled and empty, combined
                return 3;
            }
        }
    }
}

void Mir2Map::CompressGroundInfoPreOrder(
        int nStartX, int nStartY, int nSize,
        std::vector<bool> &stGroundInfoBitV, std::vector<uint32_t> &stGroundInfoV)
{
    if(!ValidC(nStartX, nStartY)){ return; }

    int nType = GroundInfoBlockType(nStartX, nStartY, -1, nSize); // nSize >= 1 always
    if(nType != 0){
        // there is informaiton in this box
        stGroundInfoBitV.push_back(true);
        if(nSize == 1){
            // there is info, and it's last level, end of recursion
            if(nType == 2 || nType == 3){
                // there is info that should parse one by one
                // maybe empty/filled combined or fullfilled with different attributes
                stGroundInfoBitV.push_back(true);
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(GroundInfoBlockType(nStartX, nStartY, nIndex, 0) == 0){
                        // can only return 0 / 1
                        stGroundInfoBitV.push_back(false);
                    }else{
                        stGroundInfoBitV.push_back(true);
                        stGroundInfoV.push_back(GroundInfo(nStartX, nStartY, nIndex));
                    }
                }
            }else{
                // ntype == 1 here, full-filled with unified info
                stGroundInfoBitV.push_back(false);
                stGroundInfoV.push_back(GroundInfo(nStartX, nStartY, 0));
            }
        }else{
            // not the last level, and there is info
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stGroundInfoBitV.push_back(true);
                CompressGroundInfoPreOrder(
                        nStartX, nStartY,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX + nSize / 2, nStartY,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX, nStartY + nSize / 2,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX + nSize / 2, nStartY + nSize / 2,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
            }else{
                // nType == 1 here
                stGroundInfoBitV.push_back(false);
                stGroundInfoV.push_back(GroundInfo(nStartX, nStartY, 0));
            }
        }
    }else{
        // nType == 0, no information in this box
        stGroundInfoBitV.push_back(false);
    }
}

void Mir2Map::CompressGroundInfoPreOrder(
        int nStartX, int nStartY, int nSize,
        std::vector<bool> &stGroundInfoBitV, std::vector<uint32_t> &stGroundInfoV)
{
    if(!ValidC(nStartX, nStartY)){ return; }

    int nType = GroundInfoBlockType(nStartX, nStartY, -1, nSize); // nSize >= 1 always
    if(nType == 0){
        // no information in this box
        stGroundInfoBitV.push_back(false);
    }else{
        // there is informaiton in this box
        stGroundInfoBitV.push_back(true);
        if(nType == 1){
            // unified information
            stGroundInfoBitV.push_back(false);
            stGroundInfoV.push_back(GroundInfo(nStartX, nStartY, 0));
        }else{
            // combined grid
            // 1. may be empty/filled combined
            // 2. may be full-filled with different attributes
            //
            // for both possibility we further recursively retrieve
            // don't read directly in case 2
            // think the situation: large grid of same attributes with on exception
            stGroundInfoBitV.push_back(true);
            if(nSize == 1){
                // last level, no recursion anymore
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(GroundInfoBlockType(nStartX, nStartY, nIndex, 0) == 0){
                        // can only return 0 / 1
                        stGroundInfoBitV.push_back(false);
                    }else{
                        stGroundInfoBitV.push_back(true);
                        stGroundInfoV.push_back(GroundInfo(nStartX, nStartY, nIndex));
                    }
                }
            }else{
                // recursion
                CompressGroundInfoPreOrder(
                        nStartX, nStartY,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX + nSize / 2, nStartY,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX, nStartY + nSize / 2,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
                CompressGroundInfoPreOrder(
                        nStartX + nSize / 2, nStartY + nSize / 2,
                        nSize / 2,
                        stGroundInfoBitV, stGroundInfoV);
            }
        }
    }
}

void Mir2Map::CompressBaseTileInfoPreOrder(
        int nStartX, int nStartY, int nSize,
        std::vector<bool> &stTileInfoBitV, std::vector<uint32_t> &stTileInfoV)
{
    if(!ValidC(nStartX, nStartY)){ return; }

    int nType = BaseTileInfoBlockType(nStartX, nStartY, nSize);
    if(nType == 0){
        // no information in this box
        stBaseTileInfoBitV.push_back(false);
    }else{
        // there is informaiton in this box
        stBaseTileInfoBitV.push_back(true);
        if(nType == 1){
            // unified information
            stBaseTileInfoBitV.push_back(false);
            AppendBaseTileInfo(stBaseTileInfoV, nStartX, nStartY);
        }else{
            // combined grid
            stBaseTileInfoBitV.push_back(true);
            if(nSize == 2){
                // last level, no recursion anymore
                if(CanGetOneBaseTileInfo(nStartX, nStartY)){
                    stBaseTileInfoBitV.push_back(true);
                    AppendBaseTileInfo(stBaseTileInfoV, nStartX, nStartY);
                }else{
                    stBaseTileInfoBitV.push_back(false);
                }
            }else{
                // recursion
                CompressBaseTileInfoPreOrder(
                        nStartX, nStartY,
                        nSize / 2,
                        stBaseTileInfoBitV, stBaseTileInfoV);
                CompressBaseTileInfoPreOrder(
                        nStartX + nSize / 2, nStartY,
                        nSize / 2,
                        stBaseTileInfoBitV, stBaseTileInfoV);
                CompressBaseTileInfoPreOrder(
                        nStartX, nStartY + nSize / 2,
                        nSize / 2,
                        stBaseTileInfoBitV, stBaseTileInfoV);
                CompressBaseTileInfoPreOrder(
                        nStartX + nSize / 2, nStartY + nSize / 2,
                        nSize / 2,
                        stBaseTileInfoBitV, stBaseTileInfoV);
            }
        }
    }
}

void Mir2Map::CompressGroundInfo(std::vector<bool> &stGroundInfoBitV, std::vector<uint8_t> &stGroundInfoV)
{
    stGroundInfoBitV.clear();
    stGroundInfoV.clear();
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressGroundInfoPreOrder(nX, nY, 8, stTileInfoBitV, stTileInfoV);
        }
    }

}

void Mir2Map::CompressBaseTileInfo(std::vector<bool> &stTileInfoBitV, std::vector<uint32_t> &stTileInfoV)
{
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressBaseTileInfoPreOrder(nX, nY, 8, stTileInfoBitV, stTileInfoV);
        }
    }
}

bool Mir2Map::EmptyCellTileBlock(int nStartX, int nStartY, int nSize)
{
    for(int nYCnt = nStartY; nYCnt < nStartY + nSize; ++nYCnt){
        for(int nXCnt = nStartX; nXCnt < nStartX + nSize; ++nXCnt){
            if(m_CellDesc){
                int nArrayNum = nXCnt + nYCnt * m_stMapFileHeader.shWidth;
                // if(m_CellDesc[nArrayNum].dwDesc){
                //     return false;
                // }

                if(false
                        || m_CellDesc[nArrayNum].dwDesc    != 0XFFFFFFFF
                        || m_CellDesc[nArrayNum].dwObject1 != 0XFFFFFFFF
                        || m_CellDesc[nArrayNum].dwObject2 != 0XFFFFFFFF
                        || m_CellDesc[nArrayNum].dwLight   != 0XFFFFFFFF
                  ){
                    return false;
                }
                continue;
            }
            if(m_pstCellInfo){
                int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
                { // door
                    // for door
                    // bDoorIdx first bit show whether there is a door
                    // next 7 bit show the index, but if it's 0, still it's a null door
                    //
                    // then bDoorOffset the first bit show this door is open or close
                    // next 7 bit show the offset of image of open and close
                    //
                    if(true
                            && (m_pstCellInfo[nArrayNum].bDoorIndex & 0X80)> 0 &&
                            (m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F)> 0){
                        return false;
                    }
                }

                {// light
                    if(m_pstCellInfo[nArrayNum].wLigntNEvent != 0){
                        return false;
                    }
                }

                {// first layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                        return false;
                    }
                }
                {// second layer:
                    int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
                    if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void Mir2Map::CompressCellTileInfoPreOrder(int nStartX, int nStartY, int nSize, 
        std::vector<bool> &stCellDescBitV, std::vector<CELLDESC> &stCellDescV)
{
    if(EmptyCellTileBlock(nStartX, nStartY, nSize)){
        stCellDescBitV.push_back(false);
    }else{
        stCellDescBitV.push_back(true);
        if(nSize == 1){
            if(m_CellDesc){
                auto &stDesc = m_CellDesc[nStartX + nStartY * m_stMapFileHeader.shWidth];
                stCellDescV.push_back(stDesc);

            }else if(m_pstCellInfo){

                auto &stCellInfo = CellInfo(nStartX, nStartY);
                CELLDESC stDesc;

                { // converting here
                    // input : CELLINFO stCellInfo
                    // output: CELLDESC stDesc

                    // object info
                    // stDesc.dwObject1 = (((uint32_t)(stCellInfo.wFileIndex & 0XFF00)) << 16)
                    //     + (((uint32_t)(stCellInfo.bObj1Ani)) << 16)
                    //     + ((uint32_t)(stCellInfo.wObj1));
                    // stDesc.dwObject2 = (((uint32_t)(stCellInfo.wFileIndex & 0X00FF)) << 24)
                    //     + (((uint32_t)(stCellInfo.bObj2Ani)) << 16)
                    //     + ((uint32_t)(stCellInfo.wObj2));
                    { // first layer
                        uint32_t nPrecode    = 0;
                        uint32_t nFileIndex  = ((uint32_t)(stCellInfo.wFileIndex & 0XFF00)) << 8;
                        uint32_t nImageIndex = ((uint32_t)(stCellInfo.wObj1));

                        stDesc.dwObject1 = nPrecode + nFileIndex + nImageIndex;
                    }
                    { // second layer
                        uint32_t nPrecode    = 0;
                        uint32_t nFileIndex  = ((uint32_t)(stCellInfo.wFileIndex & 0X00FF)) << 16;
                        uint32_t nImageIndex = ((uint32_t)(stCellInfo.wObj2));

                        stDesc.dwObject2 = nPrecode + nFileIndex + nImageIndex;
                    }

                    // light info
                    //
                    // actually I read the source code
                    // they only use there is or isn't light
                    // but how to draw the light, the parameter is independent from LIGHTINFO struct
                    // they use setted static info
                    //
                    // TODO:
                    // I think this info is abandoned
                    // just put it here
                    stDesc.dwLight = (uint32_t)stCellInfo.wLigntNEvent;

                    // door or dynamic tile info
                    // they share one uint32_t means there couldn't be door and dynamic tile
                    // at the same time
                    // TODO: design this bitset later
                    // stDesc.dwDesc = 0X80000000 // always show it as a door if possible
                    //     + (((uint32_t)stCellInfo.bDoorIndex) << 8)
                    //     + ((uint32_t)stCellInfo.bDoorOffset);

                    {
                        uint32_t nObjAniDesc1 = ((uint32_t)(stCellInfo.bObj1Ani));
                        uint32_t nObjAniDesc2 = ((uint32_t)(stCellInfo.bObj2Ani))    <<  8;
                        uint32_t nDoorOffset  = ((uint32_t)(stCellInfo.bDoorOffset)) << 16;
                        uint32_t nDoorIndex   = ((uint32_t)(stCellInfo.bDoorIndex))  << 24;
                        if(stCellInfo.bDoorIndex > 255){
                            // TODO:
                            // I think this info is abandoned
                            // but if really happens
                            // use a table to make door local to one map
                            // then one map can contain 256 doors
                            printf("door index is of length > 255, loss of information here!!!");
                        }

                        stDesc.dwDesc = nObjAniDesc1 + nObjAniDesc2 + nDoorOffset + nDoorIndex;
                    }
                }
                stCellDescV.push_back(stDesc);
            }else{
                // can never be here
                printf("error!");
            }
        }else{
            CompressCellTileInfoPreOrder(
                    nStartX, nStartY,
                    nSize / 2,
                    stCellDescBitV, stCellDescV);
            CompressCellTileInfoPreOrder(
                    nStartX + nSize / 2, nStartY,
                    nSize / 2,
                    stCellDescBitV, stCellDescV);
            CompressCellTileInfoPreOrder(
                    nStartX, nStartY + nSize / 2,
                    nSize / 2,
                    stCellDescBitV, stCellDescV);
            CompressCellTileInfoPreOrder(
                    nStartX + nSize / 2, nStartY + nSize / 2,
                    nSize / 2,
                    stCellDescBitV, stCellDescV);
        }
    }
}

void Mir2Map::CompressCellTileInfo(
        std::vector<bool> &stCellDescBitV, std::vector<CELLDESC> &stCellDescV)
{
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressCellTileInfoPreOrder(nX, nY, 8, stCellDescBitV, stCellDescV);
        }
    }
}

uint32_t Mir2Map::BitPickOne(uint32_t *pU32BitStream, uint32_t nOffset)
{
    // nOffset can only be even number
    uint32_t nShift = 31 - (nOffset % 32);
    return ((uint32_t)(pU32BitStream[nOffset / 32] & ((uint32_t)0X01) << nShift)) >> nShift;
}

void Mir2Map::SetOneGroundInfoGrid(
        int nStartX, int nStartY, int nSubGrid, uint32_t nGroundInfoAttr)
{
    // be careful! here it's not arranged as old format
    int nOffset = (nStartY * m_stMapFileHeader.shWidth + nStartX) * 4 + (nSubGrid % 4);
    m_GroundInfo[nOffset] = nGroundInfoAttr;
}

void Mir2Map::SetGroundInfoBlock(
        int nStartX, int nStartY, int nSize, uint32_t nGroundInfoAttr)
{
    // this function copy one unique attribute to nSize * nSize * 4 grid
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            SetOneGroundInfoGrid(nX, nY, 0, nGroundInfoAttr);
            SetOneGroundInfoGrid(nX, nY, 1, nGroundInfoAttr);
            SetOneGroundInfoGrid(nX, nY, 2, nGroundInfoAttr);
            SetOneGroundInfoGrid(nX, nY, 3, nGroundInfoAttr);
        }
    }
}

void Mir2Map::ParseGroundInfoStream(int nStartX, int nStartY, int nSize,
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

bool Mir2Map::LoadGroundInfo(
        uint32_t * pU32BitStream, uint32_t,
        uint32_t * pU32GroundInfo, uint32_t)
{
    uint32_t nU32BitStreamOffset  = 0;
    uint32_t nU32GroundInfoOffset = 0;
    for(int nBlkY = 0; nBlkY < Height() / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < Width() / 8; ++nBlkX){
            ParseGroundInfoStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOffset,
                    pU32GroundInfo, nU32GroundInfoOffset);
        }
    }
    return true;
}

void Mir2Map::SetBaseTileBlock(
        int nStartX, int nStartY, int nSize, uint32_t nAttr)
{
    for(int nY = nStartY; nY < nStartY + nSize; nY += 2){
        for(int nX = nStartX; nX < nStartX + nSize; nX += 2){
            m_BaseTileInfo[(nY / 2) * (m_stMapFileHeader.shWidth / 2) + (nX / 2)] = nAttr;
        }
    }
}

void Mir2Map::ParseBaseTileStream(int nStartX, int nStartY, int nSize,
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

bool Mir2Map::LoadBaseTileInfo(
        uint32_t *pU32BitStream,    uint32_t,
        uint32_t *pU32BaseTileInfo, uint32_t)
{

    uint32_t nU32BitStreamOffset    = 0;
    uint32_t nU32BaseTileInfoOffset = 0;
    for(int nBlkY = 0; nBlkY < Height() / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < Width() / 8; ++nBlkX){
            ParseBaseTileStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOffset,
                    pU32BaseTileInfo, nU32BaseTileInfoOffset);
        }
    }
    return true;
}

void Mir2Map::SetCellDescBlock(
        int nStartX, int nStartY, int nSize, const CELLDESC & stCellDesc)
{
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            m_CellDesc[nY * m_stMapFileHeader.shWidth + nX] = stCellDesc;
        }
    }
}

void Mir2Map::ParseCellDescStream(int nStartX, int nStartY, int nSize,
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

bool Mir2Map::LoadCellDesc(
        uint32_t *pU32BitStream, uint32_t,
        CELLDESC *pCellDesc,     uint32_t)
{

    uint32_t nU32BitStreamOffset = 0;
    uint32_t nCellDescOffset     = 0;
    for(int nBlkY = 0; nBlkY < Height() / 8; ++nBlkY){
        for(int nBlkX = 0; nBlkX < Width() / 8; ++nBlkX){
            ParseCellDescStream(nBlkX * 8, nBlkY * 8, 8,
                    pU32BitStream, nU32BitStreamOffset,
                    pCellDesc, nCellDescOffset);
        }
    }
    return true;
}

std::string Mir2Map::MapInfo()
{
    if(Valid()){
        std::string szMapInfo;
        char szTmpInfo[128];

        std::sprintf(szTmpInfo, "Width:%d\n", m_stMapFileHeader.shWidth);
        szMapInfo += szTmpInfo;

        std::sprintf(szTmpInfo, "Height:%d\n", m_stMapFileHeader.shHeight);
        szMapInfo += szTmpInfo;

        std::sprintf(szTmpInfo, "Alpha:%d\n", m_AlphaBlend);
        szMapInfo += szTmpInfo;

        std::sprintf(szTmpInfo, "Door:%d\n", m_DoorCount);
        szMapInfo += szTmpInfo;

        std::sprintf(szTmpInfo, "Light:%d\n", m_DoorCount);
        szMapInfo += szTmpInfo;

        return szMapInfo;
    }else{
        return "Invalid Map";
    }
}

bool Mir2Map::NewLoadMap(const char *szFullName)
{
    // disable the mir2 map
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;

    m_Valid = false;

    return m_Mir2xMap.Load(szFullName);
}

void Mir2Map::Optimize()
{
    // try to remove some unnecessary tile/cell

    // tile
    for(int nY = 0; nY < Height(); ++nY){
        for(int nX = 0; nX < Width(); ++nX){
            OptimizeBaseTile(nX, nY);
            OptimizeCell(nX, nY);
        }
    }
}

void Mir2Map::OptimizeBaseTile(int nX, int nY)
{
    if(nX % 2 || nY % 2){
        return;
    }

    if(m_BaseTileInfo){
        int nArrNum = nX / 2 + nY * m_stMapFileHeader.shWidth / 4;
        uint32_t &nBaseTileInfo= m_BaseTileInfo[nArrNum];

        { // drop 000200020.PNG
            // if(nBaseTileInfo == 0X80020014){
            //     nBaseTileInfo = 0;
            // }
            if(nBaseTileInfo == 0X00020014){
                nBaseTileInfo = 0XFFFFFFFF;
            }
        }
    }

    if(m_pstTileInfo){
        int nArrNum = nY / 2 + nX * m_stMapFileHeader.shHeight / 4;
        auto &stBaseTileInfo = m_pstTileInfo[nArrNum];

        { // drop 000200020.PNG
            if(stBaseTileInfo.bFileIndex == 2 && stBaseTileInfo.wTileIndex == 20){
                stBaseTileInfo = {255, 65535};
            }
        }
    }
}

void Mir2Map::OptimizeCell(int, int)
{
}

void Mir2Map::SetMapInfo()
{
    if(!m_Valid){ return; }

    // Tile Infomation doesn't include any for alpha-blend, light or door

    for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; ++nXCnt){
        for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){

            int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
            {
                // for light info
                if((m_pstCellInfo[nArrayNum].wLigntNEvent != 0)
                        || (m_pstCellInfo[nArrayNum].wLigntNEvent & 0X0007) == 1){
                    // TODO quite doubt of this logic
                    // seems the 15~4 for specified light-frog
                    // 3~0 for general light-frog
                    m_LightCount++;
                }
            }

            {
                // for door info
                // for one cell there are two object fields but only one door info field

                // bDoorIndex & 0X80 for whether there is a door
                // bDoorIndex & 0X7F for door index, if non-zero
                // bDoorOffset & 0X80 for open/close the door
                // bDoorOffset & 0X7F for door image offset
                if((m_pstCellInfo[nArrayNum].bDoorIndex & 0X80)
                        && (m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F) != 0){
                    m_DoorCount++;

                    printf("%d, %d, %d\n", nXCnt, nYCnt, GetDoorImageIndex(nXCnt, nYCnt));
                }
            }

            // for light info
            {
                // first layer:
                int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
                if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                    if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                        if(m_pstCellInfo[nArrayNum].bObj1Ani & 0X80){
                            m_AlphaBlend++;
                        }
                    }
                }
            }

            {
                // second layer:
                int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
                if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                    if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                        if(m_pstCellInfo[nArrayNum].bObj2Ani & 0X80){
                            m_AlphaBlend++;
                        }
                    }
                }
            }
        }
    }
}

uint8_t Mir2Map::GetDoor(int nXCnt, int nYCnt)
{
    uint8_t bRes = 0;
    int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
    if(m_pstCellInfo[nArrayNum].bDoorIndex & 0X80){
        bRes = (m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F);
    }
    return bRes;
}

void Mir2Map::OpenDoor(int nX, int nY, uint8_t nDoorIndex)
{
    for(int nCntY = nY - 8; nCntY < nY + 10; nCntY++){
        for(int nCntX = nX - 8; nCntX < nX + 10; nCntX++){
            if(true
                    && nCntX >= 0
                    && nCntY >= 0
                    && nCntX <  m_stMapFileHeader.shWidth
                    && nCntY <  m_stMapFileHeader.shHeight
              ){
                int nArrayNum = nCntY + nCntX * m_stMapFileHeader.shHeight;
                if((m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F) == nDoorIndex){
                    m_pstCellInfo[nArrayNum].bDoorOffset |= 0X80;
                }
            }
        }
    }
}

void Mir2Map::CloseDoor(int nX, int nY, uint8_t nDoorIndex)
{
    for(int nCntY = nY - 8; nCntY < nY + 10; nCntY++){
        for(int nCntX = nX - 8; nCntX < nX + 10; nCntX++){
            if(true
                    && nCntX >= 0
                    && nCntY >= 0
                    && nCntX <  m_stMapFileHeader.shWidth
                    && nCntY <  m_stMapFileHeader.shHeight
              ){
                int nArrayNum = nCntY + nCntX * m_stMapFileHeader.shHeight;
                if((m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F) == nDoorIndex){
                    m_pstCellInfo[nArrayNum].bDoorOffset &= 0X7F;
                }
            }
        }
    }
}

void Mir2Map::OpenAllDoor()
{
    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            uint8_t nRes = GetDoor(nX, nY);
            if(nRes){
                OpenDoor(nX, nY, nRes);
            }
        }
    }
}

void Mir2Map::CloseAllDoor()
{
    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            uint8_t nRes = GetDoor(nX, nY);
            if(nRes){
                CloseDoor(nX, nY, nRes);
            }
        }
    }
}
