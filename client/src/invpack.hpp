/*
 * =====================================================================================
 *
 *       Filename: invpack.hpp
 *        Created: 11/11/2017 00:55:42
 *    Description: class private to MyHero
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

#include <vector>
#include "pack2d.hpp"
#include "sysconst.hpp"

class InvPack
{
    private:
        const size_t m_W;

    private:
        std::vector<PackBin> m_PackBinList;

    public:
        InvPack(size_t nW = SYS_INVGRIDW)
            : m_W(nW)
            , m_PackBinList()
        {}

    public:
        const std::vector<PackBin> &GetPackBinList() const
        {
            return m_PackBinList;
        }

    public:
        size_t W() const
        {
            return m_W;
        }

    public:
        bool Repack();

    public:
        bool Add(uint32_t);
        bool Remove(uint32_t, int, int);

    private:
        static PackBin MakePackBin(uint32_t);
};
