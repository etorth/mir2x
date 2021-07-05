/*
 * =====================================================================================
 *
 *       Filename: pack2d.hpp
 *        Created: 11/07/2017 23:30:43
 *    Description: maintain a 2D mask to take care of valid grids
 *                 if one grid is taken then mask it as 1
 *
 *                 so for 2X2 masked grid
 *                 we can't tell if it's a 2x2 obj or 4 * 1x1 objs
 *
 *                 need more advanced 2d packing algorithm
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
#include <tuple>
#include <cstdint>
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "serdesmsg.hpp"

struct PackBin
{
    SDItem item;

    int x = -1;
    int y = -1;
    int w =  1;
    int h =  1;

    operator bool () const
    {
        return item.itemID != 0;
    }

    static PackBin makePackBin(SDItem);
    static std::tuple<int, int> getPackBinSize(const SDItem &);
};

class Pack2D
{
    private:
        const size_t m_w;

    private:
        std::vector<uint64_t> m_packMap;

    public:
        size_t h() const
        {
            return m_packMap.size();
        }

        size_t w() const
        {
            return m_w;
        }

    public:
        Pack2D(size_t width)
            : m_w(width)
        {
            if(m_w > sizeof(decltype(m_packMap)::value_type) * 8){
                throw fflerror("invalid Pack2D width: %zu", m_w);
            }
        }

    public:
        bool occupied(int, int) const;
        bool occupied(int, int, int, int, bool = true) const;

    public:
        void occupy(int, int, bool);
        void occupy(int, int, int, int, bool);

    public:
        void pack(std::vector<PackBin> &, int);

    private:
        void findRoom(PackBin *);

    public:
        void add(PackBin *, size_t = 1);

    public:
        bool put(int x, int y, int argW, int argH)
        {
            if(!occupied(x, y, argW, argH, true)){
                occupy  (x, y, argW, argH, true);
                return true;
            }
            return false;
        }

        bool remove(const PackBin &bin)
        {
            if(occupied(bin.x, bin.y, bin.w, bin.h, false)){
                occupy (bin.x, bin.y, bin.w, bin.h, false);
                return true;
            }
            return false;
        }

    private:
        void shrink()
        {
            while(!(m_packMap.empty() || m_packMap.back())){
                m_packMap.pop_back();
            }
        }

        bool validC(int x, int y, int argW, int argH) const
        {
            return true
                && x >= 0
                && y >= 0
                && argW > 0
                && argH > 0
                && x + argW <= to_d(w());
        }

        void validCEx(int x, int y, int argW, int argH) const
        {
            if(!validC(x, y, argW, argH)){
                throw fflerror("invalid arguments: x = %d, y = %d, w = %d, h = %d, pack2D::w = %zu", x, y, argW, argH, w());
            }
        }
};
