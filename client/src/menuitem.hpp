#pragma once
#include "menu.hpp"
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
            Menu::ItemSize  itemSize {}; // margin not included

            Widget::WADPair         gfxWidget {};
            MenuItem::SubWidgetArgs subWidget {};

            Widget::VarBool showIndicator = false;
            Widget::VarBool showSeparator = false;
            Widget::VarBool expandOnHover = false;

            Widget::VarU32 bgColor = 0U;
            Menu::ClickCBFunc onClick = nullptr;

            Widget::WADPair parent {};
        };

    private:
        Widget       *m_subWidget;
        TrigfxButton *m_gfxButton;

    private:
        Menu::ItemSize m_itemSize;

    private:
        Widget m_gfxWidgetCrop;
        GfxShapeBoard m_indicator; // a small triangle indicates submenu exists

    private:
        ItemPair m_canvas;
        MarginWrapper m_wrapper;

    public:
        MenuItem(MenuItem::InitArgs);

    public:
        auto gfxWidget(this auto &&self)
        {
            return self.m_gfxWidgetCrop.firstChild();
        }

        auto subWidget(this auto &&self)
        {
            return self.m_subWidget;
        }

    public:
        void setItemWidth (Widget::VarSizeOpt argItemWidth ){ m_itemSize.w = std::move(argItemWidth ); }
        void setItemHeight(Widget::VarSizeOpt argItemHeight){ m_itemSize.h = std::move(argItemHeight); }

    public:
        int getItemWidth () const { return m_gfxWidgetCrop.w(); }
        int getItemHeight() const { return m_gfxWidgetCrop.h(); }

    public:
        void drawDefault(Widget::ROIMap m) const override;
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override;
};
