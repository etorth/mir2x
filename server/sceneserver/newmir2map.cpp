#include "newmir2map.hpp"
#include <utility>
#include <memory.h>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <vector>
#include "monster.hpp"
#include "directiverectcover.hpp"

NewMir2Map::NewMir2Map()
    : m_GroundInfo(nullptr)
    , m_BaseTileInfo(nullptr)
    , m_CellDesc(nullptr)
    , m_Valid(false)
    , m_W(0)
    , m_H(0)
{}

NewMir2Map::~NewMir2Map()
{
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;
}

bool NewMir2Map::NewLoadMap(const char *szFullName)
{
    delete []m_GroundInfo;   m_GroundInfo   = nullptr;
	delete []m_BaseTileInfo; m_BaseTileInfo = nullptr;
    delete []m_CellDesc;     m_CellDesc     = nullptr;

    m_Valid = false;

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
        m_W = *pU16++;
        m_H = *pU16++;

        int nMapLoadSize = m_W * m_H;
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

int NewMir2Map::Width()
{
    return m_W;
}

int NewMir2Map::Height()
{
    return m_H;
}

bool NewMir2Map::Valid()
{
    return m_Valid;
}

uint32_t NewMir2Map::BitPickOne(uint32_t *pU32BitStream, uint32_t nOffset)
{
    // nOffset can only be even number
    uint32_t nShift = 31 - (nOffset % 32);
    return ((uint32_t)(pU32BitStream[nOffset / 32] & ((uint32_t)0X01) << nShift)) >> nShift;
}

void NewMir2Map::SetOneGroundInfoGrid(
        int nStartX, int nStartY, int nSubGrid, uint32_t nGroundInfoAttr)
{
    // be careful! here it's not arranged as old format
    int nOffset = (nStartY * m_W + nStartX) * 4 + (nSubGrid % 4);
    m_GroundInfo[nOffset] = nGroundInfoAttr;
}

void NewMir2Map::SetGroundInfoBlock(
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

void NewMir2Map::ParseGroundInfoStream(int nStartX, int nStartY, int nSize,
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

bool NewMir2Map::LoadGroundInfo(
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

void NewMir2Map::SetBaseTileBlock(
        int nStartX, int nStartY, int nSize, uint32_t nAttr)
{
    for(int nY = nStartY; nY < nStartY + nSize; nY += 2){
        for(int nX = nStartX; nX < nStartX + nSize; nX += 2){
            m_BaseTileInfo[(nY / 2) * (m_W / 2) + (nX / 2)] = nAttr;
        }
    }
}

void NewMir2Map::ParseBaseTileStream(int nStartX, int nStartY, int nSize,
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

bool NewMir2Map::LoadBaseTileInfo(
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

void NewMir2Map::SetCellDescBlock(
        int nStartX, int nStartY, int nSize, const CELLDESC & stCellDesc)
{
    for(int nY = nStartY; nY < nStartY + nSize; ++nY){
        for(int nX = nStartX; nX < nStartX + nSize; ++nX){
            m_CellDesc[nY * m_W + nX] = stCellDesc;
        }
    }
}

void NewMir2Map::ParseCellDescStream(int nStartX, int nStartY, int nSize,
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

bool NewMir2Map::LoadCellDesc(
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

bool NewMir2Map::ValidPosition(int nX, int nY, const Monster *pMonster)
{
    // TODO add ground info check
    DirectiveRectCover stDRC(
            pMonster->Direction(),
            nX, nY,
            pMonster->DRCover().W(),
            pMonster->DRCover().H());

	return !Overlap(stDRC);
}

bool NewMir2Map::Overlap(const DirectiveRectCover &stDRC)
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

bool NewMir2Map::Overlap(const Triangle &stTri)
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
