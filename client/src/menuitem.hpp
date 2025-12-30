#pragma once
#include "widget.hpp"
#include "itempair.hpp"
#include "trigfxbutton.hpp"
#include "marginwrapper.hpp"
#include "gfxshapeboard.hpp"

class MenuItem: public Widget
{
    public:
        constexpr static int INDICATOR_W = 9;
        constexpr static int INDICATOR_H = 5;

    public:
        struct ItemSizeArgs final
        {
            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;
        };

    private:
        struct SubWidgetArgs final
        {
            dir8_t dir = DIR_UPRIGHT;

            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarMargin margin {};
            MenuItem::ItemSizeArgs itemSize {};

            Widget::WADPair         gfxWidget {};
            MenuItem::SubWidgetArgs subWidget {};

            Widget::VarBool showIndicator = false;
            Widget::VarBool showSeparator = false;
            Widget::VarBool expandOnHover = false;

            Widget::WADPair parent {};
        };

    private:
        Widget       *m_subWidget;
        TrigfxButton *m_gfxButton;

    private:
        Widget m_gfxWidgetCrop;
        GfxShapeBoard m_indicator; // a small triangle indicates submenu exists

    private:
        ItemPair m_canvas;
        MarginWrapper m_wrapper;

    public:
        MenuItem(MenuItem::InitArgs);

    public:
        void drawDefault(Widget::ROIMap m) const override;
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override;
};
