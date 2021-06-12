/*
 * =====================================================================================
 *
 *       Filename: editormap.cpp
 *        Created: 02/08/2016 22:17:08
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

#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <functional>
#include <FL/fl_ask.H>

#include "pngf.hpp"
#include "filesys.hpp"
#include "mir2map.hpp"
#include "sysconst.hpp"
#include "mathf.hpp"
#include "editormap.hpp"
#include "mainwindow.hpp"
#include "progressbarwindow.hpp"

EditorMap::EditorMap()
    : m_W(0)
    , m_H(0)
    , m_Valid(false)
{
    std::memset(m_AniSaveTime,  0, sizeof(m_AniSaveTime));
    std::memset(m_AniTileFrame, 0, sizeof(m_AniTileFrame));
}

void EditorMap::DrawLight(int nX, int nY, int nW, int nH, std::function<void(int, int)> fnDrawLight)
{
    if(Valid()){
        for(int nTX = 0; nTX < nW; ++nTX){
            for(int nTY = 0; nTY < nH; ++nTY){
                if(ValidC(nTX + nX, nTY + nY)){
                    auto &rstLight = Light(nTX + nX, nTY + nY);
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

                if(!Tile(nX, nY).Valid){
                    continue;
                }

                auto nFileIndex  = (uint8_t )((Tile(nX, nY).Image & 0X00FF0000) >> 16);
                auto nImageIndex = to_u16((Tile(nX, nY).Image & 0X0000FFFF) >>  0);

                // provide cell-coordinates on map
                // fnDrawTile should convert it to drawarea pixel-coordinates
                fnDrawTile(nFileIndex, nImageIndex, nX, nY);
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
                        auto &rstObj = Object(nX, nY, nIndex);
                        if(true
                                && rstObj.Valid
                                && rstObj.Ground == bGround){

                            auto nFileIndex  = (uint8_t )((rstObj.Image & 0X00FF0000) >> 16);
                            auto nImageIndex = to_u16((rstObj.Image & 0X0000FFFF) >>  0);
                            auto nAnimated   = (bool    )((rstObj.Animated));
                            auto nAniType    = (uint8_t )((rstObj.AniType ));
                            auto nAniCount   = (uint8_t )((rstObj.AniCount));

                            if(nAnimated){
                                if(false
                                        || nFileIndex == 11
                                        || nFileIndex == 26
                                        || nFileIndex == 41
                                        || nFileIndex == 56
                                        || nFileIndex == 71){

                                    nImageIndex += to_u16(m_AniTileFrame[nAniType][nAniCount]);
                                }
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
        nX = (std::max<int>)(0, nX);
        nY = (std::max<int>)(0, nY);

        if(nX % 2){ nX--; nW++; }
        if(nY % 2){ nY--; nH++; }
        if(nW % 2){ nW++; }
        if(nH % 2){ nH++; }

        nW = (std::min<int>)(nW, W() - nX);
        nH = (std::min<int>)(nH, H() - nY);

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
    m_Mir2Map = std::make_unique<Mir2Map>(szFullName);
    m_Mir2xMapData.reset();

    MakeBuf(m_Mir2Map->w(), m_Mir2Map->h());
    InitBuf();

    m_Mir2Map.reset();
    m_Mir2xMapData.reset();

    return Valid();
}

bool EditorMap::LoadMir2xMapData(const char *szFullName)
{
    m_Mir2Map.reset();
    m_Mir2xMapData = std::make_unique<Mir2xMapData>();

    m_Mir2xMapData->load(szFullName);
    MakeBuf(m_Mir2xMapData->w(), m_Mir2xMapData->h());
    InitBuf();

    m_Mir2Map.reset();
    m_Mir2xMapData.reset();

    return Valid();
}

void EditorMap::Optimize()
{
    if(!Valid()){
        return;
    }

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

    if(m_Mir2Map){
        nW = m_Mir2Map->w();
        nH = m_Mir2Map->h();
    }
    else if(m_Mir2xMapData){
        nW = m_Mir2xMapData->w();
        nH = m_Mir2xMapData->h();
    }
    else{
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
            && nX / 2 < to_d(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < to_d(m_BlockBuf[nX / 2].size())){

        if(m_Mir2Map){
            extern ImageDB *g_ImageDB;
            if(m_Mir2Map->tileValid(nX, nY, *g_ImageDB)){
                Tile(nX, nY).Valid = true;
                Tile(nX, nY).Image = m_Mir2Map->tile(nX, nY) & 0X00FFFFFF;
            }
        }
        else if(m_Mir2xMapData){
            if(m_Mir2xMapData->tile(nX, nY).texIDValid){
                Tile(nX, nY).Valid = true;
                Tile(nX, nY).Image = m_Mir2xMapData->tile(nX, nY).texID;
            }
        }
    }
}

void EditorMap::SetBufGround(int nX, int nY)
{
    if(true
            && nX >= 0
            && nX / 2 < to_d(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < to_d(m_BlockBuf[nX / 2].size())){

        bool    bCanFly    = false;
        bool    bCanWalk   = false;
        uint8_t nAttribute = 0;

        if(m_Mir2Map){
            if(m_Mir2Map->groundValid(nX, nY)){
                bCanFly    = true;
                bCanWalk   = true;
                nAttribute = 0;
            }
        }else if(m_Mir2xMapData){
            bCanWalk   = m_Mir2xMapData->cell(nX, nY).canWalk;
            bCanFly    = m_Mir2xMapData->cell(nX, nY).canFly;
            nAttribute = m_Mir2xMapData->cell(nX, nY).landType;
        }

        Cell(nX, nY).CanFly    = bCanFly;
        Cell(nX, nY).CanWalk   = bCanWalk;
        Cell(nX, nY).LandType = nAttribute;
    }
}

void EditorMap::SetBufObj(int nX, int nY, int nIndex)
{
    if(true
            && nIndex >= 0
            && nIndex <= 1

            && nX >= 0
            && nX / 2 < to_d(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < to_d(m_BlockBuf[nX / 2].size())){

        uint32_t nObj       = 0;
        bool     bObjValid  = false;
        bool     bGroundObj = false;
        bool     bAniObj    = false;
        bool     bAlphaObj  = false;
        uint8_t  nAniType   = 0;
        uint8_t  nAniCount  = 0;

        if(m_Mir2Map){
            // mir2 map
            // animation info is in Mir2Map::Object() at higher 8 bits
            extern ImageDB *g_ImageDB;
            if(m_Mir2Map->objectValid(nX, nY, nIndex, *g_ImageDB)){

                // I checked the code
                // here the mir 3 checked CELLINFO::bFlag

                bObjValid = true;
                if(m_Mir2Map->groundObjectValid(nX, nY, nIndex, *g_ImageDB)){
                    bGroundObj = true;
                }
                if(m_Mir2Map->aniObjectValid(nX, nY, nIndex, *g_ImageDB)){
                    bAniObj = true;
                }

                // [31:24] : animation info
                // [23:16] : file index
                // [15: 0] : image index

                auto nObjDesc = m_Mir2Map->object(nX, nY, nIndex);
                nAniType  = (uint8_t )((nObjDesc & 0X70000000) >> (4 + 8 + 16));
                nAniCount = (uint8_t )((nObjDesc & 0X0F000000) >> (0 + 8 + 16));
                nObj      = to_u32((nObjDesc & 0X00FFFFFF));

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
        }
        else if(m_Mir2xMapData){
            const auto &obj = m_Mir2xMapData->cell(nX, nY).obj[nIndex];
            if(obj.texIDValid){
                bObjValid = true;
                nObj = obj.texID;
                bGroundObj = (obj.depthType == OBJD_GROUND);

                bAniObj = obj.animated;
                nAniType  = obj.tickType;
                nAniCount = obj.frameCount;
                bAlphaObj = obj.alpha;
            }
        }

        Object(nX, nY, nIndex).Valid    = bObjValid;
        Object(nX, nY, nIndex).Alpha    = bAlphaObj;
        Object(nX, nY, nIndex).Ground   = bGroundObj;
        Object(nX, nY, nIndex).Animated = bAniObj;
        Object(nX, nY, nIndex).AniType  = nAniType;
        Object(nX, nY, nIndex).AniCount = nAniCount;
        Object(nX, nY, nIndex).Image    = nObj;
    }
}

void EditorMap::SetBufLight(int nX, int nY)
{
    if(true
            && nX >= 0
            && nX / 2 < to_d(m_BlockBuf.size())

            && nY >= 0
            && nY / 2 < to_d(m_BlockBuf[nX / 2].size())){

        if(m_Mir2Map){
            if(m_Mir2Map->lightValid(nX, nY)){
                Light(nX, nY).Valid  = true;
                Light(nX, nY).Color  = 0;
                Light(nX, nY).Alpha  = 0;
                Light(nX, nY).Radius = 0;
            }
        }
        else if(m_Mir2xMapData){
            const auto &light = m_Mir2xMapData->cell(nX, nY).light;
            if(light.valid){
                Light(nX, nY).Valid  = true;
                Light(nX, nY).Color  = 0;
                Light(nX, nY).Alpha  = 0;
                Light(nX, nY).Radius = 0;
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
                    if(ValidC(nTX, nTY) && Cell(nTX, nTY).SelectConf.Ground){
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
                    Cell(nX, nY).SelectConf.Ground = false;
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
    stMapData.allocate(W(), H());

    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){

            // tile
            if(!(nX % 2) && !(nY % 2)){
                stMapData.tile(nX, nY).texIDValid = to_boolbit(Tile(nX, nY).Valid);
                stMapData.tile(nX, nY).texID      = Tile(nX, nY).Image & 0X00FFFFFF;
            }

            // cell
            auto &rstDstCell = stMapData.cell(nX, nY);

            // cell::land
            rstDstCell.canWalk  = Cell(nX, nY).CanWalk;
            rstDstCell.canFly   = Cell(nX, nY).CanFly;
            rstDstCell.landType = Cell(nX, nY).LandType;

            // cell::light
            rstDstCell.light.valid  = Light(nX, nY).Valid;
            rstDstCell.light.radius = Light(nX, nY).Radius;
            rstDstCell.light.alpha  = Light(nX, nY).Alpha;
            rstDstCell.light.color  = Light(nX, nY).Color;

            // cell::obj[0:1]
            for(int nIndex: {0, 1}){
                rstDstCell.obj[nIndex].texIDValid = Object(nX, nY, nIndex).Valid;
                rstDstCell.obj[nIndex].texID      = Object(nX, nY, nIndex).Image;
                rstDstCell.obj[nIndex].animated   = Object(nX, nY, nIndex).Animated;
                rstDstCell.obj[nIndex].tickType   = Object(nX, nY, nIndex).AniType;
                rstDstCell.obj[nIndex].frameCount = Object(nX, nY, nIndex).AniCount;
                rstDstCell.obj[nIndex].alpha      = Object(nX, nY, nIndex).Alpha;
                rstDstCell.obj[nIndex].depthType  = Object(nX, nY, nIndex).Ground ? OBJD_GROUND : OBJD_OVERGROUND0;
            }
        }
    }

    stMapData.save(szFullName);
    return true;
}

void EditorMap::ExportOverview(std::function<void(uint8_t, uint16_t, int, int, bool)> fnExportOverview)
{
    if(Valid()){
        int nCountAll = W() * H() * 3;
        auto fnUpdateProgressBar = [nCountAll, nLastPercent = 0](int nCurrCount) mutable
        {
            auto nPercent = std::lround(100.0 * nCurrCount / nCountAll);
            if(nPercent > nLastPercent){
                // 1. record percent
                nLastPercent = nPercent;

                // 2. update progress bar
                extern ProgressBarWindow *g_ProgressBarWindow;
                g_ProgressBarWindow->SetValue(nPercent);
                g_ProgressBarWindow->Redraw();
                g_ProgressBarWindow->ShowAll();
                Fl::check();
            }
        };

        int nCount = 0;
        for(int nX = 0; nX < W(); ++nX){
            for(int nY = 0; nY < H(); ++nY){
                fnUpdateProgressBar(nCount++);
                if(true
                        && !(nX % 2)
                        && !(nY % 2)){

                    auto &rstTile = Tile(nX, nY);
                    if(rstTile.Valid){
                        fnExportOverview((rstTile.Image & 0X00FF0000) >> 16, (rstTile.Image & 0X0000FFFF), nX, nY, false);
                    }
                }
            }
        }

        for(int nX = 0; nX < W(); ++nX){
            for(int nY = 0; nY < H(); ++nY){
                fnUpdateProgressBar(nCount++);
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    auto rstObj = Object(nX, nY, nIndex);
                    if(true
                            && rstObj.Valid
                            && rstObj.Ground){
                        fnExportOverview((rstObj.Image & 0X00FF0000) >> 16, (rstObj.Image & 0X0000FFFF), nX, nY, true);
                    }
                }
            }
        }

        for(int nX = 0; nX < W(); ++nX){
            for(int nY = 0; nY < H(); ++nY){
                fnUpdateProgressBar(nCount++);
                for(int nIndex = 0; nIndex < 2; ++nIndex){
                    auto rstObj = Object(nX, nY, nIndex);
                    if(true
                            &&  rstObj.Valid
                            && !rstObj.Ground){
                        fnExportOverview((rstObj.Image & 0X00FF0000) >> 16, (rstObj.Image & 0X0000FFFF), nX, nY, true);
                    }
                }
            }
        }

        extern ProgressBarWindow *g_ProgressBarWindow;
        g_ProgressBarWindow->HideAll();
    }
}

EditorMap *EditorMap::ExportLayer()
{
    if(true
            &&  Valid()
            && !(W() % 2)
            && !(H() % 2)){

        int nX0 =  W();
        int nY0 =  H();
        int nX1 = -1;
        int nY1 = -1;

        auto pEditorMap = new EditorMap();
        pEditorMap->Allocate(W(), H());

        auto fnExtend = [&nX0, &nY0, &nX1, &nY1](int nX, int nY)
        {
            nX0 = (std::min<int>)(nX0, nX);
            nY0 = (std::min<int>)(nY0, nY);
            nX1 = (std::max<int>)(nX1, nX);
            nY1 = (std::max<int>)(nY1, nY);
        };

        extern LayerBrowserWindow *g_LayerBrowserWindow;
        for(int nX = 0; nX < W(); ++nX){
            for(int nY = 0; nY < H(); ++nY){

                // 1. tile
                if(g_LayerBrowserWindow->ImportTile()){
                    if(true
                            && !(nX % 2)
                            && !(nY % 2)){
                        auto &rstTile = Tile(nX, nY);
                        if(true
                                && rstTile.Valid
                                && rstTile.SelectConf.Tile){
                            pEditorMap->Tile(nX, nY) = rstTile;
                            fnExtend(nX, nY);
                        }
                    }
                }

                // 2. object, two layers
                for(int bGroundObj = 0; bGroundObj < 2; ++bGroundObj){
                    if(g_LayerBrowserWindow->ImportObject((bool)(bGroundObj))){
                        for(int nIndex = 0; nIndex < 2; ++nIndex){
                            auto &rstCell = Cell(nX, nY);
                            if(rstCell.Obj[nIndex].Valid){
                                if(false
                                        || ( bGroundObj &&  rstCell.Obj[nIndex].Ground && rstCell.SelectConf.GroundObj)
                                        || (!bGroundObj && !rstCell.Obj[nIndex].Ground && rstCell.SelectConf.OverGroundObj)){

                                    for(int nValidSlotIndex = 0; nValidSlotIndex < 2; ++nValidSlotIndex){
                                        if(!pEditorMap->Object(nX, nY, nValidSlotIndex).Valid){
                                            pEditorMap->Object(nX, nY, nValidSlotIndex) = rstCell.Obj[nIndex];
                                            fnExtend(nX, nY);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // 3. light
            }
        }

        if(true
                && ValidC(nX0, nY0)
                && ValidC(nX1, nY1)){

            nX0 = (nX0 / 2) * 2;
            nY0 = (nY0 / 2) * 2;

            int nW = ((nX1 - nX0 + 1 + 1) / 2) * 2;
            int nH = ((nY1 - nY0 + 1 + 1) / 2) * 2;

            if(pEditorMap->Resize(nX0, nY0, nW, nH, 0, 0, nW, nH)){
                return pEditorMap;
            }
        }

        delete pEditorMap; pEditorMap = nullptr;
    }

    return nullptr;
}

bool EditorMap::Allocate(int nW, int nH)
{
    if(true
            && !(nW % 2)
            && !(nH % 2)){

        m_W     = nW;
        m_H     = nH;
        m_Valid = true;

        std::memset(m_AniSaveTime,  0, sizeof(m_AniSaveTime));
        std::memset(m_AniTileFrame, 0, sizeof(m_AniTileFrame));

        m_Mir2Map      = nullptr;
        m_Mir2xMapData = nullptr;

        m_BlockBuf.resize(nW / 2);
        for(auto &rstBuf: m_BlockBuf){
            rstBuf.resize(nH / 2);
            std::memset(&(rstBuf[0]), 0, rstBuf.size() * sizeof(rstBuf[0]));
        }
        return true;
    }
    return false;
}
