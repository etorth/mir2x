/*
 * =====================================================================================
 *
 *       Filename: editormap.cpp
 *        Created: 02/08/2016 22:17:08
 *  Last Modified: 08/18/2017 17:57:02
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

#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <functional>
#include <FL/fl_ask.H>

#include "savepng.hpp"
#include "filesys.hpp"
#include "mir2map.hpp"
#include "sysconst.hpp"
#include "mathfunc.hpp"
#include "editormap.hpp"

EditorMap::EditorMap()
    : m_W(0)
    , m_H(0)
    , m_Valid(false)
    , m_Mir2Map(nullptr)
    , m_Mir2xMapData(nullptr)
    , m_BlockBuf()
{
    std::memset(m_AniSaveTime,  0, sizeof(m_AniSaveTime));
    std::memset(m_AniTileFrame, 0, sizeof(m_AniTileFrame));
}

EditorMap::~EditorMap()
{
    delete m_Mir2Map;
    delete m_Mir2xMapData;
}

void EditorMap::ExtractOneTile(int nX, int nY, std::function<void(uint8_t, uint16_t)> fnWritePNG)
{
    if(true
            &&  Valid()
            &&  ValidC(nX, nY)
            && !(nX % 2)
            && !(nY % 2)
            &&  m_BlockBuf[nX / 2][nY / 2].Tile.Valid){

        auto nFileIndex  = (uint8_t )((m_BlockBuf[nX / 2][nY / 2].Tile.Image & 0X00FF0000) >> 16);
        auto nImageIndex = (uint16_t)((m_BlockBuf[nX / 2][nY / 2].Tile.Image & 0X0000FFFF) >>  0);

        fnWritePNG(nFileIndex, nImageIndex);
    }
}

void EditorMap::ExtractTile(std::function<void(uint8_t, uint16_t)> fnWritePNG)
{
    if(Valid()){
        for(int nXCnt = 0; nXCnt < W(); ++nXCnt){
            for(int nYCnt = 0; nYCnt < H(); ++nYCnt){
                if(!(nXCnt % 2) && !(nYCnt % 2)){
                    ExtractOneTile(nXCnt, nYCnt, fnWritePNG);
                }
            }
        }
    }
}

void EditorMap::DrawLight(int nX, int nY, int nW, int nH, std::function<void(int, int)> fnDrawLight)
{
    if(Valid()){
        for(int nTX = 0; nTX < nW; ++nTX){
            for(int nTY = 0; nTY < nH; ++nTY){
                if(ValidC(nTX + nX, nTY + nY)){
                    auto &rstLight = m_BlockBuf[(nTX + nX) / 2][(nTY + nY) / 2].Cell[(nTX + nX) % 2][(nTY + nY) % 2].Light;
                    if(rstLight.Valid){
                        fnDrawLight(nTX + nX, nTY + nY);
                    }
                }
            }
        }
    }
}

void EditorMap::DrawTile(int nCX, int nCY, int nCW,  int nCH, std::function<void(uint8_t, uint16_t, int, int)> fnDrawTile)
{
    if(Valid()){
        for(int nY = nCY; nY < nCY + nCH; ++nY){
            for(int nX = nCX; nX < nCX + nCW; ++nX){
                if(!ValidC(nX, nY)){
                    continue;
                }

                if(nX % 2 || nY % 2){
                    continue;
                }

                if(!m_BlockBuf[nX / 2][nY / 2].Tile.Valid){
                    continue;
                }

                auto nFileIndex  = (uint8_t )((m_BlockBuf[nX / 2][nY / 2].Tile.Image & 0X00FF0000) >> 16);
                auto nImageIndex = (uint16_t)((m_BlockBuf[nX / 2][nY / 2].Tile.Image & 0X0000FFFF) >>  0);

                // provide cell-coordinates on map
                // fnDrawTile should convert it to drawarea pixel-coordinates
                fnDrawTile(nFileIndex, nImageIndex, nX, nY);
            }
        }
    }
}

void EditorMap::ExtractOneObject(int nXCnt, int nYCnt, int nIndex, std::function<void(uint8_t, uint16_t, uint32_t)> fnWritePNG)
{
    if(true
            && Valid()
            && ValidC(nXCnt, nYCnt)
            && nIndex >= 0
            && nIndex <= 1){

        auto &rstObj = m_BlockBuf[nXCnt / 2][nYCnt / 2].Cell[nXCnt % 2][nYCnt % 2].Obj[nIndex];
        if(rstObj.Valid){
            auto nFileIndex  = (uint8_t )((rstObj.Image & 0X00FF0000) >> 16);
            auto nImageIndex = (uint16_t)((rstObj.Image & 0X0000FFFF) >>  0);

            int      nFrameCount = (rstObj.Animated ? rstObj.AniCount : 1);
            uint32_t nImageColor = (rstObj.Alpha ? 0X80FFFFFF : 0XFFFFFFFF);

            for(int nIndex = 0; nIndex < nFrameCount; ++nIndex){
                fnWritePNG(nFileIndex, nImageIndex + (uint16_t)(nIndex), nImageColor);
            }
        }
    }
}

void EditorMap::ExtractObject(std::function<void(uint8_t, uint16_t, uint32_t)> fnWritePNG)
{
    if(Valid()){
        for(int nYCnt = 0; nYCnt < H(); ++nYCnt){
            for(int nXCnt = 0; nXCnt < W(); ++nXCnt){
                ExtractOneObject(nXCnt, nYCnt, 0, fnWritePNG);
                ExtractOneObject(nXCnt, nYCnt, 1, fnWritePNG);
            }
        }
    }
}

void EditorMap::DrawObject(int nCX, int nCY, int nCW, int nCH, bool bGround,
        std::function<void(uint8_t, uint16_t, int, int)> fnDrawObj, std::function<void(int, int)> fnDrawExt)
{
    if(Valid()){
        for(int nY = nCY; nY < nCY + nCH; ++nY){
            for(int nX = nCX; nX < nCX + nCW; ++nX){
                // // we should draw actors, extensions here, but I delay it after the over-ground
                // // object drawing, it's really wired but works
                // if(!bGround){ fnDrawExt(nX, nY); }

                // 2. regular draw
                if(ValidC(nX, nY)){
                    for(int nIndex = 0; nIndex < 2; ++nIndex){
                        auto &rstObj = m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex];
                        if(true
                                && rstObj.Valid
                                && rstObj.Ground == bGround){

                            auto nFileIndex  = (uint8_t )((rstObj.Image & 0X00FF0000) >> 16);
                            auto nImageIndex = (uint16_t)((rstObj.Image & 0X0000FFFF) >>  0);
                            auto nAnimated   = (bool    )((rstObj.Animated));
                            auto nAniType    = (uint8_t )((rstObj.AniType ));
                            auto nAniCount   = (uint8_t )((rstObj.AniCount));

                            if(nAnimated){
                                nImageIndex += (uint16_t)(m_AniTileFrame[nAniType][nAniCount]);
                            }

                            fnDrawObj(nFileIndex, nImageIndex, nX, nY);
                        }
                    }
                }

                // put the actors, extensions rendering here, it's really really wired but
                // works, tricky part for mir2 resource maker
                // if(!bGround){ fnDrawExt(nX, nY); }
            }

            for(int nX = nCX; nX < nCX + nCW; ++nX){
                if(!bGround){ fnDrawExt(nX, nY); }
            }
        }
    }
}

void EditorMap::UpdateFrame(int nLoopTime)
{
    // m_AniTileFrame[i][j]:
    //   i: denotes how fast the animation is.
    //   j: denotes how many frames the animation has.

    if(Valid()){
        const static uint32_t nDelayMS[] = {150, 200, 250, 300, 350, 400, 420, 450};
        for(int nCnt = 0; nCnt < 8; ++nCnt){
            m_AniSaveTime[nCnt] += nLoopTime;
            if(m_AniSaveTime[nCnt] > nDelayMS[nCnt]){
                for(int nFrame = 0; nFrame < 16; ++nFrame){
                    m_AniTileFrame[nCnt][nFrame]++;
                    if(m_AniTileFrame[nCnt][nFrame] >= nFrame){
                        m_AniTileFrame[nCnt][nFrame] = 0;
                    }
                }
                m_AniSaveTime[nCnt] = 0;
            }
        }
    }
}

bool EditorMap::Resize(
        int nX, int nY, int nW, int nH, // define a region on original map
        int nNewX, int nNewY,           // where the cropped region start on new map
        int nNewW, int nNewH)           // size of new map
{
    // we only support 2M * 2N cropping and expansion
    if(Valid()){
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
                && nNewH == nH){
            // return if nothing need to do
            return true;
        }

        if(nNewW <= 0 || nNewH <= 0){
            return false;
        }

        // region is not empty
        // OK now we have a practical job to do

        auto stOldBlockBuf = m_BlockBuf;
        MakeBuf(nNewW, nNewH);

        for(int nTY = 0; nTY < nH; ++nTY){
            for(int nTX = 0; nTX < nW; ++nTX){
                int nSrcX = nTX + nX;
                int nSrcY = nTY + nY;
                int nDstX = nTX + nNewX;
                int nDstY = nTY + nNewY;

                if(true
                        && nDstX >= 0 && nDstX < nNewW
                        && nDstY >= 0 && nDstY < nNewH){

                    if(true
                            && !(nDstX % 2) && !(nDstY % 2)
                            && !(nSrcX % 2) && !(nSrcY % 2)){
                        m_BlockBuf[nDstX / 2][nDstY / 2] = stOldBlockBuf[nSrcX / 2][nSrcY / 2];
                    }
                }
            }
        }

        m_Valid = true;
        m_W     = nNewW;
        m_H     = nNewH;

        return true;
    }
    return false;
}

bool EditorMap::LoadMir2Map(const char *szFullName)
{
    delete m_Mir2Map     ; m_Mir2Map      = new Mir2Map();
    delete m_Mir2xMapData; m_Mir2xMapData = nullptr;

    if(m_Mir2Map->Load(szFullName)){
        MakeBuf(m_Mir2Map->W(), m_Mir2Map->H());
        InitBuf();
    }

    delete m_Mir2xMapData; m_Mir2xMapData = nullptr;
    delete m_Mir2xMapData; m_Mir2xMapData = nullptr;

    return Valid();
}

bool EditorMap::LoadMir2xMapData(const char *szFullName)
{
    delete m_Mir2Map     ; m_Mir2Map      = nullptr;
    delete m_Mir2xMapData; m_Mir2xMapData = new Mir2xMapData();

    if(!m_Mir2xMapData->Load(szFullName)){
        MakeBuf(m_Mir2xMapData->W(), m_Mir2xMapData->H());
        InitBuf();
    }

    delete m_Mir2xMapData; m_Mir2xMapData = nullptr;
    delete m_Mir2xMapData; m_Mir2xMapData = nullptr;

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

void EditorMap::OptimizeTile(int, int)
{
}

void EditorMap::OptimizeCell(int, int)
{
}

void EditorMap::ClearBuf()
{
    m_W = 0;
    m_H = 0;

    m_Valid = false;
    m_BlockBuf.clear();
}

bool EditorMap::InitBuf()
{
    int nW = 0;
    int nH = 0;

    m_Valid = false;

    if(m_Mir2Map && m_Mir2Map->Valid()){
        nW = m_Mir2Map->W();
        nH = m_Mir2Map->H();
    }else if(m_Mir2xMapData && m_Mir2xMapData->Valid()){
        nW = m_Mir2xMapData->W();
        nH = m_Mir2xMapData->H();
    }else{
        return false;
    }

    for(int nY = 0; nY < nH; ++nY){
        for(int nX = 0; nX < nW; ++nX){
            if(!(nX % 2) && !(nY % 2)){
                SetBufTile(nX, nY);
            }

            SetBufLight(nX, nY);
            SetBufGround(nX, nY);

            SetBufObj(nX, nY, 0);
            SetBufObj(nX, nY, 1);
        }
    }

    m_W     = nW;
    m_H     = nH;
    m_Valid = true;

    return true;
}

void EditorMap::MakeBuf(int nW, int nH)
{
    // make a buffer for loading new map
    // or extend / crop old map
    ClearBuf();
    if(nW == 0 || nH == 0 || nW % 2 || nH % 2){ return; }

    m_BlockBuf.resize(nW / 2);
    for(auto &rstBuf: m_BlockBuf){
        rstBuf.resize(nH / 2);
        std::memset(&(rstBuf[0]), 0, rstBuf.size() * sizeof(rstBuf[0]));
    }
}

void EditorMap::SetBufTile(int nX, int nY)
{
    // don't check Valid() and ValidC()
    // called in InitBuf() and where Valid() is not set yet

    if(true
            && !(nX % 2) && !(nY % 2)

            && nX >= 0
            && nX / 2 < (int)(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < (int)(m_BlockBuf[nX / 2].size())){

        if(m_Mir2Map && m_Mir2Map->Valid()){
            extern ImageDB g_ImageDB;
            if(m_Mir2Map->TileValid(nX, nY, g_ImageDB)){
                m_BlockBuf[nX / 2][nY / 2].Tile.Valid = true;
                m_BlockBuf[nX / 2][nY / 2].Tile.Image = m_Mir2Map->Tile(nX, nY) & 0X00FFFFFF;
            }
        }else if(m_Mir2xMapData && m_Mir2xMapData->Valid()){
            if(m_Mir2xMapData->Tile(nX, nY).Param & 0X80000000){
                m_BlockBuf[nX / 2][nY / 2].Tile.Valid = true;
                m_BlockBuf[nX / 2][nY / 2].Tile.Image = m_Mir2xMapData->Tile(nX, nY).Param & 0X00FFFFFF;
            }
        }
    }
}

void EditorMap::SetBufGround(int nX, int nY)
{
    if(true
            && nX >= 0
            && nX / 2 < (int)(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < (int)(m_BlockBuf[nX / 2].size())){

        bool    bCanFly    = false;
        bool    bCanWalk   = false;
        uint8_t nAttribute = 0;

        if(m_Mir2Map && m_Mir2Map->Valid()){
            if(m_Mir2Map->GroundValid(nX, nY)){
                bCanFly    = true;
                bCanWalk   = true;
                nAttribute = 0;
            }
        }else if(m_Mir2xMapData && m_Mir2xMapData->Valid()){
            uint8_t nLandByte = m_Mir2xMapData->Cell(nX, nY).LandByte();
            bCanWalk   = (nLandByte & 0X80) ? true : false;
            bCanFly    = (nLandByte & 0X40) ? true : false;
            nAttribute = (nLandByte & 0X3F);
        }

        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].CanFly    = bCanFly;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].CanWalk   = bCanWalk;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].LandType = nAttribute;
    }
}

void EditorMap::SetBufObj(int nX, int nY, int nIndex)
{
    if(true
            && nIndex >= 0
            && nIndex <= 1

            && nX >= 0
            && nX / 2 < (int)(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < (int)(m_BlockBuf[nX / 2].size())){

        uint32_t nObj       = 0;
        bool     bObjValid  = false;
        bool     bGroundObj = false;
        bool     bAniObj    = false;
        bool     bAlphaObj  = false;
        uint8_t  nAniType   = 0;
        uint8_t  nAniCount  = 0;

        if(m_Mir2Map && m_Mir2Map->Valid()){
            // mir2 map
            // animation info is in Mir2Map::Object() at higher 8 bits
            extern ImageDB g_ImageDB;
            if(m_Mir2Map->ObjectValid(nX, nY, nIndex, g_ImageDB)){
                bObjValid = true;
                if(m_Mir2Map->GroundObjectValid(nX, nY, nIndex, g_ImageDB)){
                    bGroundObj = true;
                }
                if(m_Mir2Map->AniObjectValid(nX, nY, nIndex, g_ImageDB)){
                    bAniObj = true;
                }

                // [31:24] : animation info
                // [23:16] : file index
                // [15: 0] : image index

                auto nObjDesc = m_Mir2Map->Object(nX, nY, nIndex);
                nAniType  = (uint8_t )((nObjDesc & 0X70000000) >> (4 + 8 + 16));
                nAniCount = (uint8_t )((nObjDesc & 0X0F000000) >> (0 + 8 + 16));
                nObj      = (uint32_t)((nObjDesc & 0X00FFFFFF));

                // in mir2map if an object is not animated
                // then it shouldn't be alpha-blended, check GameProc.cpp
                if(bAniObj){
                    bAlphaObj = ((nObj & 0X80000000) ? true : false);
                }else{
                    bAlphaObj = false;
                    nAniType  = 0;
                    nAniCount = 0;
                }
            }
        }else if(m_Mir2xMapData && m_Mir2xMapData->Valid()){
            auto stArray = m_Mir2xMapData->Cell(nX, nY).ObjectArray(nIndex);
            if(stArray[4] & 0X80){
                bObjValid = true;
                nObj = 0
                    | (((uint32_t)(stArray[2])) << 16)
                    | (((uint32_t)(stArray[1])) <<  8)
                    | (((uint32_t)(stArray[0])) <<  0);

                if(stArray[4] & 0X01){
                    bGroundObj = true;
                }

                if(stArray[3] & 0X80){
                    bAniObj   = true;
                    nAniType  = ((stArray[3] & 0X70) >> 4);
                    nAniCount = ((stArray[3] & 0X0F) >> 0);
                }

                if(stArray[4] & 0X02){
                    bAlphaObj = true;
                }
            }
        }

        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].Valid    = bObjValid;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].Alpha    = bAlphaObj;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].Ground   = bGroundObj;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].Animated = bAniObj;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].AniType  = nAniType;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].AniCount = nAniCount;
        m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Obj[nIndex].Image    = nObj;
    }
}

void EditorMap::SetBufLight(int nX, int nY)
{
    if(true
            && nX >= 0
            && nX / 2 < (int)(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < (int)(m_BlockBuf[nX / 2].size())){

        if(m_Mir2Map && m_Mir2Map->Valid()){
            if(m_Mir2Map->LightValid(nX, nY)){
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Valid  = true;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Color  = 0;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Alpha  = 0;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Radius = 0;
            }
        }else if(m_Mir2xMapData && m_Mir2xMapData->Valid()){
            auto nLightByte = m_Mir2xMapData->Cell(nX, nY).LightByte();
            if(nLightByte & 0X80){
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Valid  = true;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Color  = 0;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Alpha  = 0;
                m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2].Light.Radius = 0;
            }
        }
    }
}

void EditorMap::DrawSelectGround(int nX, int nY, int nW, int nH, std::function<void(int, int, int)> fnDrawSelectGround)
{
    if(Valid()){
        for(int nTX = nX; nTX < nX + nW; ++nTX){
            for(int nTY = nY; nTY < nY + nH; ++nTY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    if(ValidC(nTX, nTY) && Cell(nTX, nTY).SelectGround){
                        fnDrawSelectGround(nX, nY, nIndex);
                    }
                }
            }
        }
    }
}

void EditorMap::ClearGroundSelect()
{
    if(Valid()){
        for(int nX = 0; nX < W(); ++nX){
            for(int nY = 0; nY < H(); ++nY){
                for(int nIndex = 0; nIndex < 4; ++nIndex){
                    Cell(nX, nY).SelectGround = false;
                }
            }
        }
    }
}

bool EditorMap::SaveMir2xMapData(const char *szFullName)
{
    if(!Valid()){
        fl_alert("%s", "Invalid editor map!");
        return false;
    }

    Mir2xMapData stMapData;
    stMapData.Allocate(W(), H());

    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){

            // tile
            if(!(nX % 2) && !(nY % 2)){
                stMapData.Tile(nX, nY).Param = Tile(nX, nY).MakeU32();
            }

            // cell
            auto &rstDstCell = stMapData.Cell(nX, nY);
            std::memset(&rstDstCell, 0, sizeof(rstDstCell));

            // cell::land
            rstDstCell.Param |= (((uint32_t)(Cell(nX, nY).MakeLandU8())) << 16);

            // cell::light
            rstDstCell.Param |= (((uint32_t)(Light(nX, nY).MakeU8())) << 8);

            // cell::obj[0]
            {
                auto stArray = Object(nX, nY, 0).MakeArray();
                if(stArray[4] & 0X80){
                    rstDstCell.Obj[0].Param = 0X80000000
                        | (((uint32_t)(stArray[2])) << 16)
                        | (((uint32_t)(stArray[1])) <<  8)
                        | (((uint32_t)(stArray[0])) <<  0);
                    rstDstCell.ObjParam |= (((uint32_t)(stArray[3] & 0XFF)) << 8);
                    rstDstCell.ObjParam |= (((uint32_t)(stArray[4] & 0X03)) << 6);
                }else{
                    rstDstCell.Obj[0].Param = 0;
                    rstDstCell.ObjParam &= 0XFFFF0000;
                }
            }

            // cell::obj[1]
            {
                auto stArray = Object(nX, nY, 1).MakeArray();
                if(stArray[4] & 0X80){
                    rstDstCell.Obj[1].Param = 0X80000000
                        | (((uint32_t)(stArray[2])) << 16)
                        | (((uint32_t)(stArray[1])) <<  8)
                        | (((uint32_t)(stArray[0])) <<  0);
                    rstDstCell.ObjParam |= (((uint32_t)(stArray[3] & 0XFF)) << 24);
                    rstDstCell.ObjParam |= (((uint32_t)(stArray[4] & 0X03)) << 22);
                }else{
                    rstDstCell.Obj[1].Param = 0;
                    rstDstCell.ObjParam &= 0X0000FFFF;
                }
            }
        }
    }

    return stMapData.Save(szFullName) ? false : true;
}
