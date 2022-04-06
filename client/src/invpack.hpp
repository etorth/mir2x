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

#pragma once
#include <vector>
#include "pack2d.hpp"
#include "sysconst.hpp"
#include "serdesmsg.hpp"

class InvPack
{
    private:
        // TODO: Leave it as a variable, not using SYS_INVGRIDGW directly
        //       Later I may change the texture width of inventory board to make it bigger
        const size_t m_w;

    private:
        size_t m_gold = 0;

    private:
        int m_repackIndex = 0;

    private:
        SDItem m_grabbedItem;
        std::vector<PackBin> m_packBinList;

    public:
        InvPack(size_t argW = SYS_INVGRIDGW)
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
        int getWeight() const;

    public:
        size_t w() const
        {
            return m_w;
        }

    public:
        void repack()
        {
            Pack2D::pack(m_packBinList, w(), m_repackIndex++);
        }

    public:
        void add(SDItem, bool playSound = true);
        void add(SDItem, int, int, bool playSound = true);

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

    public:
        size_t getGold() const;

    public:
        void setInventory(const SDInventory &);

    public:
        static void playItemSoundEffect(uint32_t, bool consume = false);
};
