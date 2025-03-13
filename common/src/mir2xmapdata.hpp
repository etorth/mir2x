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
            uint32_t valid :  1 {0};
            uint32_t texID : 24 {0};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint32_t sd_valid = valid;
                uint32_t sd_texID = texID;

                ar(sd_valid, sd_texID);

                valid = sd_valid;
                texID = sd_texID;
            }
        };

        struct OBJECT
        {
            uint32_t valid :  1 {0};
            uint32_t texID : 24 {0};
            uint32_t depth :  2 {2};

            uint32_t animated   : 1 {0};
            uint32_t alpha      : 1 {0};
            uint32_t tickType   : 3 {0};
            uint32_t frameCount : 4 {0};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint32_t sd_valid      = valid;
                uint32_t sd_texID      = texID;
                uint32_t sd_depth      = depth;
                uint32_t sd_animated   = animated;
                uint32_t sd_alpha      = alpha;
                uint32_t sd_tickType   = tickType;
                uint32_t sd_frameCount = frameCount;

                ar(sd_valid, sd_texID, sd_depth, sd_animated, sd_alpha, sd_tickType, sd_frameCount);

                valid      = sd_valid;
                texID      = sd_texID;
                depth      = sd_depth;
                animated   = sd_animated;
                alpha      = sd_alpha;
                tickType   = sd_tickType;
                frameCount = sd_frameCount;
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

        struct LAND
        {
            uint8_t type    : 6 {0};
            uint8_t canFly  : 1 {0};
            uint8_t canWalk : 1 {0};

            template<typename Archive> void serialize(Archive &ar)
            {
                uint8_t sd_type    = type;
                uint8_t sd_canFly  = canFly;
                uint8_t sd_canWalk = canWalk;

                ar(sd_type, sd_canFly, sd_canWalk);

                type    = sd_type;
                canFly  = sd_canFly;
                canWalk = sd_canWalk;
            }

            bool canThrough() const
            {
                return canWalk || canFly;
            }

            bool canMine() const
            {
                return true;
            }
        };

        // every obj can have 4 types
        // but every cell only contains 2 cells

        struct CELL
        {
            LAND land;
            LIGHT light;
            OBJECT obj[2];

            template<typename Archive> void serialize(Archive &ar)
            {
                ar(land, light, obj[0], obj[1]);
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
        struct InnMapData
        {
            size_t w = 0;
            size_t h = 0;
            std::vector<BLOCK> blockList;

            template<typename Archive> void serialize(Archive &ar)
            {
                ar(w, h, blockList);
            }

            void clear()
            {
                w = 0;
                h = 0;
                blockList.clear();
            }

            void allocate(size_t argW, size_t argH)
            {
                fflassert((argW > 0) && (argW % 2 == 0));
                fflassert((argH > 0) && (argH % 2 == 0));

                w = argW;
                h = argH;
                blockList.resize(argW * argH / 4);
            }
        };

    private:
        InnMapData m_data;

    public:
        Mir2xMapData() = default;

    public:
        Mir2xMapData(const std::string &fileName)
            : Mir2xMapData()
        {
            load(fileName);
        }

        Mir2xMapData(const void *data, size_t size)
            : Mir2xMapData()
        {
            loadData(data, size);
        }

    public:
        void allocate(int argW, int argH)
        {
            m_data.clear();
            m_data.allocate(argW, argH);
        }

        void clear()
        {
            m_data.clear();
        }

    public:
        size_t w() const
        {
            return m_data.w;
        }

        size_t h() const
        {
            return m_data.h;
        }

    public:
        auto &block(int argX, int argY)
        {
            fflassert(validC(argX, argY));
            return m_data.blockList[argX / 2 + (argY / 2) * (m_data.w / 2)];
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
            fflassert(validC(argX, argY));
            return m_data.blockList[argX / 2 + (argY / 2) * (m_data.w / 2)];
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
            m_data = cerealf::deserialize<Mir2xMapData::InnMapData>(data, size);
        }

        void load(const std::string &fileName)
        {
            std::stringstream ss;
            ss << std::ifstream(fileName, std::ifstream::binary).rdbuf();
            m_data = cerealf::deserialize<Mir2xMapData::InnMapData>(ss.str());
        }

        void save(const std::string &fileName) const
        {
            std::ofstream f(fileName, std::ofstream::binary);
            const auto s = cerealf::serialize(m_data);
            f.write(s.data(), s.size());
        }

    public:
        bool valid() const
        {
            return w() > 0 && h() > 0;
        }

        bool validC(int argX, int argY) const
        {
            return argX >= 0 && argX < to_d(w()) && argY >= 0 && argY < to_d(h());
        }

        bool validP(int argX, int argY) const
        {
            return argX >= 0 && argX < to_d(w() * SYS_MAPGRIDXP) && argY >= 0 && argY < to_d(h() * SYS_MAPGRIDYP);
        }

    public:
        Mir2xMapData submap(size_t argX, size_t argY, size_t argW, size_t argH) const
        {
            fflassert(argX % 2 == 0);
            fflassert(argY % 2 == 0);

            fflassert(argW > 0 && argW % 2 == 0);
            fflassert(argH > 0 && argH % 2 == 0);

            fflassert(argX + argW <= w());
            fflassert(argY + argH <= h());

            Mir2xMapData sub;
            sub.allocate(argW, argH);

            for(size_t iy = 0; iy < argH; ++iy){
                for(size_t ix = 0; ix < argW; ++ix){
                    if((ix % 2) || (iy % 2)){
                        continue;
                    }
                    sub.block(ix, iy) = block(argX + ix, argY + iy);
                }
            }
            return sub;
        }
};
