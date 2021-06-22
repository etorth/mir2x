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

#include "imgf.hpp"
#include "filesys.hpp"
#include "imagemapdb.hpp"
#include "mir2map.hpp"
#include "sysconst.hpp"
#include "mathf.hpp"
#include "editormap.hpp"
#include "mainwindow.hpp"
#include "progressbarwindow.hpp"

extern ImageMapDB *g_imageMapDB;
extern LayerBrowserWindow *g_layerBrowserWindow;

bool EditorMap::loadLayer(const char *fullName)
{
    m_srcType = LAYER;
    m_srcName = fullName;

    m_data.clear();
    m_data.load(fullName);

    m_selectBuf.resize(0);
    m_selectBuf.resize(m_data.w() * m_data.h() / 4);
    return valid();
}

bool EditorMap::loadMir2Map(const char *fullName)
{
    m_srcType = MIR2MAP;
    m_srcName = fullName;

    Mir2Map from(fullName);
    m_data.allocate(from.w(), from.h());

    for(int y = 0; y < to_d(from.h()); ++y){
        for(int x = 0; x < to_d(from.w()); ++x){
            if((x % 2) == 0 && (y % 2) == 0){
                from.convBlock(x, y, m_data.block(x, y), *g_imageMapDB);
            }
        }
    }

    m_selectBuf.resize(0);
    m_selectBuf.resize(m_data.w() * m_data.h() / 4);
    return valid();
}

bool EditorMap::loadMir2xMapData(const char *fullName)
{
    m_srcType = MIR2XMAPDATA;
    m_srcName = fullName;

    m_data.clear();
    m_data.load(fullName);

    m_selectBuf.resize(0);
    m_selectBuf.resize(m_data.w() * m_data.h() / 4);
    return valid();
}

void EditorMap::optimize()
{
    if(!valid()){
        return;
    }

    for(size_t y = 0; y < h(); ++y){
        for(size_t x = 0; x < w(); ++x){
            optimizeTile(x, y);
            optimizeCell(x, y);
        }
    }
}

void EditorMap::optimizeTile(int, int)
{
}

void EditorMap::optimizeCell(int, int)
{
}

bool EditorMap::saveMir2xMapData(const char *fullName)
{
    if(!valid()){
        return false;
    }

    m_data.save(fullName);
    return true;
}

bool EditorMap::exportOverview(std::function<void(uint32_t, int, int, bool)> fnExportOverview, std::atomic<int> *percent) const
{
    if(!valid()){
        return false;
    }

    const auto totalCount = w() * h() * 5;
    const auto fnReportPercent= [percent, totalCount](size_t count)
    {
        if(percent){
            percent->store(std::min<int>(to_d(count * 100 / totalCount), 100));
        }
    };


    int doneCount = 0;
    for(size_t x = 0; x < w(); ++x){
        for(size_t y = 0; y < h(); ++y){
            fnReportPercent(doneCount++);
            if(true
                    && !(x % 2)
                    && !(y % 2)){
                if(m_data.tile(x, y).valid){
                    fnExportOverview(m_data.tile(x, y).texID, x, y, false);
                }
            }
        }
    }

    for(const auto depth: {OBJD_GROUND, OBJD_OVERGROUND0, OBJD_OVERGROUND1, OBJD_SKY}){
        for(size_t x = 0; x < w(); ++x){
            for(size_t y = 0; y < h(); ++y){
                fnReportPercent(doneCount++);
                for(const int objIndex: {0, 1}){
                    const auto &obj = m_data.cell(x, y).obj[objIndex];
                    if(obj.valid && obj.depth == depth){
                        fnExportOverview(obj.texID, x, y, true);
                    }
                }
            }
        }
    }
    return true;
}

Mir2xMapData EditorMap::exportLayer() const
{
    if(!valid()){
        return {};
    }

    int x0 = INT_MAX;
    int y0 = INT_MAX;
    int x1 = INT_MIN;
    int y1 = INT_MIN;

    const auto fnExtendROI = [&x0, &y0, &x1, &y1, this](int x, int y)
    {
        if(validC(x, y)){
            x0 = std::min<int>(x0, x);
            y0 = std::min<int>(y0, y);
            x1 = std::max<int>(x1, x);
            y1 = std::max<int>(y1, y);
        }
    };

    Mir2xMapData data;
    data.allocate(w(), h());

    for(size_t x = 0; x < w(); ++x){
        for(size_t y = 0; y < h(); ++y){
            if((x % 2) == 0 && (y % 2) == 0){
                if(g_layerBrowserWindow->importTile()){
                    if(true
                            && tile(x, y).valid
                            && tileSelect(x, y).tile){
                        fnExtendROI(x, y);
                        data.tile(x, y) = tile(x, y);
                    }
                }
            }

            size_t dstObjIndex = 0;
            for(const int depth: {OBJD_GROUND, OBJD_OVERGROUND0, OBJD_OVERGROUND1, OBJD_SKY}){
                if(g_layerBrowserWindow->importObject(depth)){
                    for(const int objIndex: {0, 1}){
                        const auto &obj = cell(x, y).obj[objIndex];
                        if(true
                                && obj.valid
                                && obj.depth == depth
                                && cellSelect(x, y).obj[objIndex]){
                            if(const auto img = g_imageMapDB->setIndex(obj.texID)){
                                // objs are long bars
                                // need to extend to bigger area contains whole objs
                                const int objHCount = (img->height + SYS_MAPGRIDYP - 1) / SYS_MAPGRIDYP;
                                for(int iy = 0; iy < objHCount; ++iy){
                                    fnExtendROI(x, y - iy);
                                }
                                data.cell(x, y).obj[dstObjIndex++] = obj;
                            }
                        }
                    }
                }
            }
        }
    }

    if(validC(x0, y0) && validC(x1, y1)){
        x0 = (x0 / 2) * 2;
        y0 = (y0 / 2) * 2;

        const int cropW = ((x1 - x0 + 1 + 1) / 2) * 2;
        const int cropH = ((y1 - y0 + 1 + 1) / 2) * 2;
        return data.submap(x0, y0, cropW, cropH);
    }
    return {};
}

void EditorMap::allocate(size_t argW, size_t argH)
{
    fflassert(argW > 0 && argW % 2 == 0);
    fflassert(argH > 0 && argH % 2 == 0);

    m_selectBuf.resize(0);
    m_selectBuf.resize(argW * argH / 4);

    m_aniTimer.clear();
    m_data.allocate(argW, argH);
}

void EditorMap::clearSelect()
{
    std::fill(m_selectBuf.begin(), m_selectBuf.end(), BlockSelectConfig());
}
