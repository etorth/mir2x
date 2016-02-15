/*
 * =====================================================================================
 *
 *       Filename: editormap.cpp
 *        Created: 02/08/2016 22:17:08
 *  Last Modified: 02/14/2016 23:38:05
 *
 *    Description: EditorMap has no idea of ImageDB, WilImagePackage, etc..
 *                 Use function handler to handle draw, cache, etc
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
#include "mir2map.hpp"
#include "mir2xmap.hpp"
#include "editormap.hpp"
#include "supwarning.hpp"

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
{
    std::memset(m_bAniTileFrame, 0, sizeof(uint8_t) * 8 * 16);
    std::memset(m_dwAniSaveTime, 0, sizeof(uint32_t) * 8);
}

EditorMap::~EditorMap()
{
    delete m_Mir2xMap  ; m_Mir2xMap   = nullptr;
    delete m_OldMir2Map; m_OldMir2Map = nullptr;
}

void EditorMap::ExtractOneTile(int nXCnt, int nYCnt, std::function<void(uint8_t, uint16_t)> fnWritePNG)
{
    if(!Valid() || !ValidC(nXCnt, nYCnt)
            || (nXCnt % 2) || (nYCnt % 2) || TileValid(nXCnt, nYCnt)){ return; }

    uint32_t nDescKey    = Tile(nXCnt, nYCnt);
    uint8_t  nFileIndex  = ((nDescKey & 0X00FF0000) >> 16);
    uint16_t nImageIndex = ((nDescKey & 0X0000FFFF));

    fnWritePNG(nFileIndex, nImageIndex);
}

void EditorMap::ExtractTile(std::function<void(uint8_t, uint16_t)> fnWritePNG)
{
    if(!Valid()){ return; }

    for(int nXCnt = 0; nXCnt < W(); nXCnt++){
        for(int nYCnt = 0; nYCnt < H(); ++nYCnt){
            if(!(nXCnt % 2) && !(nYCnt % 2)){
                ExtractOneTile(nXCnt, nYCnt, fnWritePNG);
            }
        }
    }
}

void EditorMap::DrawTile(
        int nStartCellX, int nStartCellY,
        int nStopCellX,  int nStopCellY,
        std::function<void(uint8_t, uint16_t, int, int)> fnDrawTile)
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
            
            if(!TileValid(nX, nY)){
                continue;
            }

            uint32_t nDescKey    = Tile(nX, nY);
            uint8_t  nFileIndex  = ((nDescKey & 0X00FF0000) >> 16);
            uint16_t nImageIndex = ((nDescKey & 0X0000FFFF));

            // provide cell-coordinates on map
            // fnDrawTile should convert it to drawarea pixel-coordinates
            fnDrawTile(nFileIndex, nImageIndex, nX, nY);
        }
    }
}

void EditorMap::ExtractOneObject(
        int nXCnt, int nYCnt, int nIndex, std::function<void(uint8_t, uint16_t, uint32_t)> fnWritePNG)
{
    if(!Valid() || !ValidC(nXCnt, nYCnt) || !ObjectValid(nXCnt, nYCnt, nIndex)){ return; }

    uint32_t nKey = Object(nXCnt, nYCnt, nIndex);

    bool     bBlend      = ((nKey & 0X80000000) !=  0);
    uint8_t  nFileIndex  = ((nKey & 0X00FF0000) >> 16);
    uint16_t nImageIndex = ((nKey & 0X0000FFFF));
    int      nAniCnt     = ((nKey & 0X0F000000) >> 24);

    int      nFrameCount = (ObjectAni(nXCnt, nYCnt, nIndex) ? nAniCnt : 1);
    uint32_t nImageColor = (bBlend ? 0X80FFFFFF : 0XFFFFFFFF);

    for(int nIndex = 0; nIndex < nFrameCount; ++nIndex){
        fnWritePNG(nFileIndex, nImageIndex + (uint16_t)nIndex, nImageColor);
    }
}

void EditorMap::ExtractObject(std::function<void(uint8_t, uint16_t, uint32_t)> fnWritePNG)
{
    if(!Valid()){ return; }

    for(int nYCnt = 0; nYCnt < H(); ++nYCnt){
        for(int nXCnt = 0; nXCnt < W(); ++nXCnt){
            ExtractOneObject(nXCnt, nYCnt, 0, fnWritePNG);
            ExtractOneObject(nXCnt, nYCnt, 1, fnWritePNG);
        }
    }
}

void EditorMap::DrawObject(
        int nStartCellX, int nStartCellY, int nStopCellX, int nStopCellY,
        bool bGround,
        std::function<void(uint8_t, uint16_t, int, int)> fnDrawObj)
{
    if(!Valid()){ return; }

    nStartCellX = (std::max)(0, nStartCellX);
    nStartCellY = (std::max)(0, nStartCellY);
    nStopCellX  = (std::min)(nStopCellX, W() - 1);
    nStopCellY  = (std::min)(nStopCellY, H() - 1);

    for(int nYCnt = nStartCellY; nYCnt <= nStopCellY; ++nYCnt){
        for(int nXCnt = nStartCellX; nXCnt <= nStopCellX; ++nXCnt){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(ObjectValid(nXCnt, nYCnt, nIndex)
                        && bGround == ObjectGround(nXCnt, nYCnt, nIndex)){

                    uint32_t nKey         = Object(nXCnt, nYCnt, nIndex);
                    uint8_t  nFileIndex   = ((nKey & 0X00FF0000) >> 16);
                    uint16_t nImageIndex  = ((nKey & 0X0000FFFF));
                    int      nAniType     = ((nKey & 0X70000000) >> 28);
                    int      nAniCnt      = ((nKey & 0X0F000000) >> 24);

                    if(ObjectAni(nXCnt, nYCnt, nIndex)){
                        nImageIndex += ObjectOff(nAniType, nAniCnt);
                    }

                    fnDrawObj(nFileIndex, nImageIndex, nXCnt, nYCnt);
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
    auto stOldBufAniObjMark    = m_BufAniObjMark;
    auto stOldBufGround        = m_BufGround;
    auto stOldBufGroundMark    = m_BufGroundMark;

    // this function will clear the new buffer
    // with all zeros
    MakeBuf(nNewW, nNewH);
    InitBuf();

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

                stOldBufAniObjMark     [nDstX / 2][nDstY / 2][0] = m_BufAniObjMark     [nDstX / 2][nDstY / 2][0];
                stOldBufAniObjMark     [nDstX / 2][nDstY / 2][1] = m_BufAniObjMark     [nDstX / 2][nDstY / 2][1];

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
    std::swap(stOldBufAniObjMark   , m_BufAniObjMark   );
    std::swap(stOldBufGround       , m_BufGround       );
    std::swap(stOldBufGroundMark   , m_BufGroundMark   );

    return true;
}

int EditorMap::ObjectBlockType(int nStartX, int nStartY, int nIndex, int nSize)
{
    // assume valid map, valid parameters
    if(nSize == 1){
        return ObjectValid(nStartX, nStartY, nIndex) ? 1 : 0;
    }else{
        bool bFindEmpty = false;
        bool bFindFill  = false;
        bool bFindDiff  = false;

        bool bInited = false;
        uint32_t nObjectSample = 0;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                if(ObjectValid(nX, nY, nIndex)){
                    bFindFill = true;
                    if(bInited){
                        if(nObjectSample != Object(nX, nY, nIndex)){
                            bFindDiff = true;
                        }
                    }else{
                        nObjectSample = Object(nX, nY, nIndex);
                    }
                }else{
                    bFindEmpty = true;
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

int EditorMap::TileBlockType(int nStartX, int nStartY, int nSize)
{
    // assume valid map, valid parameters
    if(nSize == 2){
        return TileValid(nStartX, nStartY) ? 1 : 0;
    }else{
        bool bFindEmpty = false;
        bool bFindFill  = false;
        bool bFindDiff  = false;

        bool bInited = false;
        uint32_t nTileSample = 0;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                if(TileValid(nX, nY)){
                    bFindFill = true;
                    if(bInited){
                        if(nTileSample != Tile(nX, nY)){
                            bFindDiff = true;
                        }
                    }else{
                        nTileSample = Tile(nX, nY);
                    }
                }else{
                    bFindEmpty = true;
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

int EditorMap::LightBlockType(int nStartX, int nStartY, int nSize)
{
    // assume valid map, valid parameters
    if(nSize == 1){
        return LightValid(nStartX, nStartY) ? 1 : 0;
    }else{
        bool bFindEmpty = false;
        bool bFindFill  = false;
        bool bFindDiff  = false;

        bool bInited = false;
        uint16_t nLightSample = 0;

        for(int nX = 0; nX < nSize; ++nX){
            for(int nY = 0; nY < nSize; ++nY){
                if(LightValid(nX, nY)){
                    bFindFill = true;
                    if(bInited){
                        if(nLightSample != Light(nX, nY)){
                            bFindDiff = true;
                        }
                    }else{
                        nLightSample = Light(nX, nY);
                    }
                }else{
                    bFindEmpty = true;
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
                for(int nIdx = 0; nIdx < 4; ++nIdx){
                    if(GroundValid(nX, nY, nIdx)){
                        bFindFill = true;
                        if(bInited){
                            if(nGroundInfoSample != Ground(nX, nY, nIdx)){
                                bFindDiff = true;
                            }
                        }else{
                            nGroundInfoSample = Ground(nX, nY, nIdx);
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
    bool bGroundObj = ObjectGround(nX, nY, nIndex);
    uint32_t nObjDesc = Object(nX, nY, nIndex);

    stMarkV.push_back(bGroundObj);

    stDataV.push_back((uint8_t)((nObjDesc & 0X000000FF ) >>  0));
    stDataV.push_back((uint8_t)((nObjDesc & 0X0000FF00 ) >>  8));
    stDataV.push_back((uint8_t)((nObjDesc & 0X00FF0000 ) >> 16));
    stDataV.push_back((uint8_t)((nObjDesc & 0XFF000000 ) >> 24));
}

void EditorMap::RecordTile(std::vector<uint8_t> &stDataV, int nX, int nY)
{
    uint32_t nTileDesc = Tile(nX, nY);

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

// tile compression is relatively simple, end-level is 2
void EditorMap::DoCompressTile(int nX, int nY, int nSize,
        std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV)
{
    if(!ValidC(nX, nY)){ return; }

    int nType = TileBlockType(nX, nY, nSize);
    if(nType != 0){
        // there is informaiton in this box
        stMarkV.push_back(true);
        if(nSize == 2){
            // there is info, and it's last level, so nType can only be 1
            // end of recursion
            RecordTile(stDataV, nX, nY);
        }else{
            // there is info, and it's not the last level
            if(nType == 2 || nType == 3){
                // there is info and need further parse
                stMarkV.push_back(true);
                DoCompressTile(nX            , nY            , nSize / 2, stMarkV, stDataV);
                DoCompressTile(nX + nSize / 2, nY            , nSize / 2, stMarkV, stDataV);
                DoCompressTile(nX            , nY + nSize / 2, nSize / 2, stMarkV, stDataV);
                DoCompressTile(nX + nSize / 2, nY + nSize / 2, nSize / 2, stMarkV, stDataV);
            }else{
                // nType == 1 here, unified info
                stMarkV.push_back(false);
                RecordTile(stDataV, nX, nY);
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

bool EditorMap::LoadMir2xMap(const char *szFullName)
{
    delete m_OldMir2Map; m_OldMir2Map = nullptr;
    delete m_Mir2xMap  ; m_Mir2xMap   = new Mir2xMap();

    if(m_Mir2xMap->Load(szFullName)){
        MakeBuf(m_Mir2xMap->W(), m_Mir2xMap->H());
        InitBuf();
    }

    delete m_Mir2xMap;
    return Valid();
}

bool EditorMap::LoadMir2Map(const char *szFullName)
{
    delete m_OldMir2Map; m_OldMir2Map = new Mir2Map();
    delete m_Mir2xMap  ; m_Mir2xMap   = nullptr;

    if(m_OldMir2Map->Load(szFullName)){
        MakeBuf(m_OldMir2Map->W(), m_OldMir2Map->H());
        InitBuf();
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
            OptimizeTile(nX, nY);
            OptimizeCell(nX, nY);
        }
    }
}

void EditorMap::OptimizeTile(int nX, int nY)
{
    // TODO
    UNUSED(nX); UNUSED(nY);
}

void EditorMap::OptimizeCell(int nX, int nY)
{
    // TODO
    UNUSED(nX); UNUSED(nY);
}

void EditorMap::ClearBuf()
{
    m_W = 0;
    m_H = 0;
    m_Valid = false;

    m_BufLight.clear();
    m_BufLightMark.clear();
    m_BufTile.clear();
    m_BufTileMark.clear();
    m_BufObj.clear();
    m_BufObjMark.clear();
    m_BufGroundObjMark.clear();
    m_BufGround.clear();
    m_BufGroundMark.clear();
}

bool EditorMap::InitBuf()
{
    if((m_Mir2xMap && m_Mir2xMap->Valid()) || (m_OldMir2Map && m_OldMir2Map->Valid())){
        for(int nY = 0; nY < nH; ++nY){
            for(int nX = 0; nX < nW; ++nX){
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

        if(m_Mir2xMap && m_Mir2xMap->Valid()){
            m_W = m_Mir2xMap->W();
            m_H = m_Mir2xMap->H();
            m_Valid = true;
        }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
            m_W = m_OldMir2Map->W();
            m_H = m_OldMir2Map->H();
            m_Valid = true;
        }else{
            // can't be here
            m_W = 0;
            m_H = 0;
            m_Valid = false;
        }
    }else{
        // otherwise m_Valid is false
        m_Valid = false;
    }

    return m_Valid;
}

void EditorMap::MakeBuf(int nW, int nH)
{
    // make a buffer for loading new map
    // or extend / crop old map
    ClearBuf();
    if(nW == 0 || nH == 0 || nW % 2 || nH % 2){ return; }

    // m_BufLight[nX][nY]
    m_BufLight = std::vector<std::vector<uint16_t>>(nW, std::vector<uint16_t>(nH, 0));
    // m_BufLightMark[nX][nY]
    m_BufLightMark = std::vector<std::vector<int>>(nW, std::vector<int>(nH, 0));

    // m_BufTile[nX][nY]
    m_BufTile = std::vector<std::vector<uint32_t>>(nW / 2, std::vector<uint32_t>(nH / 2, 0));
    // m_BufTileMark[nX][nY]
    m_BufTileMark = std::vector<std::vector<int>>(nW / 2, std::vector<int>(nH / 2, 0));

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
    uint32_t nObj       = 0;
    int      nObjValid  = 0;
    int      nGroundObj = 0;
    int      nAniObj    = 0;

    if(m_Mir2xMap && m_Mir2xMap->Valid()){
        // mir2x map
        if(m_Mir2xMap->ObjectValid(nX, nY, nIndex)){
            nObjValid = 1;
            if(m_Mir2xMap->ObjectGround(nX, nY, nIndex)){
                nGroundObj = 1;
            }
            if(m_Mir2xMap->ObjectAni(nX, nY, nIndex)){
                nAniObj = 1;
            }
            nObj = m_Mir2xMap->Object(nX, nY, nIndex);
        }
    }else if(m_OldMir2Map && m_OldMir2Map->Valid()){
        // mir2 map
        if(m_OldMir2Map->ObjectValid(nX, nY, nIndex)){
            nObjValid = 1;
            if(m_OldMir2Map->ObjectGround(nX, nY, nIndex)){
                nGroundObj = 1;
            }
            if(m_OldMir2Map->ObjectAni(nX, nY, nIndex)){
                nAniObj = 1;
            }
            nObj = m_OldMir2Map->Object(nX, nY, nIndex);
        }
    }

    m_BufObj          [nX][nY][nIndex] = nObj;
    m_BufObjMark      [nX][nY][nIndex] = nObjValid;
    m_BufGroundObjMark[nX][nY][nIndex] = nGroundObj;
    m_BufAniObjMark   [nX][nY][nIndex] = nAniObj;
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
