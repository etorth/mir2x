/*
 * =====================================================================================
 *
 *       Filename: mir2map.cpp
 *        Created: 05/03/2016 15:00:35
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <cstring>
#include <cstdint>
#include <memory.h>
#include "totype.hpp"
#include "mir2map.hpp"
#include "sysconst.hpp"

Mir2Map::Mir2Map()
    : m_valid(false)
    , m_pstTileInfo(nullptr)
    , m_pstCellInfo(nullptr)
{
    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));
}

Mir2Map::~Mir2Map()
{
    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;
}

bool Mir2Map::Load(const char *szMapFileName)
{
    m_valid = false;

    delete []m_pstTileInfo;  m_pstTileInfo  = nullptr;
    delete []m_pstCellInfo;  m_pstCellInfo  = nullptr;

    std::memset(&m_stMapFileHeader, 0, sizeof(MAPFILEHEADER));

    auto hFile = fopen(szMapFileName, "rb");
    if(hFile == nullptr){
        return false;
    }

    if(fread(&m_stMapFileHeader, sizeof(MAPFILEHEADER), 1, hFile) != 1){
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

    m_valid = true;

    return m_valid;
}

uint8_t Mir2Map::Flag(int nX, int nY)
{
    return CellInfo(nX, nY).bFlag;
}

bool Mir2Map::LightValid(int nX, int nY)
{
    return (CellInfo(nX, nY).wLightNEvent != 0) || ((CellInfo(nX, nY).wLightNEvent & 0X0007) == 1);
}

uint16_t Mir2Map::Light(int nX, int nY)
{
    return CellInfo(nX, nY).wLightNEvent;
}

bool Mir2Map::GroundValid(int nX, int nY)
{
    return (CellInfo(nX, nY).bFlag & 0X01) != 0;
}

uint8_t Mir2Map::Ground(int nX, int nY)
{
    return CellInfo(nX, nY).bFlag;
}

bool Mir2Map::AniObjectValid(int nX, int nY, int nIndex, ImageDB &stImageDB)
{
    if(ObjectValid(nX, nY, nIndex, stImageDB)){
        if(nIndex == 0){
            return CellInfo(nX, nY).bObj1Ani != 255;
        }else{
            return CellInfo(nX, nY).bObj2Ani != 255;
        }
    }
    return false;
}

bool Mir2Map::GroundObjectValid(int nX, int nY, int nIndex, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = CellInfo(nX, nY).bFileIndex1;
        nImageIndex = CellInfo(nX, nY).wObj1;
    }else{
        nFileIndex  = CellInfo(nX, nY).bFileIndex2;
        nImageIndex = CellInfo(nX, nY).wObj2;
    }

    if(true
            && nFileIndex > 0
            && nFileIndex < 75

            && nImageIndex != 65536
            && stImageDB.setIndex(to_u8(nFileIndex), to_u16(nImageIndex))){
        return stImageDB.currImageInfo()->width == SYS_MAPGRIDXP && stImageDB.currImageInfo()->height == SYS_MAPGRIDYP;
    }
    return false;
}

bool Mir2Map::ObjectValid(int nX, int nY, int nIndex, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = CellInfo(nX, nY).bFileIndex1;
        nImageIndex = CellInfo(nX, nY).wObj1;
    }else{
        nFileIndex  = CellInfo(nX, nY).bFileIndex2;
        nImageIndex = CellInfo(nX, nY).wObj2;
    }

    if(true
            && nFileIndex > 0
            && nFileIndex < 75

            && nImageIndex != 65536

            && stImageDB.setIndex(to_u8(nFileIndex), to_u16(nImageIndex))){
        return true;
    }
    return false;
}

uint32_t Mir2Map::Object(int nX, int nY, int nIndex)
{
    uint32_t nObjectDesc = 0;
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = CellInfo(nX, nY).bFileIndex1;
        nImageIndex = CellInfo(nX, nY).wObj1;
        nObjectDesc = CellInfo(nX, nY).bObj1Ani;
    }else{
        nFileIndex  = CellInfo(nX, nY).bFileIndex2;
        nImageIndex = CellInfo(nX, nY).wObj2;
        nObjectDesc = CellInfo(nX, nY).bObj2Ani;
    }
    // didn't change ObjAni, the highest bit of it is for alpha still
    return (nObjectDesc << 24) + (nFileIndex << 16) + nImageIndex;
}

bool Mir2Map::TileValid(int nX, int nY, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = BaseTileInfo(nX, nY).bFileIndex;
    uint32_t nImageIndex = BaseTileInfo(nX, nY).wTileIndex;
    return stImageDB.setIndex(to_u8(nFileIndex), to_u16(nImageIndex));
}

uint32_t Mir2Map::Tile(int nX, int nY)
{
    uint32_t nFileIndex  = BaseTileInfo(nX, nY).bFileIndex;
    uint32_t nImageIndex = BaseTileInfo(nX, nY).wTileIndex;
    return (nFileIndex << 16) + nImageIndex;
}

uint32_t Mir2Map::GetDoorImageIndex(int nX, int nY)
{
    uint32_t nDoorIndex = 0;
    if(Valid()){
        // seems bDoorOffset & 0X80 shows open or close
        //       bDoorIndex  & 0X80 shows whether there is a door
        if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorOffset & 0X80) > 0){
            if((m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorIndex & 0X7F) > 0){
                nDoorIndex += (m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight].bDoorOffset & 0X7F);
            }
        }
    }
    return nDoorIndex;
}

uint8_t Mir2Map::GetDoor(int nXCnt, int nYCnt)
{
    uint8_t bRes = 0;
    if(Valid()){
        int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
        if(m_pstCellInfo[nArrayNum].bDoorIndex & 0X80){
            bRes = (m_pstCellInfo[nArrayNum].bDoorIndex & 0X7F);
        }
    }
    return bRes;
}

void Mir2Map::OpenDoor(int nX, int nY, uint8_t nDoorIndex)
{
    if(Valid()){
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
}

void Mir2Map::CloseDoor(int nX, int nY, uint8_t nDoorIndex)
{
    if(Valid()){
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
}

void Mir2Map::OpenAllDoor()
{
    if(Valid()){
        for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
            for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
                uint8_t nRes = GetDoor(nX, nY);
                if(nRes){
                    OpenDoor(nX, nY, nRes);
                }
            }
        }
    }
}

void Mir2Map::CloseAllDoor()
{
    if(Valid()){
        for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
            for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
                uint8_t nRes = GetDoor(nX, nY);
                if(nRes){
                    CloseDoor(nX, nY, nRes);
                }
            }
        }
    }
}

std::string Mir2Map::MapInfo()
{
    if(!Valid()){ return std::string("Invalid map!"); }

    int nLightCount = 0;
    int nDoorCount  = 0;
    int nAlphaBlend = 0;

    // Tile Infomation doesn't include any for alpha-blend, light or door

    for(int nXCnt = 0; nXCnt < m_stMapFileHeader.shWidth; ++nXCnt){
        for(int nYCnt = 0; nYCnt < m_stMapFileHeader.shHeight; ++nYCnt){

            int nArrayNum = nYCnt + nXCnt * m_stMapFileHeader.shHeight;
            {
                // for light info
                if((m_pstCellInfo[nArrayNum].wLightNEvent != 0)
                        || (m_pstCellInfo[nArrayNum].wLightNEvent & 0X0007) == 1){
                    // TODO quite doubt of this logic
                    // seems the 15~4 for specified light-frog
                    // 3~0 for general light-frog
                    nLightCount++;
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
                    nDoorCount++;

                    printf("%d, %d, %d\n", nXCnt, nYCnt, GetDoorImageIndex(nXCnt, nYCnt));
                }
            }

            // for light info
            {
                // first layer:
                int nFileIndex = m_pstCellInfo[nArrayNum].bFileIndex1;
                if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj1 != 65535){
                    if(m_pstCellInfo[nArrayNum].bObj1Ani != 255){
                        if(m_pstCellInfo[nArrayNum].bObj1Ani & 0X80){
                            nAlphaBlend++;
                        }
                    }
                }
            }

            {
                // second layer:
                int nFileIndex = m_pstCellInfo[nArrayNum].bFileIndex2;
                if(nFileIndex != 255 && m_pstCellInfo[nArrayNum].wObj2 != 65535){
                    if(m_pstCellInfo[nArrayNum].bObj2Ani != 255){
                        if(m_pstCellInfo[nArrayNum].bObj2Ani & 0X80){
                            nAlphaBlend++;
                        }
                    }
                }
            }
        }
    }

    std::string szMapInfo;
    char szTmpInfo[128];

    std::sprintf(szTmpInfo, "Width:%d\n", m_stMapFileHeader.shWidth);
    szMapInfo += szTmpInfo;

    std::sprintf(szTmpInfo, "Height:%d\n", m_stMapFileHeader.shHeight);
    szMapInfo += szTmpInfo;

    std::sprintf(szTmpInfo, "Alpha:%d\n", nAlphaBlend);
    szMapInfo += szTmpInfo;

    std::sprintf(szTmpInfo, "Door:%d\n", nDoorCount);
    szMapInfo += szTmpInfo;

    std::sprintf(szTmpInfo, "Light:%d\n", nLightCount);
    szMapInfo += szTmpInfo;

    return szMapInfo;
}
