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
        using ClickCBFunc = std::variant<std::nullptr_t,
                                         std::function<void(        )>,
                                         std::function<void(Widget *)>>;

    public:
        static void evalClickCBFunc(const ClickCBFunc &cbFunc, Widget *widget)
        {
            std::visit(VarDispatcher
            {
                [      ](const std::function<void(        )> &f){ if(f){ f(      ); }},
                [widget](const std::function<void(Widget *)> &f){ if(f){ f(widget); }},

                [](const auto &){},
            },
            cbFunc);
        }

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
            MenuItem::ItemSizeArgs itemSize {}; // margin not included

            Widget::WADPair         gfxWidget {};
            MenuItem::SubWidgetArgs subWidget {};

            Widget::VarBool showIndicator = false;
            Widget::VarBool showSeparator = false;
            Widget::VarBool expandOnHover = false;

            Widget::VarU32 bgColor = 0U;
            MenuItem::ClickCBFunc onClick = nullptr;

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
        auto gfxWidget(this auto &&self)
        {
            return self.m_gfxWidgetCrop.firstChild();
        }

        auto subWidget(this auto &&self)
        {
            return self.m_subWidget;
        }

    public:
        void drawDefault(Widget::ROIMap m) const override;
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override;
};
