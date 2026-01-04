#pragma once
#include "widget.hpp"
#include "gfxshapeboard.hpp"

class MarginContainer: public Widget
{
    public:
        struct ContainedWidget final
        {
            Widget::VarDir dir = DIR_NONE;

            Widget * widget     = nullptr;
            bool     autoDelete = false;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            ContainedWidget contained {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget *m_canvas; // the unique child

    private:
        ContainedWidget m_contained;

    private:
        GfxShapeBoard *m_bgBoard;
        GfxShapeBoard *m_fgBoard;

    public:
        MarginContainer(MarginContainer::InitArgs args)
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
                              .w = std::move(args.w),
                              .h = std::move(args.h),

                              .attrs
                              {
                                  .inst
                                  {
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

            , m_contained(std::move(args.contained))
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

            if(m_contained.widget){
                doSetContained();
            }

            if(m_fgBoard){
                m_canvas->addChild(m_fgBoard, true);
            }
        }

    public:
        auto contained(this auto && self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>
        {
            return self.m_contained.widget;
        }

    public:
        void setContained(Widget::VarDir argDir, Widget *argWidget, bool argAutoDelete)
        {
            if(m_contained.widget){
                m_canvas->removeChild(m_contained.widget->id(), m_contained.widget != argWidget); // same widget may change autoDelete
            }

            m_contained = ContainedWidget{.dir = std::move(argDir), .widget = argWidget, .autoDelete = argAutoDelete};
            doSetContained();
        }

        void clearContained()
        {
            if(m_contained.widget){
                m_canvas->removeChild(m_contained.widget->id(), true);
                m_contained = ContainedWidget{};
            }
        }

    private:
        void doSetContained()
        {
            if(m_contained.widget){
                m_canvas->addChildAt(m_contained.widget, [this]
                {
                    return Widget::evalDir(m_contained.dir, this);
                },

                [this]
                {
                    return Widget::xSizeOff(Widget::evalDir(m_contained.dir, this), [this]{ return w(); });
                },

                [this]
                {
                    return Widget::ySizeOff(Widget::evalDir(m_contained.dir, this), [this]{ return h(); });
                },

                m_contained.autoDelete);
            }
        }
};
