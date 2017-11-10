/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.hpp
 *        Created: 10/08/2017 19:06:52
 *  Last Modified: 11/10/2017 09:52:05
 *
 *    Description: inventory class
 *                 server won't store the item place
 *                 it only stores the item weight in total
 *
 *                 client can use different strategy to store them
 *                 player has max weight to hold only
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
#include "widget.hpp"
#include "pack2d.hpp"
#include "labelboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class InventoryBoard: public Widget
{
    private:
        struct ItemLocation
        {
            uint32_t ItemID;

            int X;
            int Y;

            ItemLocation(uint32_t nItemID = 0, int nX = -1, int nY = -1)
                : ItemID(nItemID)
                , X(nX)
                , Y(nY)
            {}
        };

    private:
        Pack2D m_Pack2D;

    private:
        std::vector<PackBin> m_PackBinList;

    private:
        LabelBoard m_GoldBoard;

    private:
        TritexButton m_CloseButton;

    private:
        ProcessRun *m_ProcessRun;

    public:
        InventoryBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void DrawEx(int, int, int, int, int, int);

    private:
        void DrawItem(int, int, uint32_t, int, int, int, int);

    public:
        bool ProcessEvent(const SDL_Event &, bool *);

    public:
        bool Repack();
        bool Add(uint32_t);
        bool Remove(uint32_t, int, int);
};
