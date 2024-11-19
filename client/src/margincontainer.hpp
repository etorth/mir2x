#pragma once
#include "widget.hpp"
#include "shapeclipboard.hpp"

class MarginContainer: public Widget
{
    private:
        Widget::VarDir m_widgetDir;

    public:
        MarginContainer(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSize argW,
                Widget::VarSize argH,

                Widget *argWidget,

                Widget::VarDir argWidgetDir,
                bool           argWidgetAutoDelete,

                std::function<void(const Widget *, int, int)> argDrawFunc = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  std::move(argW)
                  std::move(argH)

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_widgetDir(std::move(argWidgetDir))
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

            Widget::addChild(argWidget, [this](const Widget *)
            {
                return Widget::evalDir(m_widgetDir, this);
            },

            [this](const Widget *)
            {
                switch(Widget::evalDir(m_widgetDir, this)){
                    case DIR_UPLEFT   : return       0;
                    case DIR_UP       : return w() / 2;
                    case DIR_UPRIGHT  : return w() - 1;
                    case DIR_LEFT     : return       0;
                    case DIR_RIGHT    : return w() - 1;
                    case DIR_DOWNLEFT : return       0;
                    case DIR_DOWN     : return w() / 2;
                    case DIR_DOWNRIGHT: return w() - 1;
                    default           : return w() / 2; // DIR_NONE
                }
            },

            [this](const Widget *)
            {
                switch(Widget::evalDir(m_widgetDir, this)){
                    case DIR_UPLEFT   : return       0;
                    case DIR_UP       : return       0;
                    case DIR_UPRIGHT  : return       0;
                    case DIR_LEFT     : return h() / 2;
                    case DIR_RIGHT    : return h() / 2;
                    case DIR_DOWNLEFT : return h() - 1;
                    case DIR_DOWN     : return h() - 1;
                    case DIR_DOWNRIGHT: return h() - 1;
                    default           : return h() / 2; // DIR_NONE
                }
            },

            argWidgetAutoDelete);
        }

    public:
        bool processEventDefault(const SDL_Event &event, bool valid) override
        {
            return contained()->processEvent(event, valid);
        }

    public:
        void addChild(Widget *,                                                 bool) override { throw fflreach(); }
        void addChild(Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool) override { throw fflreach(); }

    public:
        const Widget *contained() const { return lastChild(); }
        /* */ Widget *contained()       { return lastChild(); }
};
