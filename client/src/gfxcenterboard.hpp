#pragma once
#include <algorithm>
#include "widget.hpp"

class GfxCenterBoard: public Widget
{
    public:
        struct InitArgs
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x   = 0;
            Widget::VarInt y   = 0;

            std::vector<Widget::WADPair> childList {};

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    public:
        GfxCenterBoard(GfxCenterBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w = std::nullopt,
                  .h = std::nullopt,

                  .attrs  = std::move(args.attrs ),
                  .parent = std::move(args.parent),

              }}
        {
            for(auto &child: args.childList){
                addChildAt(child.widget, DIR_NONE, [this]{ return w() / 2; }, [this]{ return h() / 2; }, child.autoDelete);
            }
        }

    public:
        int w() const override { int maxW = 0; foreachChild(false, [&maxW, this](const Widget *widget, bool){ maxW = std::max<int>(maxW, widget->w()); }); return maxW; }
        int h() const override { int maxH = 0; foreachChild(false, [&maxH, this](const Widget *widget, bool){ maxH = std::max<int>(maxH, widget->h()); }); return maxH; }
};
