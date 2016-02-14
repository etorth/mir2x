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

EditorMap::EditorMap()
    : m_Valid(false)
    , m_OldMir2Map(nullptr)
    , m_Mir2xMap(nullptr)
    , m_SelectedGrid()
{
    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));
    std::memset(m_bAniTileFrame, 0, sizeof(uint8_t) * 8 * 16);
    std::memset(m_dwAniSaveTime, 0, sizeof(uint32_t) * 8);
}

EditorMap::~EditorMap()
{
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;
}

void EditorMap::LoadMapImage(WilImagePackage *pWilImagePackage)
{
    m_pxTileImage = pWilImagePackage;
}

bool EditorMap::LoadMap(const char *szMapFileName)
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
            W(), std::vector<std::array<bool, 4>>(
                H(), {false, false, false, false}));

    return m_Valid;
}

void EditorMap::ExtractOneBaseTile(
            std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc,
            int nXCnt, int nYCnt)
{
    if(!Valid() || !ValidC(nXCnt, nYCnt) || (nXCnt % 2) || (nYCnt % 2)){ return; }

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // decode with new map structure
        if(!m_Mir2xMap->TileValid(nXCnt, nYCnt)){ return; }

        // no alpha blend for tile 
        uint32_t nKey   = m_Mir2xMap->TileKey(nXCnt, nYCnt);
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

void EditorMap::ExtractBaseTile(std::function<bool(uint32_t, uint32_t)> fnCheckExistFunc,
        std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc)
{
    if(!Valid()){ return; }

    int nFileIndex  = 0;
    int nImageIndex = 0;

    uint32_t *pBuff    = nullptr;
    int       nBuffLen = 0;

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        for(int nXCnt = 0; nXCnt < m_Mir2xMap->W(); nXCnt++){
            for(int nYCnt = 0; nYCnt < m_Mir2xMap->H(); ++nYCnt){
                if(!(nXCnt % 2) && !(nYCnt % 2)){
                    if(!m_Mir2xMap->TileValid(nXCnt, nYCnt)){ continue; }

                    // no alpha blend for tile 
                    uint32_t nKey   = m_Mir2xMap->TileKey(nXCnt, nYCnt);
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

int EditorMap::W()
{
    return (int)(m_stMapFileHeader.shWidth);
}

int EditorMap::H()
{
    return (int)(m_stMapFileHeader.shHeight);
}

const CELLINFO &EditorMap::CellInfo(int nX, int nY)
{
    return m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight];
}

const TILEINFO &EditorMap::BaseTileInfo(int nX, int nY)
{
    return m_pstTileInfo[(nX / 2) * m_stMapFileHeader.shHeight / 2 + nY / 2];
}

uint32_t EditorMap::GetDoorImageIndex(int nX, int nY)
{
    uint32_t nDoorIndex = 0;
    if(Valid()){
        if(m_Mir2xMap && m_Mir2xMap->Valid()){
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

void EditorMap::DrawBaseTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<void(uint32_t, uint32_t, int, int)> fnDrawTileFunc)
{
    if(!Valid()){ return; }

    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, W() - 1);
    nStopCellY  = (std::min)(nStopCellY, H() - 1);

    for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
        for(int nX = nStartCellX; nX <= nStopCellX; ++nX){
            if(nX % 2 || nY % 2){
                continue;
            }

            if(m_Mir2xMap && m_Mir2xMap->Valid()){
                if(!m_Mir2xMap->TileValid(nX, nY)){ continue; }
                uint32_t nKey = m_Mir2xMap->TileKey(nX, nY);
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

void EditorMap::ExtractOneObjectTile(std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc, int nXCnt, int nYCnt)
{
    if(!Valid() || !ValidC(nX, nY)){ return; }

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nIndex = 0; nIndex < 2; ++nIndex){
            if(m_Mir2xMap->ObjectValid(nX, nY, nIndex)){
                uint32_t nKey = m_Mir2xMap->ObjectBaseKey(nX, nY, nIndex);

                int nImageDesc  = ((nKey & 0XFF000000) >> 24);
                int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                int nImageIndex = ((nKey & 0X0000FFFF));
                int nFrameCount = m_Mir2xMap->ObjectFrameCount(nX, nY, nIndex);

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

void EditorMap::ExtractObjectTile(std::function<bool(uint32_t, uint32_t)> fnCheckExistFunc,
        std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc)
{
    if(!Valid()){ return; }

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nYCnt = 0; nYCnt < m_Mir2xMap->H(); ++nYCnt){
            for(int nXCnt = 0; nXCnt < m_Mir2xMap->W(); ++nXCnt){
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    if(m_Mir2xMap->ObjectValid(nX, nY, nIndex)){
                        uint32_t nKey = m_Mir2xMap->ObjectBaseKey(nX, nY, nIndex);

                        int nImageDesc  = ((nKey & 0XFF000000) >> 24);
                        int nFileIndex  = ((nKey & 0X00FF0000) >> 16);
                        int nImageIndex = ((nKey & 0X0000FFFF));
                        int nFrameCount = m_Mir2xMap->ObjectFrameCount(nX, nY, nIndex);

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

void EditorMap::DrawObjectTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<bool(uint32_t, uint32_t, Fl_Shared_Image * &, int, int, int)> fnCheckFunc,
        std::function<void(uint32_t, uint32_t, Fl_Shared_Image *, int, int, int)> fnDrawObjFunc)
{
    if(!Valid()){ return; }

    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, W() - 1);
    nStopCellY  = (std::min)(nStopCellY, H() - 1);

    for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
        for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(m_Mir2xMap->ObjectValid(nXCnt, nYCnt, nIndex)){
                    uint32_t nKey = m_Mir2xMap->ObjectKey(nXCnt, nYCnt, nIndex);
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
}

void EditorMap::UpdateFrame(int nLoopTime)
{
    // m_bAniTileFrame[i][j]:
    //     i: denotes how fast the animation is.
    //     j: denotes how many frames the animation has.

    if(!Valid()){ return; }

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

bool EditorMap::Resize(
        int nX, int nY, int nW, int nH, // define a region on original map
        int nNewX, int nNewY,           // where the cropped region start on new map
        int nNewW, int nNewH)           // size of new map
{
    // we only support 2M * 2N cropping and expansion
    if(!Valid()){ return false; }

    nX = (std::max)(0, nX);
    nY = (std::max)(0, nY);

    if(nX % 2){ nX--; nW++; }
    if(nY % 2){ nY--; nH++; }
    if(nW % 2){ nW++; }
    if(nH % 2){ nH++; }

    nW = (std::min)(nW, W() - nX);
    nH = (std::min)(nH, H() - nY);

    if(true
            && nX == 0
            && nY == 0
            && nW == W()
            && nH == H()
            && nNewX == 0
            && nNewY == 0
            && nNewW == nW
            && nNewH == nH)
    {
        // no need to do anything
        return true;
    }

    // here (nX, nY, nW, nH) has been a valid subsection of original map
    // could be null
    //
    // invalid new map defination
    if(nNewW <= 0 || nNewH <= 0){
        return false;
    }

    // start for new memory allocation
    auto stOldBufLight         = m_BufLight;
    auto stOldBufLightMark     = m_BufLightMark;
    auto stOldBufTile          = m_BufTile;
    auto stOldBufTileMark      = m_BufTileMark;
    auto stOldBufObj           = m_BufObj;
    auto stOldBufObjMark       = m_BufObjMark;
    auto stOldBufGroundObjMark = m_BufGroundObjMark;
    auto stOldBufGround        = m_BufGround;
    auto stOldBufGroundMark    = m_BufGroundMark;

    // this function will clear the new buffer
    // with all zeros
    MakeBuf(nNewW, nNewH);

    for(int nTY = 0; nTY < nH; ++nTY){
        for(int nTX = 0; nTX < nW; ++nTX){
            int nSrcX = nTX + nX;
            int nSrcY = nTY + nY;
            int nDstX = nTX + nNewX;
            int nDstY = nTY + nNewY;

            if(nDstX >= 0 && nDstX < nNewW && nDstY >= 0 && nDstY < nNewH){
                if(!(nDstX % 2) && !(nDstY % 2) && !(nSrcX % 2) && !(nSrcY % 2)){
                    stOldBufTile    [nDstX / 2][nDstY / 2] = m_BufTile    [nDstX / 2][nDstY / 2];
                    stOldBufTileMark[nDstX / 2][nDstY / 2] = m_BufTileMark[nDstX / 2][nDstY / 2];
                }

                stOldBufLight          [nDstX / 2][nDstY / 2]    = m_BufLight          [nDstX / 2][nDstY / 2]   ;
                stOldBufLightMark      [nDstX / 2][nDstY / 2]    = m_BufLightMark      [nDstX / 2][nDstY / 2]   ;

                stOldBufObj            [nDstX / 2][nDstY / 2][0] = m_BufObj            [nDstX / 2][nDstY / 2][0];
                stOldBufObj            [nDstX / 2][nDstY / 2][1] = m_BufObj            [nDstX / 2][nDstY / 2][1];

                stOldBufObjMark        [nDstX / 2][nDstY / 2][0] = m_BufObjMark        [nDstX / 2][nDstY / 2][0];
                stOldBufObjMark        [nDstX / 2][nDstY / 2][1] = m_BufObjMark        [nDstX / 2][nDstY / 2][1];

                stOldBufGroundObjMark  [nDstX / 2][nDstY / 2][0] = m_BufGroundObjMark  [nDstX / 2][nDstY / 2][0];
                stOldBufGroundObjMark  [nDstX / 2][nDstY / 2][1] = m_BufGroundObjMark  [nDstX / 2][nDstY / 2][1];

                stOldBufGround         [nDstX / 2][nDstY / 2][0] = m_BufGround         [nDstX / 2][nDstY / 2][0];
                stOldBufGround         [nDstX / 2][nDstY / 2][1] = m_BufGround         [nDstX / 2][nDstY / 2][1];
                stOldBufGround         [nDstX / 2][nDstY / 2][2] = m_BufGround         [nDstX / 2][nDstY / 2][2];
                stOldBufGround         [nDstX / 2][nDstY / 2][3] = m_BufGround         [nDstX / 2][nDstY / 2][3];

                stOldBufGroundMark     [nDstX / 2][nDstY / 2][0] = m_BufGroundMark     [nDstX / 2][nDstY / 2][0];
                stOldBufGroundMark     [nDstX / 2][nDstY / 2][1] = m_BufGroundMark     [nDstX / 2][nDstY / 2][1];
                stOldBufGroundMark     [nDstX / 2][nDstY / 2][2] = m_BufGroundMark     [nDstX / 2][nDstY / 2][2];
                stOldBufGroundMark     [nDstX / 2][nDstY / 2][3] = m_BufGroundMark     [nDstX / 2][nDstY / 2][3];

            }
        }
    }

    std::swap(stOldBufLight        , m_BufLight        );
    std::swap(stOldBufLightMark    , m_BufLightMark    );
    std::swap(stOldBufTile         , m_BufTile         );
    std::swap(stOldBufTileMark     , m_BufTileMark     );
    std::swap(stOldBufObj          , m_BufObj          );
    std::swap(stOldBufObjMark      , m_BufObjMark      );
    std::swap(stOldBufGroundObjMark, m_BufGroundObjMark);
    std::swap(stOldBufGround       , m_BufGround       );
    std::swap(stOldBufGroundMark   , m_BufGroundMark   );

    return true;
}

bool EditorMap::Valid()
{
    return m_Valid;
}

bool EditorMap::ValidC(int nX, int nY)
{
    // assume valid map
    return true
        && nX >= 0
        && nX <  W()
        && nY >= 0
        && nY <  H();
}

bool EditorMap::ValidP(int nX, int nY)
{
    // assume valid map
    return true
        && nX >= 0
        && nX <  48 * W()
        && nY >= 0
        && nY <  32 * H();
}

bool EditorMap::!GroundValid(int nStartX, int nStartY, int nIndex)
{
}

uint8_t EditorMap::Ground(int nStartX, int nStartY, int nIndex)
{
    return m_BufGround[nStartX][nStartY][nIndex];
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
int EditorMap::GroundBlockType(int nStartX, int nStartY, int nIndex, int nSize)
{
    // assume valid map, valid parameters
    if(nSize == 0){
        return GroundValid(nStartX, nStartY, nIndex) ? 1 : 0;
    }else{
        bool bFindEmpty = false;
        bool bFindFill  = false;
        bool bFindDiff  = false;

        bool bInited = false;
        uint8_t nGroundInfoSample = 0;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(GroundValid(nX, nY, nIndex)){
                        bFindFill = true;
                        if(bInited){
                            if(nGroundInfoSample != Ground(nX, nY, nIndex)){
                                bFindDiff = true;
                            }
                        }else{
                            nGroundInfoSample = Ground(nX, nY, nIndex);
                        }
                    }else{
                        bFindEmpty = true;
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
void EditorMap::DoCompressGround(int nX, int nY, int nSize,
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
                        // equivlent to GroundValid()
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
                DoCompressGround(nX            , nY            , nSize / 2, stMarkV, stDataV);
                DoCompressGround(nX + nSize / 2, nY            , nSize / 2, stMarkV, stDataV);
                DoCompressGround(nX            , nY + nSize / 2, nSize / 2, stMarkV, stDataV);
                DoCompressGround(nX + nSize / 2, nY + nSize / 2, nSize / 2, stMarkV, stDataV);
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

void EditorMap::RecordGround(std::vector<uint8_t> &stDataV, int nX, int nY, int nIndex)
{
    stDataV.push_back(Ground(nX, nY, nIndex));
}

void EditorMap::RecordLight(std::vector<uint8_t> &stDataV, int nX, int nY)
{
    stDataV.push_back((Light(nX, nY) & 0X00FF) >> 0);
    stDataV.push_back((Light(nX, nY) & 0XFF00) >> 8);
}

void EditorMap::RecordObject(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV, int nX, int nY, int nIndex)
{
    bool bGroundObj = GroundObjectValid(nX, nY, nIndex);
    uint32_t nObjDesc = ObjectDesc(nX, nY, nIndex);

    stMarkV.push_back(bGroundObj);

    stDataV.push_back((uint8_t)((nObjDesc & 0X000000FF ) >>  0));
    stDataV.push_back((uint8_t)((nObjDesc & 0X0000FF00 ) >>  8));
    stDataV.push_back((uint8_t)((nObjDesc & 0X00FF0000 ) >> 16));
    stDataV.push_back((uint8_t)((nObjDesc & 0XFF000000 ) >> 24));
}

void EditorMap::RecordTile(std::vector<uint8_t> &stDataV, int nX, int nY)
{
    uint32_t nTileDesc = TileDesc(nX, nY, nIndex);

    stDataV.push_back((uint8_t)((nTileDesc & 0X000000FF ) >>  0));
    stDataV.push_back((uint8_t)((nTileDesc & 0X0000FF00 ) >>  8));
    stDataV.push_back((uint8_t)((nTileDesc & 0X00FF0000 ) >> 16));
    stDataV.push_back((uint8_t)((nTileDesc & 0XFF000000 ) >> 24));
}

// Light is simplest one
void EditorMap::DoCompressLight(int nX, int nY, int nSize,
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
                DoCompressLight(nX            , nY            , nSize / 2, stMarkV, stDataV);
                DoCompressLight(nX + nSize / 2, nY            , nSize / 2, stMarkV, stDataV);
                DoCompressLight(nX            , nY + nSize / 2, nSize / 2, stMarkV, stDataV);
                DoCompressLight(nX + nSize / 2, nY + nSize / 2, nSize / 2, stMarkV, stDataV);
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
void EditorMap::DoCompressTile(int nX, int nY, int nSize,
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
                DoCompressObject(nX            , nY            , nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX + nSize / 2, nY            , nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX            , nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX + nSize / 2, nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
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
void EditorMap::DoCompressObject(int nX, int nY, int nIndex, int nSize,
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
                DoCompressObject(nX            , nY            , nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX + nSize / 2, nY            , nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX            , nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
                DoCompressObject(nX + nSize / 2, nY + nSize / 2, nIndex, nSize / 2, stMarkV, stDataV);
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

void EditorMap::CompressLight(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < H(); nY += 8){
        for(int nX = 0; nX < W(); nX += 8){
            DoCompressLight(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

void EditorMap::CompressGround(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < H(); nY += 8){
        for(int nX = 0; nX < W(); nX += 8){
            DoCompressGround(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

void EditorMap::CompressTile(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < H(); nY += 8){
        for(int nX = 0; nX < W(); nX += 8){
            DoCompressTile(nX, nY, 8, stMarkV, stDataV);
        }
    }
}

bool EditorMap::LoadMap2xMap(const char *szFullName)
{
    delete m_OldMir2Map; m_OldMir2Map = nullptr;
    delete m_Mir2xMap  ; m_Mir2xMap   = new Mir2xMap();

    if(m_Mir2xMap->Load(szFullName)){
        MakeBuf();
    }

    delete m_Mir2xMap;
    return Valid();
}

bool EditorMap::LoadMir2Map(const char *szFullName)
{
    delete m_OldMir2Map; m_OldMir2Map = new Mir2Map();
    delete m_Mir2xMap  ; m_Mir2xMap   = nullptr;

    if(m_OldMir2Map->Load(szFullName)){
        MakeBuf();
    }

    delete m_OldMir2Map;
    return Valid();
}

void EditorMap::Optimize()
{
    if(!Valid()){ return; }

    // try to remove some unnecessary tile/cell
    // tile
    for(int nY = 0; nY < H(); ++nY){
        for(int nX = 0; nX < W(); ++nX){
            OptimizeBaseTile(nX, nY);
            OptimizeCell(nX, nY);
        }
    }
}

void EditorMap::OptimizeBaseTile(int nX, int nY)
{
    // TODO
}

void EditorMap::OptimizeCell(int, int)
{
    // TODO
}

void EditorMap::ClearBuf()
{
    m_W = 0;
    m_H = 0;
    m_BufValid = false;
}

bool EditorMap::InitBuf(int nW, int nH)
{
    if(nW == 0 || nH == 0 || nW % 2 || nH % 2){ return false; }

    // m_BufLight[nX][nY]
    m_BufLight = std::vector<std::vector<uint16_t>>(nW, std::vector<uint16_t>(nH, 0));
    // m_BufLightMark[nX][nY]
    m_BufLightMark = std::vector<std::vector<int>>(nW, std::vector<uint32_t>(nH, 0));

    // m_BufTile[nX][nY]
    m_BufTile = std::vector<std::vector<uint32_t>>(nW / 2, std::vector<uint32_t>(nH / 2, 0));
    // m_BufTileMark[nX][nY]
    m_BufTileMark = std::vector<std::vector<int>>(nW / 2, std::vector<uint32_t>(nH / 2, 0));

    // m_BufObj[nX][nY][0]
    m_BufObj = std::vector<std::vector<std::array<uint32_t, 2>>>(
            nW, std::vector<std::array<uint32_t, 2>>(nH, {0, 0}));
    // m_BufObjMark[nX][nY][0]
    m_BufObjMark = std::vector<std::vector<std::array<int, 2>>>(
            nW, std::vector<std::array<int, 2>>(nH, {0, 0}));
    // m_BufGroundObjMark[nX][nY][0]
    m_BufGroundObjMark = std::vector<std::vector<std::array<int, 2>>>(
            nW, std::vector<std::array<int, 2>>(nH, {0, 0}));

    // m_BufGround[nX][nY][0]
    m_BufGround = std::vector<std::vector<std::array<uint8_t, 4>>>(
            nW, std::vector<std::array<uint8_t, 4>>(nH, {0, 0, 0, 0}));
    // m_BufGroundMark[nX][nY][0]
    m_BufGroundMark = std::vector<std::vector<std::array<int, 4>>>(
            nW, std::vector<std::array<int, 4>>(nH, {0, 0, 0, 0}));

    return true;
}

bool EditorMap::MakeBuf()
{
    // after load a map by mir2x map or old mir2 map
    // always make a buffer of it, then we can edit on this buffer
    //
    // all queries and operations are on this buffer
    // fixed format maps are only for one-shoot usage
    //
    // which simplifies outside funtion handle design

    ClearBuf();

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        m_W = m_Mir2xMap->W();
        m_H = m_Mir2xMap->H();
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        m_W = m_OldMir2Map->W();
        m_H = m_OldMir2Map->H();
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

void EditorMap:SetBufTile(int nX, int nY)
{
    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // mir2x map
        if(m_Mir2xMap->TileValid(nX, nY)){
            m_BufTile[nX][nY] = m_Mir2xMap->Tile(nX, nY);
            m_BufTileMark[nX][nY] = 1;
        }
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        if(m_OldMir2Map->TileValid(nX, nY)){
            m_BufTile[nX][nY] = m_OldMir2Map->Tile(nX, nY);
            m_BufTileMark[nX][nY] = 1;
        }
    }
}

void EditorMap::SetBufGround(int nX, int nY, int nIndex)
{
    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // mir2x map
        if(m_Mir2xMap->GroundValid(nX, nY, nIndex)){
            m_BufGroundMark[nX][nY][nIndex] = 1;
            m_BufGround[nX][nY][nIndex] = m_Mir2xMap->Ground(nX, nY, nIndex);
        }
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        // mir2 map
        if(m_OldMir2Map->GroundValid(nX, nY)){
            m_BufGroundMark[nX][nY][nIndex] = 1;
            m_BufGround[nX][nY][nIndex] = 0X0000; // set by myselt
        }
    }
}

void EditorMap::SetBufObj(int nX, int nY, int nIndex)
{
    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // mir2x map
        if(m_Mir2xMap->ObjectValid(nX, nY, nIndex)){
            if(m_Mir2xMap->GroundObjectValid(nX, nY, nIndex)){
                m_BufObjMark[nX][nY][nIndex] = 3;
            }else{
                m_BufObjMark[nX][nY][nIndex] = 1;
            }

            m_BufObj[nX][nY][nIndex] = m_Mir2xMap->Object(nX, nY, nIndex);
        }
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        // mirx map
        if(m_OldMir2Map->ObjectValid(nX, nY, nIndex)){
            m_BufObj[nX][nY][nIndex] = m_OldMir2Map->Object(nX, nY, nIndex);
            if(m_OldMir2Map->GroundObjectValid(nX, nY, nIndex)){
                m_BufObjMark[nX][nY][nIndex] = 3;
            }else{
                m_BufObjMark[nX][nY][nIndex] = 1;
            }
        }
    }
}

void EditorMap::SetBufLight(int nX, int nY)
{
    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // mir2x map
        if(m_Mir2xMap->LightValid(nX, nY)){
            m_BufLight[nX][nY]     = m_Mir2xMap->Light(nX, nY);
            m_BufLightMark[nX][nY] = 1;
        }
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        // mir2 map
        if(m_OldMir2Map->LightValid(nX, nY)){
            // we deprecate it
            // uint16_t nLight = m_OldMir2Map->Light(nX, nY);
            //
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
