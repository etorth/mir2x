#pragma once
#include "widget.hpp"
#include "gfxdupboard.hpp"
#include "gfxcropboard.hpp"

class GfxResizeBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROI vr {};

            Widget::VarSize2D resize {};
            Widget::VarMargin margin {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;
        Widget::VarROI m_vr;

    private:
        Widget::VarSize2D m_resize;
        Widget::VarMargin m_margin;

    private:
        GfxShapeBoard *m_bgBoard;
        GfxShapeBoard *m_fgBoard;

    public:
        GfxResizeBoard(GfxResizeBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .attrs = std::move(args.attrs),
                  .parent = std::move(args.parent),
              }}

            , m_getter(std::move(args.getter))
            , m_vr(std::move(args.vr))

            , m_resize(std::move(args.resize))
            , m_margin(std::move(args.margin))

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
            setSize([this]
            {
                const auto vx = m_vr.x(this);
                const auto vw = m_vr.w(this);
                const auto rw = m_resize.w(this);

                if(auto gfxPtr = gfxWidget()){ return margin(2) + std::max<int>(vx, 0) + rw + std::max<int>(gfxPtr->w() - (vx + vw), 0) + margin(3); }
                else                         { return margin(2) + std::max<int>(vx, 0) + rw                                             + margin(3); }
            },

            [this]
            {
                const auto vy = m_vr.y(this);
                const auto vh = m_vr.h(this);
                const auto rh = m_resize.h(this);

                if(auto gfxPtr = gfxWidget()){ return margin(0) + std::max<int>(vy, 0) + rh + std::max<int>(gfxPtr->h() - (vy + vh), 0) + margin(1); }
                else                         { return margin(0) + std::max<int>(vy, 0) + rh                                             + margin(1); }
            });

            if(m_bgBoard){ Widget::addChild(m_bgBoard, true); }
            if(m_fgBoard){ Widget::addChild(m_fgBoard, true); }
        }

    public:
        Widget *gfxWidget() const
        {
            return Widget::evalGetter(m_getter, this);
        }

        void setGfxWidget(Widget::VarGetter<Widget *> getter)
        {
            m_getter = std::move(getter);
        }

    public:
        Widget::ROI gfxCropROI() const
        {
            return m_vr.roi(this, nullptr);
        }

        void setGfxCropROI(Widget::VarROI vr)
        {
            m_vr = std::move(vr);
        }

    public:
        Widget::IntMargin gfxMargin() const
        {
            return Widget::IntMargin
            {
                .up    = margin(0),
                .down  = margin(1),
                .left  = margin(2),
                .right = margin(3),
            };
        }

        void setGfxMargin(Widget::VarMargin margin)
        {
            m_margin = std::move(margin);
        }

    public:
        Widget::ROI gfxResizeROI() const
        {
            return Widget::ROI
            {
                .x = margin(2),
                .y = margin(0),
                .w = m_resize.w(this),
                .h = m_resize.h(this),
            };
        }

        void setGfxResize(Widget::VarSize2D resize)
        {
            m_resize = std::move(resize);
        }

    public:
        int margin(int index) const
        {
            return Widget::evalSize(m_margin[index], this);
        }

    private:
        void gridHelper(this auto && self, Widget::ROIMap m, auto && func)
        {
            auto gfxWidget = self.gfxWidget();
            if(!gfxWidget){
                return;
            }

            const auto fnOp = [gfxWidget, m = std::cref(m), &func](const Widget::ROI &cr, int dx, int dy, std::optional<std::pair<int, int>> dupSize = std::nullopt)
            {
                if(!cr){
                    return;
                }

                const int dw = dupSize.has_value() ? dupSize->first  : cr.w;
                const int dh = dupSize.has_value() ? dupSize->second : cr.h;

                if(const auto cm = m.get().create({dx, dy, dw, dh})){
                    GfxCropBoard crop{{.getter = gfxWidget, .vr{cr}}};
                    if(dupSize.has_value()){
                        GfxDupBoard dup{{.w = dupSize->first, .h = dupSize->second, .getter = &crop}};
                        func(&dup, cm);
                    }
                    else{
                        func(&crop, cm);
                    }
                }
            };

            const auto r = self.gfxCropROI();

            const int mx = self.margin(2);
            const int my = self.margin(0);

            const int ox = std::max<int>(r.x, 0);
            const int oy = std::max<int>(r.y, 0);

            const int cw = gfxWidget->w();
            const int ch = gfxWidget->h();

            const auto [rw, rh] = self.m_resize.size(&self);

            fnOp({        0,         0,            r.x,            r.y}, mx          , my                                                          ); // top-left
            fnOp({      r.x,         0,            r.w,            r.y}, mx + ox     , my          , std::make_pair(rw            ,            r.y)); // top-middle
            fnOp({r.x + r.w,         0, cw - r.x - r.w,            r.y}, mx + ox + rw, my                                                          ); // top-right
            fnOp({        0,       r.y,            r.x,            r.h}, mx          , my + oy     , std::make_pair(           r.x, rh            )); // middle-left
            fnOp({      r.x,       r.y,            r.w,            r.h}, mx + ox     , my + oy     , std::make_pair(rw            , rh            )); // middle
            fnOp({r.x + r.w,       r.y, cw - r.x - r.w,            r.h}, mx + ox + rw, my + oy     , std::make_pair(cw - r.x - r.w, rh            )); // middle-right
            fnOp({        0, r.y + r.h,            r.x, ch - r.y - r.h}, mx          , my + oy + rh                                                ); // bottom-left
            fnOp({      r.x, r.y + r.h,            r.w, ch - r.y - r.h}, mx + ox     , my + oy + rh, std::make_pair(rw            , ch - r.y - r.h)); // bottom-middle
            fnOp({r.x + r.w, r.y + r.h, cw - r.x - r.w, ch - r.y - r.h}, mx + ox + rw, my + oy + rh                                                ); // bottom-right
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            if(m_bgBoard){
                drawChild(m_bgBoard, m);
            }

            gridHelper(m, [](const auto *widget, const auto &cm)
            {
                widget->draw(cm);
            });

            if(m_fgBoard){
                drawChild(m_fgBoard, m);
            }
        }

        bool processEventDefault(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            if(!m.calibrate(this)){
                return false;
            }

            bool takenEvent = false;
            gridHelper(m, [&e, valid, &takenEvent](auto *widget, const auto &cm)
            {
                takenEvent |= widget->processEvent(e, valid && !takenEvent, cm);
            });

            return takenEvent;
        }
};
