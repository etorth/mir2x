#include "mir2map.hpp"
#include "wilimagepackage.hpp"
#include <memory.h>
#include "assert.h"
#include <cstring>
#include "misc.hpp"
#include <functional>
#include <cstdint>
#include <algorithm>
#include <vector>

Mir2Map::Mir2Map()
    : m_Valid(false)
    , m_pstTileInfo(nullptr)
    , m_pstCellInfo(nullptr)
    , m_BaseTileInfo(nullptr)
    , m_GroundInfo(nullptr)
    , m_CellDesc(nullptr)
{
    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));
    std::memset(m_bAniTileFrame, 0, sizeof(uint8_t) * 8 * 16);
    std::memset(m_dwAniSaveTime, 0, sizeof(uint32_t) * 8);
}

Mir2Map::~Mir2Map()
{
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;
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
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;

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

    // expand to 8 * 8
	int nNewW = 0;
	if(m_stMapFileHeader.shWidth % 8){
        nNewW = (1 + m_stMapFileHeader.shWidth / 8) * 8;
    }else{
        nNewW = m_stMapFileHeader.shWidth;
    }

	int nNewH = 0;
	if(m_stMapFileHeader.shHeight % 8){
        nNewH = (1 + m_stMapFileHeader.shHeight / 8) * 8;
    }else{
        nNewH = m_stMapFileHeader.shHeight;
    }
        
    if((m_stMapFileHeader.shWidth != nNewW) || (m_stMapFileHeader.shHeight != nNewH)){
        if(Expand(nNewW, nNewH)){
            m_Valid = true;
        }
	}else{
		m_Valid = true;
	}

    return m_Valid;
}

bool Mir2Map::Expand(int nNewW, int nNewH)
{
    if(m_pstTileInfo && m_pstCellInfo){
        int  nMapLoadSize = nNewW * nNewH;
        auto pNewTileInfo = new TILEINFO[nMapLoadSize / 4];
        auto pNewCellInfo = new CELLINFO[nMapLoadSize];

        for(int nX = 0; nX < nNewW; ++nX){
            for(int nY = 0; nY < nNewH; ++nY){
                if(nX < m_stMapFileHeader.shWidth && nY < m_stMapFileHeader.shHeight){
                    pNewTileInfo[nY / 2 + (nX / 2) * nNewH / 2] = 
                        m_pstTileInfo[(nY / 2) + (nX / 2) * m_stMapFileHeader.shHeight / 2];
                    pNewCellInfo[nY + nX * nNewH] = 
                        m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight];
                }else{
                    pNewTileInfo[nY / 2 + (nX / 2) * nNewH / 2] = {255, 65535};
                    pNewCellInfo[nY + nX * nNewH] = {
                        0, 255, 255, 65535, 65535, 65535, 0, 0, 0};
                }
            }
        }

        delete [] m_pstTileInfo;
        delete [] m_pstCellInfo;

        m_pstTileInfo = pNewTileInfo;
        m_pstCellInfo = pNewCellInfo;

        m_stMapFileHeader.shWidth  = nNewW;
        m_stMapFileHeader.shHeight = nNewH;

        return true;
    }
    return false;
}

void Mir2Map::ExtractOneBaseTile(
            std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc,
            int nXCnt, int nYCnt)
{
    if(!m_Valid){ return; }

    if(m_BaseTileInfo){
        uint32_t *pBuff = nullptr;

        if(!(nXCnt % 2) && !(nYCnt % 2)){
            uint32_t nArrayNum     = (nXCnt / 2) + (nYCnt / 2) * (m_stMapFileHeader.shWidth / 2);
            uint32_t nBaseTileInfo = m_BaseTileInfo[nArrayNum];
            if(nBaseTileInfo == 0XFFFFFFFF){
                return;
            }

            // 6-10-16
            // 0XFC000000
            // 0X03FF0000
            // 0X0000FFFF
            int nFileIndex  = ((nBaseTileInfo & 0X03FF0000) >> 16);
            int nImageIndex = ((nBaseTileInfo & 0X0000FFFF));

            // redu\t in base tile function
            // just put it here
            if(nFileIndex == 255 || nImageIndex == 65535){
                return;
            }

            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                    m_pxTileImage[nFileIndex].CurrentImageValid()){

                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                if(nW * nH > 0){
                    pBuff = new uint32_t[nW * nH];
                }else{
                    return;
                }
                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
            }
        }
        delete pBuff;
        return;
    }

    if(m_pstTileInfo){
        uint32_t *pBuff = nullptr;

        if(!(nXCnt % 2) && !(nYCnt % 2)){
            int nFileIndex  = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].bFileIndex;
            int nImageIndex = m_pstTileInfo[(nYCnt / 2) + (nXCnt / 2)*m_stMapFileHeader.shHeight / 2].wTileIndex;

            if(nFileIndex != 255 && nImageIndex != 65535) {
                if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                        m_pxTileImage[nFileIndex].CurrentImageValid()){

                    int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                    int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                    if(nW * nH > 0){
                        pBuff = new uint32_t[nW * nH];
                    }else{
                        return;
                    }
                    m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                    fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                }
            }
        }
        delete pBuff;
        return;
    }

}

void Mir2Map::ExtractBaseTile(std::function<bool(uint32_t, uint32_t)> fnCheckExistFunc,
        std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)> fnWritePNGFunc)
{
    if(!m_Valid){ return; }

    if(m_BaseTileInfo){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; nXCnt++){
            for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){
                if(!(nXCnt % 2) && !(nYCnt % 2)){

                    uint32_t nArrayNum     = (nXCnt / 2) + (nYCnt / 2) * (m_stMapFileHeader.shWidth / 2);
                    uint32_t nBaseTileInfo = m_BaseTileInfo[nArrayNum];
                    if(nBaseTileInfo == 0XFFFFFFFF){
                        continue;
                    }

                    int nFileIndex  = (nBaseTileInfo & 0X03FF0000) >> 16;
                    int nImageIndex = (nBaseTileInfo & 0X0000FFFF);

                    if(nFileIndex != 255 && nImageIndex != 65535) {
                        if(fnCheckExistFunc(nFileIndex, nImageIndex)){
                            continue;
                        }
                        if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex) &&
                                m_pxTileImage[nFileIndex].CurrentImageValid()){

                            int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                            int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                            if(nW * nH > nBuffLen){
                                delete pBuff;
                                pBuff    = new uint32_t[nW * nH];
                                nBuffLen = nW * nH;
                            }
                            m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                            fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
        delete pBuff;
        return;
    }

    if(m_pstTileInfo){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

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
                            if(nW * nH > nBuffLen){
                                delete pBuff;
                                pBuff    = new uint32_t[nW * nH];
                                nBuffLen = nW * nH;
                            }
                            m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                            fnWritePNGFunc(pBuff, nFileIndex, nImageIndex, nW, nH, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
        delete pBuff;
        return;
    }
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

bool Mir2Map::Valid()
{
    return m_Valid;
}

uint32_t Mir2Map::GetDoorImageIndex(int nX, int nY)
{
    uint32_t nDoorIndex = 0;

	if (m_CellDesc){
		auto stCellDesc = m_CellDesc[nX + nY * m_stMapFileHeader.shWidth];
		if((stCellDesc.dwDesc & 0XFF000000) > 0){
			nDoorIndex += ((stCellDesc.dwDesc & 0X007F0000) >> 16);
			printf("be careful: GetDoorImageIndex() returns non-zero value !!!\n");
		}
		return nDoorIndex;
	}

    if(m_pstCellInfo){
        // seems bDoorOffset & 0X80 shows open or close
        //       bDoorIndex  & 0X80 shows whether there is a door
        if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shWidth].bDoorOffset & 0X80) > 0){
            if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shWidth].bDoorIndex & 0X7F) > 0){
                nDoorIndex += m_pstCellInfo[nY + nX * m_stMapFileHeader.shWidth].bDoorOffset & 0X7F;
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
    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, m_stMapFileHeader.shWidth - 1);
    nStopCellY  = (std::min)(nStopCellY, m_stMapFileHeader.shHeight - 1);

    for(int nY = nStartCellY; nY <= nStopCellY; ++nY){
        for(int nX = nStartCellX; nX <= nStopCellX; ++nX){
            if(nX % 2 || nY % 2){
                continue;
            }

            if(m_BaseTileInfo){
                uint32_t nArrayNum = (nX / 2) + (nY / 2) * (m_stMapFileHeader.shWidth / 2);
                uint32_t nBaseTileInfo = m_BaseTileInfo[nArrayNum];
                if(nBaseTileInfo == 0XFFFFFFFF){
                    continue;
                }

                int nFileIndex  = (nBaseTileInfo & 0X03FF0000) >> 16;
                int nImageIndex = (nBaseTileInfo & 0X0000FFFF);

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
    if(!m_Valid){ return; }

    if(m_CellDesc){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        int  nArrayNum  = nXCnt + nYCnt * m_stMapFileHeader.shWidth;
        auto stCellDesc = m_CellDesc[nArrayNum];


        if(true
                && stCellDesc.dwDesc    == 0XFFFFFFFF
                && stCellDesc.dwObject1 == 0XFFFFFFFF
                && stCellDesc.dwObject2 == 0XFFFFFFFF
                && stCellDesc.dwLight   == 0XFFFFFFFF
          ){
            return;
        }

        {// first layer:
            uint32_t nFileIndex  = (((stCellDesc.dwObject1 & 0X03FF0000) >> 16));
            uint32_t nImageIndex = (((stCellDesc.dwObject1 & 0X0000FFFF)));
            uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X000000FF)));

            if(nFileIndex != 255 && nImageIndex != 65535){
                nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                uint32_t nFrameCount = 0;

                if(nObjectDesc != 255){
                    // here we don't use bTickType when decode PNG's
                    // uint16_t bTickType = (nObjectDesc & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (nObjectDesc & 0X0F);      // for frame count

                    nFrameCount += shAniCnt;
                }

                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                            && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                        fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                    }
                }
            }
        }

        {// second
            uint32_t nFileIndex  = (((stCellDesc.dwObject2 & 0X03FF0000) >> 16));
            uint32_t nImageIndex = (((stCellDesc.dwObject2 & 0X0000FFFF)));
            uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X0000FF00) >>  8));

            if(nFileIndex != 255 && nImageIndex != 65535){
                nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                uint32_t nFrameCount = 0;

                if(nObjectDesc != 255){
                    // didn't use bTickType when decoding PNG's
                    // uint16_t bTickType = (nObjectDesc & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (nObjectDesc & 0X0F);      // for frame count

                    nFrameCount += shAniCnt;
                }

                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                            && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                        fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                    }
                }
            }
        }
        delete pBuff;
		return;
    }

    if(m_pstCellInfo){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;

        {// first layer:
            int nFileIndex = (m_pstCellInfo[nArrayNum].wFileIndex & 0XFF00) >> 8;
            if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                uint32_t nImageIndex = m_pstCellInfo[nArrayNum].wObj1 + GetDoorImageIndex(nXCnt, nYCnt);
                uint32_t nFrameCount = 0;

                if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                    // didn't use bTickType for decoding
                    // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F);        // for frame count

                    nFrameCount += shAniCnt;
                }

                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
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

                if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                    // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4; // for ticks
                    uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F);        // for frame count

                    nFrameCount += shAniCnt;
                }

                for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                    if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                        int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                        int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                        if(nBuffLen < nW * nH){
                            pBuff    = new uint32_t[nW * nH];
                            nBuffLen = nW * nH;
                        }
                        m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
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
    if(!m_Valid){ return; }

    if(m_CellDesc){
        uint32_t *pBuff    = nullptr;
        int       nBuffLen = 0;

        for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){
            for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; ++nXCnt){

                int  nArrayNum  = nXCnt + nYCnt * m_stMapFileHeader.shWidth;
                auto stCellDesc = m_CellDesc[nArrayNum];

                if(true
                        && stCellDesc.dwDesc    == 0XFFFFFFFF
                        && stCellDesc.dwObject1 == 0XFFFFFFFF
                        && stCellDesc.dwObject2 == 0XFFFFFFFF
                        && stCellDesc.dwLight   == 0XFFFFFFFF
                  ){
                    continue;
                }

                {// first layer:
                    uint32_t nFileIndex  = (((stCellDesc.dwObject1 & 0X03FF0000) >> 16));
                    uint32_t nImageIndex = (((stCellDesc.dwObject1 & 0X0000FFFF)));
                    uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X000000FF)));

                    if(nFileIndex != 255 && nImageIndex != 65535){
                        nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                        uint32_t nFrameCount = 0;

                        if(nObjectDesc != 255){
                            // uint16_t bTickType = (nObjectDesc & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (nObjectDesc & 0X0F);      // for frame count

                            nFrameCount += shAniCnt;
                        }

                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                                    && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                            }
                        }
                    }
                }

                {// second
                    uint32_t nFileIndex  = (((stCellDesc.dwObject2 & 0X03FF0000) >> 16));
                    uint32_t nImageIndex = (((stCellDesc.dwObject2 & 0X0000FFFF)));
                    uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X0000FF00) >>  8));

                    if(nFileIndex != 255 && nImageIndex != 65535){
                        nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                        uint32_t nFrameCount = 0;

                        if(nObjectDesc != 255){
                            // uint16_t bTickType = (nObjectDesc & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (nObjectDesc & 0X0F);      // for frame count

                            nFrameCount += shAniCnt;
                        }

                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame)
                                    && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
                                fnWritePNGFunc(pBuff, nFileIndex, nImageIndex + nFrame, nW, nH, nXCnt, nYCnt);
                            }
                        }
                    }
                }
            }
        }
        delete pBuff;
		return;
    }

    if(m_pstCellInfo){
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

                        if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                            // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F);        // for frame count

                            nFrameCount += shAniCnt;
                        }

                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(fnCheckExistFunc(nFileIndex, nImageIndex + nFrame)){
                                continue;
                            }
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
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

                        if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                            // uint16_t bTickType = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4; // for ticks
                            uint16_t shAniCnt  = (m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F);        // for frame count

                            nFrameCount += shAniCnt;
                        }

                        for(int nFrame = 0; (uint32_t)nFrame < (std::max)((uint32_t)1, nFrameCount); ++nFrame){
                            if(fnCheckExistFunc(nFileIndex, nImageIndex + nFrame)){
                                continue;
                            }
                            if(m_pxTileImage[nFileIndex].SetIndex(nImageIndex + nFrame) && m_pxTileImage[nFileIndex].CurrentImageValid()){
                                int nW = m_pxTileImage[nFileIndex].CurrentImageInfo().shWidth;
                                int nH = m_pxTileImage[nFileIndex].CurrentImageInfo().shHeight;
                                if(nBuffLen < nW * nH){
                                    pBuff    = new uint32_t[nW * nH];
                                    nBuffLen = nW * nH;
                                }
                                m_pxTileImage[nFileIndex].Decode(pBuff, 0XFFFFFFFF, 0XFFFFFFFF);
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
        std::function<bool(uint32_t, uint32_t, Fl_Shared_Image * &, int, int)> fnCheckFunc,
        std::function<void(uint32_t, uint32_t, Fl_Shared_Image *, int, int)> fnDrawObjFunc)
{
    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, m_stMapFileHeader.shWidth - 1);
    nStopCellY  = (std::min)(nStopCellY, m_stMapFileHeader.shHeight - 1);

    if(m_CellDesc){
        for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
            for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
                int  nArrayNum  = nXCnt + nYCnt * m_stMapFileHeader.shWidth;
                auto stCellDesc = m_CellDesc[nArrayNum];

                if(true
                        && stCellDesc.dwDesc    == 0XFFFFFFFF
                        && stCellDesc.dwObject1 == 0XFFFFFFFF
                        && stCellDesc.dwObject2 == 0XFFFFFFFF
                        && stCellDesc.dwLight   == 0XFFFFFFFF
                  ){
                    continue;
                }

                {// first layer:
                    uint32_t nFileIndex  = (((stCellDesc.dwObject1 & 0X03FF0000) >> 16));
                    uint32_t nImageIndex = (((stCellDesc.dwObject1 & 0X0000FFFF)));
                    uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X000000FF)));

                    if(nFileIndex != 255 && nImageIndex != 65535){
                        nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                        if(nObjectDesc != 255){
                            uint8_t bTickType = (nObjectDesc & 0X70) >> 4;
                            int16_t shAniCnt  = (nObjectDesc & 0X0F);

                            nImageIndex += m_bAniTileFrame[bTickType][shAniCnt];
                        }

                        Fl_Shared_Image *p = nullptr;
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt);
                        }
                    }
                }

                {// second layer:
                    uint32_t nFileIndex  = (((stCellDesc.dwObject2 & 0X03FF0000) >> 16));
                    uint32_t nImageIndex = (((stCellDesc.dwObject2 & 0X0000FFFF)));
                    uint32_t nObjectDesc = (((stCellDesc.dwDesc    & 0X0000FF00) >>  8));

                    if(nFileIndex != 255 && nImageIndex != 65535){
                        nImageIndex += GetDoorImageIndex(nXCnt, nYCnt);
                        if(nObjectDesc != 255){
                            uint8_t bTickType = (nObjectDesc & 0X70) >> 4;
                            int16_t shAniCnt  = (nObjectDesc & 0X0F);

                            nImageIndex += m_bAniTileFrame[bTickType][shAniCnt];
                        }

                        Fl_Shared_Image *p = nullptr;
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
        return;
    }

    if(m_pstCellInfo){
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
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt);
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
                        if(fnCheckFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt)){
                            fnDrawObjFunc(nFileIndex, nImageIndex, p, nXCnt, nYCnt);
                        }
                    }
                }
            }
        }
    }
}

void Mir2Map::ExtractGroundInfo(std::function<void(uint32_t, int, int, int)> fnSetGroundInfoFunc)
{
    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            if(m_GroundInfo){
                uint32_t *pU32 = m_GroundInfo + (nX + nY * m_stMapFileHeader.shWidth) * 4;
                fnSetGroundInfoFunc(pU32[0], nX, nY, 0);
                fnSetGroundInfoFunc(pU32[1], nX, nY, 1);
                fnSetGroundInfoFunc(pU32[2], nX, nY, 2);
                fnSetGroundInfoFunc(pU32[3], nX, nY, 3);
                continue;
            }

            if(m_pstCellInfo){
                uint32_t nRawGroundInfo = (CellInfo(nX, nY).bFlag & 0X01) ? 0 : 0XFFFFFFFF;
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 0);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 1);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 2);
                fnSetGroundInfoFunc(nRawGroundInfo, nX, nY, 3);
                continue;
            }

            fnSetGroundInfoFunc(0XFFFFFFFF, nX, nY, 0);
            fnSetGroundInfoFunc(0XFFFFFFFF, nX, nY, 1);
            fnSetGroundInfoFunc(0XFFFFFFFF, nX, nY, 2);
            fnSetGroundInfoFunc(0XFFFFFFFF, nX, nY, 3);
        }
    }
}

void Mir2Map::SetAniTileFrame(int nLoopTime)
{
    // m_bAniTileFrame[i][j]:
    //     i: denotes how fast the animation is.
    //     j: denotes how many frames the animation has.

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


    // cropping done here
    // next for expand to 8 * 8
    int nNewW = 0;
    if(m_stMapFileHeader.shWidth % 8){
        nNewW = (1 + m_stMapFileHeader.shWidth / 8) * 8;
    }else{
        nNewW = m_stMapFileHeader.shWidth;
    }

    int nNewH = 0;
    if(m_stMapFileHeader.shHeight % 8){
        nNewH = (1 + m_stMapFileHeader.shHeight / 8) * 8;
    }else{
        nNewH = m_stMapFileHeader.shHeight;
    }

    if((m_stMapFileHeader.shWidth != nNewW) || (m_stMapFileHeader.shHeight != nNewH)){
        return Expand(nNewW, nNewH);
    }
    return true;
}

bool Mir2Map::EmptyBaseTileBlock(int nStartX, int nStartY, int nSize)
{

    for(int nX = nStartX; nX < nStartX + nSize; nX += 2){
        for(int nY = nStartY; nY < nStartY + nSize; nY += 2){
            if(m_BaseTileInfo){
                int nArrNum = nX / 2 + nY * m_stMapFileHeader.shWidth / 4;
                if(m_BaseTileInfo[nArrNum] != 0XFFFFFFFF){
                    return false;
                }
                // if(m_BaseTileInfo[nArrNum] & 0X00FFFFFF != 0X00FFFFFF){
                //     return false;
                // }
                continue;
            }
            if(m_pstTileInfo){
                auto &stBTInfo = BaseTileInfo(nX, nY);
                // there do have difference here!!
                if(stBTInfo.bFileIndex != 255 && stBTInfo.wTileIndex != 65535){
                    return false;
                }
            }
        }
    }
    return true;
}

void Mir2Map::CompressBaseTileInfoPreOrder(
        int nStartX, int nStartY, int nSize,
        std::vector<bool> &stTileInfoBitV, std::vector<uint32_t> &stTileInfoV)
{
    if(EmptyBaseTileBlock(nStartX, nStartY, nSize)){
        stTileInfoBitV.push_back(false);
    }else{
        stTileInfoBitV.push_back(true);
        if(nSize == 2){
            // precode(8 bit) / fileindex(8 bit) / imageindex(16 bit)
            // new:
            // precode(6 bit) | fileindex(10 bit) | imageindex(16 bit)
            if(m_BaseTileInfo){
                int nArrNum = nStartX / 2 + nStartY * m_stMapFileHeader.shWidth / 4;
                stTileInfoV.push_back(m_BaseTileInfo[nArrNum]);
            }else if(m_pstTileInfo){
                // uint32_t nPrecode    = 0X80000000;
                uint32_t nPrecode    = 0; // for map: code is 0
                uint32_t nFileIndex  = ((uint32_t)(BaseTileInfo(nStartX, nStartY).bFileIndex)) << 16;
                uint32_t nImageIndex = ((uint32_t)(BaseTileInfo(nStartX, nStartY).wTileIndex));
                stTileInfoV.push_back(nPrecode + nFileIndex + nImageIndex);
            }else{
                // can never be here since m_Valid == true;
                printf("error!\n");
            }
        }else{
            CompressBaseTileInfoPreOrder(
                    nStartX, nStartY,
                    nSize / 2,
                    stTileInfoBitV, stTileInfoV);
            CompressBaseTileInfoPreOrder(
                    nStartX + nSize / 2, nStartY,
                    nSize / 2,
                    stTileInfoBitV, stTileInfoV);
            CompressBaseTileInfoPreOrder(
                    nStartX, nStartY + nSize / 2,
                    nSize / 2,
                    stTileInfoBitV, stTileInfoV);
            CompressBaseTileInfoPreOrder(
                    nStartX + nSize / 2, nStartY + nSize / 2,
                    nSize / 2,
                    stTileInfoBitV, stTileInfoV);
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
        uint32_t * pU32BitStream, uint32_t nU32BitStreamLen,
        uint32_t * pU32GroundInfo, uint32_t nU32GroundInfoCount)
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
        uint32_t *pU32BitStream,    uint32_t nU32BitStreamLen,
        uint32_t *pU32BaseTileInfo, uint32_t nU32BaseTileInfoLen)
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
        uint32_t *pU32BitStream, uint32_t nU32BitStreamLen,
        CELLDESC *pCellDesc,     uint32_t nCellDescLen)
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

bool Mir2Map::NewLoadMap(const char *szFullName)
{
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;

    m_Valid = false;

    extern WilImagePackage g_WilImagePackage[128];
    LoadMapImage(g_WilImagePackage);

    auto pFile = fopen(szFullName, "rb");
    if(pFile == nullptr){
        return false;
    }

    // file is already aligned to 64 byte
    fseek(pFile, 0, SEEK_END);
    int nSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto pRawData = new uint8_t[nSize];
    fread(pRawData, 1, nSize, pFile);

    {
        // read map size
        uint16_t *pU16 = (uint16_t *)pRawData;
        m_stMapFileHeader.shWidth  = *pU16++;
        m_stMapFileHeader.shHeight = *pU16++;

        int nMapLoadSize = m_stMapFileHeader.shWidth * m_stMapFileHeader.shHeight;
        m_GroundInfo   = new uint32_t[nMapLoadSize * 4];
        m_BaseTileInfo = new uint32_t[nMapLoadSize];
        m_CellDesc     = new CELLDESC[nMapLoadSize];
    }

    {
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
        if(nBitLen % 32){
            nU32Len = (nBitLen / 32 + 1);
        }else{
            nU32Len = nBitLen / 32;
        }

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

void Mir2Map::OptimizeCell(int nX, int nY)
{
}
