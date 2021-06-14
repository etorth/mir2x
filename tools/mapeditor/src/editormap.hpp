/*
 * =====================================================================================
 *
 *       Filename: editormap.hpp
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
#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <functional>

#include "mir2map.hpp"
#include "sysconst.hpp"
#include "landtype.hpp"
#include "fflerror.hpp"
#include "wilanitimer.hpp"
#include "mir2xmapdata.hpp"

class EditorMap
{
    public:
        enum
        {
            NONE = 0,
            MIR2MAP,
            MIR2XMAPDATA,
            LAYER,
        };

    private:
        struct TileSelectConfig
        {
            bool tile = false;
        };

        struct CellSelectConfig
        {
            bool ground = false;
            bool attribute = false;

            bool obj[2]
            {
                false,
                false,
            };
        };

        struct BlockSelectConfig
        {
            TileSelectConfig tile;
            CellSelectConfig cell[4];
        };

    private:
        WilAniTimer m_aniTimer;

    private:
        Mir2xMapData m_data;

    private:
        std::vector<BlockSelectConfig> m_selectBuf;

    private:
        int m_srcType = NONE;
        std::string m_srcName;

    public:
        EditorMap() = default;

    public:
        int srcType() const
        {
            return m_srcType;
        }

        const std::string &srcName() const
        {
            return m_srcName;
        }

    public:
        bool loadLayer(const char *);
        bool loadMir2Map(const char *);
        bool loadMir2xMapData(const char *);

    public:
        bool valid() const
        {
            return w() > 0 && h() > 0;
        }

        bool validC(int nX, int nY) const
        {
            return nX >= 0 && nX < w() && nY >= 0 && nY < h();
        }

        bool validP(int nX, int nY) const
        {
            return nX >= 0 && nX < SYS_MAPGRIDXP * w() && nY >= 0 && nY < SYS_MAPGRIDYP * h();
        }

    public:
        int w() const
        {
            return m_data.w();
        }

        int h() const
        {
            return m_data.h();
        }

    public:
        auto &block(int x, int y)
        {
            return m_data.block(x, y);
        }

        auto &tile(int x, int y)
        {
            return m_data.tile(x, y);
        }

        auto &cell(int x, int y)
        {
            return m_data.cell(x, y);
        }

    public:
        const auto &block(int x, int y) const
        {
            return m_data.block(x, y);
        }

        const auto &tile(int x, int y) const
        {
            return m_data.tile(x, y);
        }

        const auto &cell(int x, int y) const
        {
            return m_data.cell(x, y);
        }

    public:
        auto &blockSelect(int x, int y)
        {
            fflassert(validC(x, y));
            return m_selectBuf.at(x / 2 + (y / 2) * (m_data.w() / 2));
        }

        const auto &blockSelect(int x, int y) const
        {
            fflassert(validC(x, y));
            return m_selectBuf.at(x / 2 + (y / 2) * (m_data.w() / 2));
        }

        auto &tileSelect(int x, int y)
        {
            return blockSelect(x, y).tile;
        }

        const auto &tileSelect(int x, int y) const
        {
            return blockSelect(x, y).tile;
        }

        auto &cellSelect(int x, int y)
        {
            return blockSelect(x, y).cell[(y % 2) * 2 + (x % 2)];
        }

        const auto &cellSelect(int x, int y) const
        {
            return blockSelect(x, y).cell[(y % 2) * 2 + (x % 2)];
        }

    public:
        void clear()
        {
            m_data.clear();
            m_selectBuf.clear();
        }

    public:
        bool exportOverview(std::function<void(uint32_t, int, int, bool)>, std::atomic<int> *) const;

    public:
        void drawLight(int, int, int, int, std::function<void(int, int)>);

    public:
        void updateFrame(int);

    public:
        void allocate(size_t, size_t);
        bool saveMir2xMapData(const char *);

    public:
        void optimize();
        void optimizeTile(int, int);
        void optimizeCell(int, int);

    public:
        Mir2xMapData exportLayer() const;
};
