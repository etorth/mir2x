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
        // TODO: Leave it as a variable, not using SYS_INVGRIDCW directly
        //       Later I may change the texture width of inventory board to make it bigger
        const size_t m_w;

    private:
        std::vector<PackBin> m_packBinList;

    public:
        InvPack(size_t argW = SYS_INVGRIDCW)
            : m_w(argW)
            , m_packBinList()
        {}

    public:
        const auto &getPackBinList() const
        {
            return m_packBinList;
        }

    public:
        size_t w() const
        {
            return m_w;
        }

    public:
        void repack()
        {
            Pack2D(w()).pack(m_packBinList);
        }

    public:
        void add(uint32_t, size_t);
        void add(uint32_t, size_t, int, int);

    public:
        void addBin(const PackBin &bin)
        {
            add(bin.id, bin.count, bin.x, bin.y);
        }

    public:
        size_t remove(uint32_t, size_t, int, int);

    public:
        size_t removeBin(const PackBin &bin)
        {
            return remove(bin.id, bin.count, bin.x, bin.y);
        }

    private:
        static PackBin makePackBin(uint32_t, size_t);
};
