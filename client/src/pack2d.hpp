/*
 * =====================================================================================
 *
 *       Filename: pack2d.hpp
 *        Created: 11/07/2017 23:30:43
 *  Last Modified: 11/11/2017 23:21:33
 *
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
#include "sysconst.hpp"
#include "condcheck.hpp"

struct PackBin
{
    uint32_t ID;

    int X;
    int Y;
    int W;
    int H;

    PackBin(uint32_t nID = 0, int nX = -1, int nY = -1, int nW = -1, int nH = -1)
        : ID(nID)
        , X(nX)
        , Y(nY)
        , W(nW)
        , H(nH)
    {}

    operator bool () const
    {
        return ID != 0;
    }
};

class Pack2D
{
    private:
        const size_t m_W;

    private:
        std::vector<int> m_PackMap;

    public:
        size_t H()
        {
            Shrink();
            return m_PackMap.size();
        }

        size_t W() const
        {
            return m_W;
        }

    public:
        Pack2D(size_t nW)
            : m_W(nW)
        {
            condcheck(m_W < sizeof(decltype(m_PackMap)::value_type) * 8);
        }

    public:
        int Occupied(int, int);
        int Occupied(int, int, int, int, bool = true);

        int Occupy(int, int, bool);
        int Occupy(int, int, int, int, bool);

    public:
        int Pack(std::vector<PackBin> *);

    private:
        int FindRoom(PackBin *);

    public:
        int Put(int, int, int, int);

    public:
        int Add(PackBin *, size_t = 1);

    public:
        int Remove(const PackBin &);

    private:
        void Shrink()
        {
            while(!m_PackMap.empty()){
                if(m_PackMap.back()){
                    break;
                }else{
                    m_PackMap.pop_back();
                }
            }
        }
};
