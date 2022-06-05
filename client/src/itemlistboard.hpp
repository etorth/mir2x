#pragma once
#include <cstdint>
#include "widget.hpp"
#include "tritexbutton.hpp"

class ItemListBoard: public Widget
{
    private:
        constexpr static int m_boxW = 38;
        constexpr static int m_boxH = 38;

        constexpr static int m_startX = 23;
        constexpr static int m_startY = 41;

        constexpr static int m_gfxSrcX = 290; // ItemListBoard on 0X08000001
        constexpr static int m_gfxSrcY =   0;
        constexpr static int m_gfxSrcW = 198;
        constexpr static int m_gfxSrcH = 204;

    protected:
        size_t m_page = 0;
        std::optional<size_t> m_selectedPageGrid;

    private:
        TritexButton m_leftButton;
        TritexButton m_selectButton;
        TritexButton m_rightButton;
        TritexButton m_closeButton;

    public:
        ItemListBoard(int, int, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    protected:
        std::optional<size_t> getPageGrid() const;

    protected:
        static std::tuple<int, int, int, int> getPageGridPLoc(size_t gridX, size_t gridY)
        {
            fflassert(gridX < 4);
            fflassert(gridY < 3);

            return
            {
                m_startX + m_boxW * gridX,
                m_startY + m_boxH * gridY,
                m_boxW,
                m_boxH,
            };
        }

    private:
        void drawGridHoverLayout(size_t) const;

    public:
        virtual size_t itemCount() const = 0;

    public:
        size_t pageCount() const
        {
            return (itemCount() + 11) / 12;
        }

    public:
        virtual const SDItem &getItem(size_t) const = 0;

    public:
        virtual std::u8string getGridHeader     (size_t) const = 0;
        virtual std::u8string getGridHoverLayout(size_t) const = 0;

    private:
        virtual void onSelect() {}
        virtual void onClose () {}
};
