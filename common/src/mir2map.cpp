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
int Mir2Map::GroundBlockType(int nStartX, int nStartY, int nIndex, int nSize)
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
// parser for ground information
// block type can be 0 1 2 3
// but the parse take 2 and 3 in the same action
//
// this is because for case 2, there is large possibility that
// a huge grid with a/b combination of a, with one exception of b
// in this case, arrange a/b combination in linear stream is bad
void Mir2Map::CompressGroundPreOrder(int nX, int nY, int nSize,
        std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    if(!ValidC(nX, nY)){ return; }

    int nType = GroundBlockType(nX, nY, -1, nSize); // nSize >= 1 always
    if(nType != 0){
        // there is informaiton in this box
        stMarkV.push_back(true);
        if(nSize == 1){
            // there is info, and it's last level, end of recursion
            if(nType == 2 || nType == 3){
                // there is info, maybe a/0 or a/b combined
                // this is at last level, parser should parse it one by one
                stMarkV.push_back(true);
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(GroundBlockType(nX, nY, nIndex, 0) == 0){
                        // when at last level, GroundBlockType() can only return 0 / 1
                        // equivlent to !EmptyGroundInfo()
                        stMarkV.push_back(false);
                    }else{
                        stMarkV.push_back(true);
                        RecordGround(stDataV, nX, nY, nIndex);
                    }
                }
            }else{
                // ntype == 1 here, a/a full-filled with unified info
                stMarkV.push_back(false);
                RecordGround(stDataV, nX, nY, 0);
            }
        }else{
            // not the last level, and there is info
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stMarkV.push_back(true);
                CompressGroundPreOrder(nX            , nY            , nSize / 2, stMarkV, stDataV);
                CompressGroundPreOrder(nX + nSize / 2, nY            , nSize / 2, stMarkV, stDataV);
                CompressGroundPreOrder(nX            , nY + nSize / 2, nSize / 2, stMarkV, stDataV);
                CompressGroundPreOrder(nX + nSize / 2, nY + nSize / 2, nSize / 2, stMarkV, stDataV);
            }else{
                // nType == 1 here, unified info
                stMarkV.push_back(false);
                RecordGround(stDataV, nX, nY, 0);
            }
        }
    }else{
        // nType == 0, no information in this box
        stMarkV.push_back(false);
    }
}

void Mir2Map::RecordGround(std::vector<uint8_t> &stDataV, int nX, int nY, int nIndex)
{
    stDataV.push_back(Ground(nX, nY, nIndex));
}

void Mir2Map::RecordLight(std::vector<uint8_t> &stDataV, int nX, int nY)
{
    uint8_t nLD = ((Light(nX, nY) & 0X00FF) >> 0);
    uint8_t nHD = ((Light(nX, nY) & 0XFF00) >> 8);

    stDataV.push_back(nLD);
    stDataV.push_back(nHD);
}

void Mir2Map::RecordObject(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV, int nX, int nY, int nIndex)
{
    bool bGroundObj = GroundObjectValid(nX, nY, nIndex);
    uint32_t nObjDesc = ObjectDesc(nX, nY, nIndex);

    stMarkV.push_back(bGroundObj);

    stDataV.push_back((uint8_t)((nObjDesc & 0X000000FF ) >>  0));
    stDataV.push_back((uint8_t)((nObjDesc & 0X0000FF00 ) >>  8));
    stDataV.push_back((uint8_t)((nObjDesc & 0X00FF0000 ) >> 16));
    stDataV.push_back((uint8_t)((nObjDesc & 0XFF000000 ) >> 24));
}

void Mir2Map::RecordTile(std::vector<uint8_t> &stDataV, int nX, int nY)
{
    uint32_t nTileDesc = TileDesc(nX, nY, nIndex);

    stDataV.push_back((uint8_t)((nTileDesc & 0X000000FF ) >>  0));
    stDataV.push_back((uint8_t)((nTileDesc & 0X0000FF00 ) >>  8));
    stDataV.push_back((uint8_t)((nTileDesc & 0X00FF0000 ) >> 16));
    stDataV.push_back((uint8_t)((nTileDesc & 0XFF000000 ) >> 24));
}

// Light is simplest one
void Mir2Map::CompressLightPreOrder(int nX, int nY, int nSize,
        std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    if(!ValidC(nX, nY)){ return; }

    int nType = LightBlockType(nX, nY, nSize);
    if(nType != 0){
        // there is informaiton in this box
        stMarkV.push_back(true);
        if(nSize == 1){
            // there is info, and it's last level, so nType can only be 1
            // end of recursion
            RecordLight(stDataV, nX, nY);
        }else{
            // there is info, and it's not the last level
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stMarkV.push_back(true);
                CompressLightPreOrder(nX            , nY            , nSize / 2, stMarkV, stDataV);
                CompressLightPreOrder(nX + nSize / 2, nY            , nSize / 2, stMarkV, stDataV);
                CompressLightPreOrder(nX            , nY + nSize / 2, nSize / 2, stMarkV, stDataV);
                CompressLightPreOrder(nX + nSize / 2, nY + nSize / 2, nSize / 2, stMarkV, stDataV);
            }else{
                // nType == 1 here, unified info
                stMarkV.push_back(false);
                RecordLight(stDataV, nX, nY);
            }
        }
    }else{
        // nType == 0, no information in this box
        stMarkV.push_back(false);
    }
}

// tile compression is relatively simple
void Mir2Map::CompressTilePreOrder(int nX, int nY, int nSize,
        std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    if(!ValidC(nX, nY)){ return; }

    int nType = ObjectBlockType(nX, nY, nIndex, nSize);
    if(nType != 0){
        // there is informaiton in this box
        stMarkV.push_back(true);
        if(nSize == 1){
            // there is info, and it's last level, so nType can only be 1
            // end of recursion
            stDataV.push_back(Light(nX, nY));
            RecordObject(stMarkV, stDataV, nX, nY, nIndex);
        }else{
            // there is info, and it's not the last level
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stMarkV.push_back(true);
                CompressObjectPreOrder(nX            , nY            , nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX + nSize / 2, nY            , nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX            , nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX + nSize / 2, nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
            }else{
                // nType == 1 here, unified info
                stMarkV.push_back(false);
                RecordObject(stMarkV, stDataV, nX, nY, nIndex);
            }
        }
    }else{
        // nType == 0, no information in this box
        stMarkV.push_back(false);
    }
}

// object compression should take care of ground/overground info in mark vect
void Mir2Map::CompressObjectPreOrder(int nX, int nY, int nIndex, int nSize,
        std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    if(!ValidC(nX, nY)){ return; }

    int nType = ObjectBlockType(nX, nY, nIndex, nSize);
    if(nType != 0){
        // there is informaiton in this box
        stMarkV.push_back(true);
        if(nSize == 1){
            // there is info, and it's last level, so nType can only be 1
            // end of recursion
            stDataV.push_back(Light(nX, nY));
            RecordObject(stMarkV, stDataV, nX, nY, nIndex);
        }else{
            // there is info, and it's not the last level
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stMarkV.push_back(true);
                CompressObjectPreOrder(nX            , nY            , nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX + nSize / 2, nY            , nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX            , nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
                CompressObjectPreOrder(nX + nSize / 2, nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
            }else{
                // nType == 1 here, unified info
                stMarkV.push_back(false);
                RecordObject(stMarkV, stDataV, nX, nY, nIndex);
            }
        }
    }else{
        // nType == 0, no information in this box
        stMarkV.push_back(false);
    }
}

void Mir2Map::CompressLight(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressLightPreOrder(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

void Mir2Map::CompressGroundInfo(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressGroundPreOrder(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

void Mir2Map::CompressTileInfo(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressTileInfoPreOrder(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

void Mir2Map::CompressCellInfo(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < Height(); nY += 8){
        for(int nX = 0; nX < Width(); nX += 8){
            CompressCellInfofoPreOrder(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

std::string Mir2Map::MapInfo()
{
    if(!Valid()){
        return "Invalid map";
    }else if(m_Mir2xMap.Valid()){
        return "MapInfo is unsupported for mir2x map";
    }else{
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
    if(!Valid() || m_Mir2xMap.Valid()){ return; }

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
    // TODO
}

void Mir2Map::OptimizeCell(int, int)
{
    // TODO
}

void Mir2Map::SetMapInfo()
{
    if(!Valid()){ return; }

    if(m_Mir2xMap.Valid()){
        // Mir2xMap doesn't support this
        return;
    }

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
    if(!Valid()){ return 0; }
    if(m_Mir2xMap.Valid()){ return 0; }

    uint8_t bRes = 0;
    int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
    if(m_pstCellInfo[nArrayNum].bDoorIndex & 0X80){
        bRes = (m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F);
    }
    return bRes;
}

void Mir2Map::OpenDoor(int nX, int nY, uint8_t nDoorIndex)
{
    if(!Valid()){ return; }
    if(m_Mir2xMap.Valid()){ return; }

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
    if(!Valid()){ return; }
    if(m_Mir2xMap.Valid()){ return; }

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
    if(!Valid()){ return; }
    if(m_Mir2xMap.Valid()){ return; }

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
    if(!Valid()){ return; }
    if(m_Mir2xMap.Valid()){ return; }

    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            uint8_t nRes = GetDoor(nX, nY);
            if(nRes){
                CloseDoor(nX, nY, nRes);
            }
        }
    }
}


void Mir2Map::ClearBuf()
{
    m_W = 0;
    m_H = 0;
    m_BufValid = false;
}

void Mir2Map::InitBuf()
{
    if(m_W == 0 || m_H == 0 || m_W % 2 || m_H % 2){ return; }

    // m_BufLight[nX][nY]
    m_BufLight = std::vector<std::vector<uint16_t>>(m_W, std::vector<uint16_t>(m_H, 0));
    // m_BufLightMark[nX][nY]
    m_BufLightMark = std::vector<std::vector<int>>(m_W, std::vector<uint32_t>(m_H, 0));

    // m_BufTile[nX][nY]
    m_BufTile = std::vector<std::vector<uint32_t>>(m_W / 2, std::vector<uint32_t>(m_H / 2, 0));
    // m_BufTileMark[nX][nY]
    m_BufTileMark = std::vector<std::vector<int>>(m_W / 2, std::vector<uint32_t>(m_H / 2, 0));

    // m_BufObj[nX][nY][0]
    m_BufObj = std::vector<std::vector<std::array<uint32_t, 2>>>(
            m_W, std::vector<std::array<uint32_t, 2>>(m_H, {0, 0}));
    // m_BufObjMark[nX][nY][0]
    m_BufObjMark = std::vector<std::vector<std::array<int, 2>>>(
            m_W, std::vector<std::array<int, 2>>(m_H, {0, 0}));
    // m_BufGroundObjMark[nX][nY][0]
    m_BufGroundObjMark = std::vector<std::vector<std::array<int, 2>>>(
            m_W, std::vector<std::array<int, 2>>(m_H, {0, 0}));

    // m_BufGround[nX][nY][0]
    m_BufGround = std::vector<std::vector<std::array<uint8_t, 4>>>(
            m_W, std::vector<std::array<uint8_t, 4>>(m_H, {0, 0, 0, 0}));
    // m_BufGroundMark[nX][nY][0]
    m_BufGroundMark = std::vector<std::vector<std::array<int, 4>>>(
            m_W, std::vector<std::array<int, 4>>(m_H, {0, 0, 0, 0}));
}

bool Mir2Map::MakeBuf()
{
    // after load a map by mir2x map or old mir2 map
    // always make a buffer of it, then we can edit on this buffer
    //
    // all queries and operations are on this buffer
    // fixed format maps are only for one-shoot usage
    //
    // which simplifies outside funtion handle design

    ClearBuf();

    if(m_Mir2xMap.Valid()){
        m_W = m_Mir2xMap.W();
        m_H = m_Mir2xMap.H();
    }else if(m_Valid){
        m_W = m_stMapFileHeader.shWidth;
        m_H = m_stMapFileHeader.shHeight;
    }else{
        // no valid map now
        return false;
    }

    InitBuf();

    for(int nY = 0; nY < m_H; ++nY){
        for(int nX = 0; nX < m_W; ++nX){
            // light
            if(!(nX % 2) && !(nY % 2)){
                SetTile(nX / 2, nY / 2);
            }

            SetBufLight(nX, nY);

            SetBufObj(nX, nY, 0);
            SetBufObj(nX, nY, 1);

            SetBufGround(nX, nY, 0);
            SetBufGround(nX, nY, 1);
            SetBufGround(nX, nY, 2);
            SetBufGround(nX, nY, 3);
        }
    }

    m_BufValid = true;
}


void Mir2Map:SetBufTile(int nX, int nY)
{
    if(m_Mir2xMap.Valid()){
        // mir2x map
        if(m_Mir2xMap.TileValid(nX, nY)){
            m_BufTile[nX][nY] = m_Mir2xMap.Tile(nX, nY);
            m_BufTileMark[nX][nY] = 1;
        }
    }else if(m_Valid){
        int nFileIndex  = m_pstTileInfo[(nY / 2) + (nX / 2)*m_stMapFileHeader.shHeight / 2].bFileIndex;
        int nImageIndex = m_pstTileInfo[(nY / 2) + (nX / 2)*m_stMapFileHeader.shHeight / 2].wTileIndex;

        if(nFileIndex != 255 && nImageIndex != 65535){
            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                    m_pxTileImage[nFileIndex].CurrentImageValid()){
                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                if(nW * nH > 0){
                    // 8 bit : unused
                    // 8 bit : file index
                    //16 bit : image index
                    m_BufTile[nX][nY] = (((uint32_t)(nFileIndex & 0X000000FF)) << 16) + (((uint32_t)(nImageIndex)) & 0X0000FFFF);
                    m_BufTileMark[nX][nY] = 1;
                }
            }
        }
    }
}

void Mir2Map::SetBufGround(int nX, int nY, int nIndex)
{
    if(m_Mir2xMap.Valid()){
        // mir2x map
        if(m_Mir2xMap.GroundValid(nX, nY, nIndex)){
            m_BufGroundMark[nX][nY][nIndex] = 1;
            m_BufGround[nX][nY][nIndex] = m_Mir2xMap.Ground(nX, nY, nIndex);
        }
    }else if(m_Valid){
        // mir2 map
        if(CellInfo(nX, nY).bFlag & 0X01){
            m_BufGroundMark[nX][nY][nIndex] = 1;
            m_BufGround[nX][nY][nIndex] = 0X0000; // set by myselt
        }
    }

}

void Mir2Map::SetBufObj(int nX, int nY, int nIndex)
{
    if(m_Mir2xMap.Valid()){
        // mir2x map
        if(m_Mir2xMap.ObjectValid(nX, nY, nIndex)){
            if(m_Mir2xMap.GroundObjectValid(nX, nY, nIndex)){
                m_BufObjMark[nX][nY][nIndex] = 3;
            }else{
                m_BufObjMark[nX][nY][nIndex] = 1;
            }

            m_BufObj[nX][nY][nIndex] = m_Mir2xMap.ObjectDesc(nX, nY, nIndex);
        }
    }else if(m_Valid){
        // mirx map
        int nFileIndex  = 0;
        int nImageIndex = 0;
        uint8_t nAttr   = 0;

        if(nIndex == 0){
            nFileIndex  = ((m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8);
            nImageIndex = m_pstCellInfo[nArrayNum].wObj1;
            nAttr       = m_pstCellInfo[nArrayNum].bObj1Ani;
        }else{
            nFileIndex  = (m_pstCellInfo[nArrayNum].wFileIndex & 0X00FF);
            nImageIndex = m_pstCellInfo[nArrayNum].wObj2;
            nAttr       = m_pstCellInfo[nArrayNum].bObj2Ani;
        }

        if(nFileIndex != 255 && nImageIndex != 65535){
            // ignore door information
            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex)
                    && m_pxTileImage[nFileIndex].CurrentImageValid()){
                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                if(nW * nH != 0){
                    if(nW == 48 && nH == 32){
                        m_BufObjMark[nX][nY][nIndex] = 3;
                    }else{
                        m_BufObjMark[nX][nY][nIndex] = 1;
                    }
                }
            }
        }
    }
}

void Mir2Map::SetBufLight(int nX, int nY)
{
    if(m_Mir2xMap.Valid()){
        // mir2x map
        if(m_Mir2xMap.LightValid(nX, nY)){
            m_BufLight[nX][nY]     = m_Mir2xMap.Light(nX, nY);
            m_BufLightMark[nX][nY] = 1;
        }
    }else if(m_Valid){
        // mir2 map
        int nArrayNum = nY + nX * m_stMapFileHeader.shHeight;
        if(m_pstCellInfo[nArrayNum].wLightNEvent != 0
                || (m_pstCellInfo[nArrayNum].wLightNEvent & 0X0007) == 1){
            // make light frog by myself
            uint16_t nColorIndex = 128;  // 0, 1, 2, 3, ..., 15  4 bits
            uint16_t nAlphaIndex =   2;  // 0, 1, 2, 3           2 bits
            uint16_t nSizeType   =   0;  // 0, 1, 2, 3, ...,  7  3 bits
            uint16_t nUnused     =   0;  //                      7 bits

            m_BufLight[nX][nY] = ((nSizeType & 0X0007) << 7) + ((nAlphaIndex & 0X0003) << 4) + ((nColorIndex & 0X000F));
            m_BufLightMark[nX][nY] = 1;
        }
    }
}
