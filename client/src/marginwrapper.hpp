#pragma once
#include "widget.hpp"
#include "shapeclipboard.hpp"

class MarginWrapper: public Widget
{
    public:
        MarginWrapper(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget *argWidget,
                bool    argWidgetAutoDelete,

                std::array<int, 4> argMargin = {},
                std::function<void(const Widget *, int, int)> argDrawFunc = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  [argWidget, argMargin](const Widget *){ return argWidget->w() + std::max<int>(argMargin[2], 0) + std::max<int>(argMargin[3], 0); },
                  [argWidget, argMargin](const Widget *){ return argWidget->h() + std::max<int>(argMargin[0], 0) + std::max<int>(argMargin[1], 0); },

                  {},

                  argParent,
                  argAutoDelete,
              }
        {
            Widget::addChild(new ShapeClipBoard
            {
                DIR_UPLEFT,
                0,
                0,

                [this](const Widget *){ return w(); },
                [this](const Widget *){ return h(); },

                std::move(argDrawFunc),
            },

            true);

            Widget::addChildAt(argWidget,
                    DIR_UPLEFT,

                    std::max<int>(argMargin[2], 0),
                    std::max<int>(argMargin[0], 0),

                    argWidgetAutoDelete);
        }

    public:
        bool processEventDefault(const SDL_Event &event, bool valid) override
        {
            return wrapped()->processEvent(event, valid);
        }

    public:
        void addChild  (Widget *,                                                 bool) override { throw fflreach(); }
        void addChildAt(Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool) override { throw fflreach(); }

    public:
        const Widget *wrapped() const { return lastChild(); }
        /* */ Widget *wrapped()       { return lastChild(); }
};
