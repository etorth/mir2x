#include "mir2map.hpp"
#include <memory.h>
#include <cstring>
#include <cstdint>

Mir2Map::Mir2Map()
    : m_Valid(false)
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
    m_Valid = false;

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

    m_Valid = true;

    return m_Valid;
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

bool Mir2Map::GroundObjectValid(int nX, int nY, int nIndex, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0XFF00) >> 8);
        nImageIndex = CellInfo(nX, nY).wObj1;
    }else{
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0X00FF) >> 0);
        nImageIndex = CellInfo(nX, nY).wObj2;
    }

    if(stImageDB.Valid((uint8_t)nFileIndex, (uint16_t)nImageIndex)){
        return stImageDB.FastW(nFileIndex) == 48 && stImageDB.FastH(nFileIndex) == 32;
    }
    return false;
}

bool Mir2Map::ObjectValid(int nX, int nY, int nIndex, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0XFF00) >> 8);
        nImageIndex = CellInfo(nX, nY).wObj1;
    }else{
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0X00FF) >> 0);
        nImageIndex = CellInfo(nX, nY).wObj2;
    }

    return stImageDB.Valid((uint8_t)nFileIndex, (uint16_t)nImageIndex);
}

uint32_t Mir2Map::Object(int nX, int nY, int nIndex)
{
    uint32_t nObjectDesc = 0;
    uint32_t nFileIndex  = 0;
    uint32_t nImageIndex = 0;
    if(nIndex == 0){
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0XFF00) >> 8);
        nImageIndex = CellInfo(nX, nY).wObj1;
        nObjectDesc = CellInfo(nX, nY).bObj1Ani;
    }else{
        nFileIndex  = ((CellInfo(nX, nY).wFileIndex & 0X00FF) >> 0);
        nImageIndex = CellInfo(nX, nY).wObj2;
        nObjectDesc = CellInfo(nX, nY).bObj2Ani;
    }
    return (nObjectDesc << 24) + (nFileIndex << 16) + nImageIndex;
}

bool Mir2Map::TileValid(int nX, int nY, ImageDB &stImageDB)
{
    uint32_t nFileIndex  = BaseTileInfo(nX, nY).bFileIndex;
    uint32_t nImageIndex = BaseTileInfo(nX, nY).wTileIndex;
    return stImageDB.Valid((uint8_t)nFileIndex, (uint16_t)nImageIndex);
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
    if(!Valid()){ return 0; }

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

    for(int nX = 0; nX < m_stMapFileHeader.shWidth; ++nX){
        for(int nY = 0; nY < m_stMapFileHeader.shHeight; ++nY){
            uint8_t nRes = GetDoor(nX, nY);
            if(nRes){
                CloseDoor(nX, nY, nRes);
            }
        }
    }
}
