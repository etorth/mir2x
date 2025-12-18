#pragma once
#include <variant>
#include <cstddef>
#include <functional>
#include <type_traits>
#include "widget.hpp"
#include "gfxshapeboard.hpp"

class MarginWrapper: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::WADPair wrapped {};
            Widget::VarMargin margin {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarMargin m_margin;

    private:
        Widget::WADPair m_wrapped;

    private:
        GfxShapeBoard *m_bgBoard;
        GfxShapeBoard *m_fgBoard;

    public:
        explicit MarginWrapper(MarginWrapper::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .attrs
                  {
                      .inst = std::move(args.attrs),
                  },
                  .parent = std::move(args.parent),
              }}

            , m_margin(std::move(args.margin))
            , m_wrapped(std::move(args.wrapped))
            , m_bgBoard(Widget::hasDrawFunc(args.bgDrawFunc) ? new GfxShapeBoard
              {{
                  .w = [this]{ return w(); },
                  .h = [this]{ return h(); },

                  .drawFunc = std::move(args.bgDrawFunc),

              }} : nullptr)

            , m_fgBoard(Widget::hasDrawFunc(args.fgDrawFunc) ? new GfxShapeBoard
              {{
                  .w = [this]{ return w(); },
                  .h = [this]{ return h(); },

                  .drawFunc = std::move(args.fgDrawFunc),

              }} : nullptr)
        {
            setSize([this]{ return (m_wrapped.widget ? m_wrapped.widget->w() : 0) + Widget::evalSize(m_margin[2], this) + Widget::evalSize(m_margin[3], this); },
                    [this]{ return (m_wrapped.widget ? m_wrapped.widget->h() : 0) + Widget::evalSize(m_margin[0], this) + Widget::evalSize(m_margin[1], this); });

            if(m_bgBoard){
                Widget::addChild(m_bgBoard, true);
            }

            doSetWrapped();

            if(m_fgBoard){
                Widget::addChild(m_fgBoard, true);
            }
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            if(m_bgBoard        && m_bgBoard       ->show()) drawChild(m_bgBoard       , m);
            if(m_wrapped.widget && m_wrapped.widget->show()) drawChild(m_wrapped.widget, m);
            if(m_fgBoard        && m_fgBoard       ->show()) drawChild(m_fgBoard       , m);
        }

        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            if(m_wrapped.widget && m_wrapped.widget->show()){
                return m_wrapped.widget->processEventParent(event, valid, m);
            }
            return false;
        }

    public:
        auto wrapped(this auto && self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>
        {
            return self.m_wrapped.widget;
        }

    public:
        void addChild  (Widget *,                                                 bool) override { throw fflvalue(name()); }
        void addChildAt(Widget *, Widget::VarDir, Widget::VarInt, Widget::VarInt, bool) override { throw fflvalue(name()); }

    public:
        void setWrapped(Widget *widget, bool autoDelete)
        {
            if(m_wrapped.widget){
                removeChild(m_wrapped.widget, true);
            }

            m_wrapped = Widget::WADPair{.widget = widget, .autoDelete = autoDelete};
            doSetWrapped();
        }

    private:
        void doSetWrapped()
        {
            Widget::addChildAt(m_wrapped.widget, DIR_UPLEFT, [this]
            {
                return Widget::evalSize(m_margin[2], this);
            },

            [this]
            {
                return Widget::evalSize(m_margin[0], this);
            },

            m_wrapped.autoDelete);
        }
};
