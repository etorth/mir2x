#pragma once
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
        Widget *m_canvas; // the unique child

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

                  .w = std::nullopt,
                  .h = std::nullopt,

                  .childList
                  {
                      {
                          .widget = new Widget
                          {{
                              .attrs
                              {
                                  .inst
                                  {
                                      .name = "Canvas",
                                      .moveOnFocus = false,
                                  }
                              }
                          }},
                          .autoDelete = true,
                      },
                  },

                  .attrs
                  {
                      .type
                      {
                          .setSize = false,
                          .addChild = false,
                          .removeChild = false,
                      },
                      .inst = std::move(args.attrs),
                  },
                  .parent = std::move(args.parent),
              }}

            , m_canvas(firstChild())

            , m_margin (std::move(args.margin))
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
            if(m_bgBoard){
                m_canvas->addChild(m_bgBoard, true);
            }

            if(m_wrapped.widget){
                doSetWrapped();
            }

            if(m_fgBoard){
                m_canvas->addChild(m_fgBoard, true);
            }

            m_canvas->setSize([this]{ return (m_wrapped.widget ? m_wrapped.widget->w() : 0) + Widget::evalSize(m_margin[2], this) + Widget::evalSize(m_margin[3], this); },
                              [this]{ return (m_wrapped.widget ? m_wrapped.widget->h() : 0) + Widget::evalSize(m_margin[0], this) + Widget::evalSize(m_margin[1], this); });
        }

    public:
        auto wrapped(this auto && self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>
        {
            return self.m_wrapped.widget;
        }

        auto wrappedPair(this auto && self) -> std::pair<std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>, bool>
        {
            return {self.m_wrapped.widget, self.m_wrapped.autoDelete};
        }

    public:
        void setWrapped(Widget *argWidget, bool argAutoDelete)
        {
            if(m_wrapped.widget){
                m_canvas->removeChild(m_wrapped.widget->id(), m_wrapped.widget != argWidget); // same widget may change autoDelete
            }

            m_wrapped = Widget::WADPair{.widget = argWidget, .autoDelete = argAutoDelete};
            doSetWrapped();
        }

        void clearWrapped(bool argTriggerAutoDelete)
        {
            if(m_wrapped.widget){
                m_canvas->removeChild(m_wrapped.widget->id(), argTriggerAutoDelete);
                m_wrapped = Widget::WADPair{};
            }
        }

    private:
        void doSetWrapped()
        {
            if(m_wrapped.widget){
                m_canvas->addChildAt(m_wrapped.widget, DIR_UPLEFT, [this]
                {
                    return Widget::evalSize(m_margin[2], this);
                },

                [this]
                {
                    return Widget::evalSize(m_margin[0], this);
                },

                m_wrapped.autoDelete);
            }
        }
};
