/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.hpp
 *        Created: 08/31/2015 18:26:57
 *    Description: class to record data for mir2x map
 *                 this class won't define operation over the data
 *
 *                 previously I was using grid compression
 *                 but I decide to disable it since I found I can use zip to compress
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
#include <array>
#include <vector>
#include <fstream>
#include <cstdint>
#include <functional>
#include "strf.hpp"
#include "totype.hpp"
#include "cerealf.hpp"
#include "landtype.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"

enum ObjDepthType: uint32_t
{
    OBJD_GROUND      = 0,
    OBJD_OVERGROUND0 = 1,    // over ground but lower  than player
    OBJD_OVERGROUND1 = 2,    // over ground but higher than player
    OBJD_SKY         = 3,
};

class Mir2xMapData final
{
    public:
#pragma pack(push, 1)
        struct TILE
        {
            uint32_t texIDValid :  1 {0};
            uint32_t texID      : 24 {0};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint32_t sd_texIDValid = texIDValid;
                uint32_t sd_texID = texID;

                ar(sd_texIDValid, sd_texID);

                texIDValid = sd_texIDValid;
                texID = sd_texID;
            }
        };

        struct OBJ
        {
            uint32_t texIDValid :  1 {0};
            uint32_t texID      : 24 {0};

            uint32_t animated   :  1 {0};
            uint32_t tickType   :  3 {0};
            uint32_t frameCount :  4 {0};
            uint32_t alpha      :  1 {0};
            uint32_t depthType  :  2 {2};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint32_t sd_texIDValid = texIDValid;
                uint32_t sd_texID      = texID;
                uint32_t sd_animated   = animated;
                uint32_t sd_tickType   = tickType;
                uint32_t sd_frameCount = frameCount;
                uint32_t sd_alpha      = alpha;
                uint32_t sd_depthType  = depthType;

                ar(sd_texIDValid, sd_texID, sd_animated, sd_tickType, sd_frameCount, sd_alpha, sd_depthType);

                texIDValid = sd_texIDValid;
                texID      = sd_texID;
                animated   = sd_animated;
                tickType   = sd_tickType;
                frameCount = sd_frameCount;
                alpha      = sd_alpha;
                depthType  = sd_depthType;
            }
        };

        struct LIGHT
        {
            uint8_t valid  : 1 {0};
            uint8_t radius : 2 {0};
            uint8_t alpha  : 2 {0};
            uint8_t color  : 3 {0};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint8_t sd_valid  = valid;
                uint8_t sd_radius = radius;
                uint8_t sd_alpha  = alpha;
                uint8_t sd_color  = color;

                ar(sd_valid, sd_radius, sd_alpha, sd_color);

                valid  = sd_valid;
                radius = sd_radius;
                alpha  = sd_alpha;
                color  = sd_color;
            }
        };

        // every obj can have 4 types
        // but every cell only contains 2 cells

        struct CELL
        {
            uint8_t canWalk  : 1 {0};
            uint8_t canFly   : 1 {0};
            uint8_t landType : 6 {0};

            OBJ obj[2];
            LIGHT light;

            template<typename Archive> void serialize(Archive &ar)
            {
                uint8_t sd_canWalk  = canWalk;
                uint8_t sd_canFly   = canFly;
                uint8_t sd_landType = landType;

                ar(sd_canWalk, sd_canFly, sd_landType, obj[0], obj[1], light);

                canWalk  = sd_canWalk;
                canFly   = sd_canFly;
                landType = sd_landType;
            }

            bool canThrough() const
            {
                return canWalk || canFly;
            }
        };

        struct BLOCK
        {
            TILE tile;
            CELL cell[4];

            template<typename Archive> void serialize(Archive &ar)
            {
                ar(tile, cell[0], cell[1], cell[2], cell[3]);
            }
        };
#pragma pack(pop)

    private:
        int m_w = 0;
        int m_h = 0;

    private:
        std::vector<BLOCK> m_data;

    public:
        Mir2xMapData() = default;

    public:
        void allocate(int argW, int argH)
        {
            fflassert((argW > 0) && (argW % 2 == 0));
            fflassert((argH > 0) && (argH % 2 == 0));

            m_data.clear();
            m_data.resize(argW * argH / 4);
        }

    public:
        template<typename Archive> void serialize(Archive &ar)
        {
            ar(m_w, m_h, m_data);
        }

    public:
        int w() const
        {
            return m_w;
        }

        int h() const
        {
            return m_h;
        }

    public:
        auto &block(int argX, int argY)
        {
            return m_data[argX / 2 + (argY / 2) * (m_w / 2)];
        }

        auto &tile(int argX, int argY)
        {
            return block(argX, argY).tile;
        }

        auto &cell(int argX, int argY)
        {
            return block(argX, argY).cell[(argY % 2) * 2 + (argX % 2)];
        }

    public:
        const auto &block(int argX, int argY) const
        {
            return m_data[argX / 2 + (argY / 2) * (m_w / 2)];
        }

        const auto &tile(int argX, int argY) const
        {
            return block(argX, argY).tile;
        }

        const auto &cell(int argX, int argY) const
        {
            return block(argX, argY).cell[(argY % 2) * 2 + (argX % 2)];
        }

    public:
        void loadData(const void *data, size_t size)
        {
            *this = cerealf::deserialize<Mir2xMapData>(data, size);
        }

        void load(const std::string &fileName)
        {
            std::stringstream ss;
            ss << std::ifstream(fileName, std::ifstream::binary).rdbuf();
            *this = cerealf::deserialize<Mir2xMapData>(ss.str());
        }

        void save(const std::string &fileName) const
        {
            std::ofstream f(fileName, std::ofstream::binary);
            const auto s = cerealf::serialize(*this);
            f.write(s.data(), s.size());
        }

    public:
        bool validC(int argX, int argY) const
        {
            return argX >= 0 && argX < m_w && argY >= 0 && argY < m_h;
        }

        bool validP(int argX, int argY) const
        {
            return argX >= 0 && argX < m_w * SYS_MAPGRIDXP && argY >= 0 && argY < m_h * SYS_MAPGRIDYP;
        }
};
