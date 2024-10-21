#pragma once
#include "widget.hpp"

class MarginWrapper: public Widget
{
    public:
        MarginWrapper(Widget::VarDir argDir,

                Widget::VarOffset argX,
                Widget::VarOffset argY,

                Widget *argWidget,
                bool    argWidgetAutoDelete,

                std::array<int, 4> argMargin = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  [argWidget](const Widget *){ return argWidget->w() + std::max<int>(argMargin[2], 0) + std::max<int>(argMargin[3], 0); },
                  [argWidget](const Widget *){ return argWidget->h() + std::max<int>(argMargin[0], 0) + std::max<int>(argMargin[1], 0); },

                  {
                      {argWidget, DIR_UPLEFT, std::max<int>(argMargin[2], 0), std::max<int>(argMargin[0], 0), argWidgetAutoDelete},
                  },

                  argParent,
                  argAutoDelete,
              }
        {}

    public:
        bool processEvent(const SDL_Event &event, bool valid) override
        {
            return wrapped()->processEvent(event, valid);
        }

    public:
        void addChild(Widget *argWidget, bool argAutoDelete) override
        {
            if(hasChild()){
                throw fflerror("margin wrapper contains single child widget");
            }
            Widget::addChild(argWidget, argAutoDelete);
        }

    public:
        const Widget *wrapped() const { return firstChild(); }
        /* */ Widget *wrapped()       { return firstChild(); }
};
