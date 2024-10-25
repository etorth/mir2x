#pragma once
#include <initializer_list>
#include <functional>
#include "widget.hpp"

class MenuBoard: public Widget
{
    private:
        const WidgetVarSize m_varW;

    private:
        const int m_itemSpace;
        const int m_seperatorSpace;

    private:
        std::function<void(Widget *)> m_onClickMenu;

    private:
        const std::array<int, 4> m_margin;

    public:
        MenuBoard(dir8_t,
                int,
                int,

                WidgetVarSize,

                int,
                int,

                std::initializer_list<std::pair<Widget *, bool>>,
                std::function<void(Widget *)> = nullptr,

                std::array<int, 4> = {},

                Widget * = nullptr,
                bool     = false);

    public:
        void addChild(Widget *, bool) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        static Widget *getSeparator();
};
