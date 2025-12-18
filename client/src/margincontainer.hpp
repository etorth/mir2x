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

            Widget::WADPair parent {};
        };

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
                  .w = std::move(args.w),
                  .h = std::move(args.h),

                  .parent = std::move(args.parent),
              }}

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
                Widget::addChild(m_bgBoard, true);
            }

            doSetContained();

            if(m_fgBoard){
                Widget::addChild(m_fgBoard, true);
            }
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            if(m_bgBoard          && m_bgBoard         ->show()) drawChild(m_bgBoard         , m);
            if(m_contained.widget && m_contained.widget->show()) drawChild(m_contained.widget, m);
            if(m_fgBoard          && m_fgBoard         ->show()) drawChild(m_fgBoard         , m);
        }

        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            if(m_contained.widget){
                return m_contained.widget->processEventParent(event, valid, m);
            }
            return false;
        }

    public:
        auto contained(this auto && self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>
        {
            return self.m_contained.widget;
        }

    public:
        void addChild  (Widget *,                                                 bool) override { throw fflvalue(name()); }
        void addChildAt(Widget *, Widget::VarDir, Widget::VarInt, Widget::VarInt, bool) override { throw fflvalue(name()); }

    public:
        void setContained(Widget::VarDir argDir, Widget *argWidget, bool argAutoDelete)
        {
            if(m_contained.widget){
                removeChild(m_contained.widget, true);
            }

            m_contained = ContainedWidget{.dir = std::move(argDir), .widget = argWidget, .autoDelete = argAutoDelete};
            doSetContained();
        }

    private:
        void doSetContained()
        {
            if(m_contained.widget){
                Widget::addChildAt(m_contained.widget, [this]
                {
                    return Widget::evalDir(m_contained.dir, this);
                },

                [this]
                {
                    return Widget::xSizeOff(Widget::evalDir(m_contained.dir, this), w());
                },

                [this]
                {
                    return Widget::ySizeOff(Widget::evalDir(m_contained.dir, this), h());
                },

                m_contained.autoDelete);
            }
        }
};
