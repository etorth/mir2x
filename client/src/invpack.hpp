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
#include "serdesmsg.hpp"

class InvPack
{
    private:
        // TODO: Leave it as a variable, not using SYS_INVGRIDCW directly
        //       Later I may change the texture width of inventory board to make it bigger
        const size_t m_w;

    private:
        size_t m_gold = 0;

    private:
        SDItem m_grabbedItem;
        std::vector<PackBin> m_packBinList;

    public:
        InvPack(size_t argW = SYS_INVGRIDCW)
            : m_w(argW)
        {}

    public:
        const auto &getGrabbedItem() const
        {
            return m_grabbedItem;
        }

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
        void add(SDItem);
        void add(SDItem, int, int);

    public:
        int update(SDItem);

    public:
        size_t remove(uint32_t, uint32_t, size_t);

    public:
        size_t remove(const SDItem &item)
        {
            return remove(item.itemID, item.seqID, item.count);
        }

    public:
        static PackBin makePackBin(SDItem);
        static std::tuple<int, int> getPackBinSize(uint32_t);

    public:
        void setGrabbedItem(SDItem);

    public:
        void setGold(int);
        void addGold(int);

    public:
        size_t getGold() const;

    public:
        void setInventory(const SDInventory &);
};
