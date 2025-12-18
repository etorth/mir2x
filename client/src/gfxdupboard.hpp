#pragma once
#include "widget.hpp"

class GfxDupBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROIOpt vro {};

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;
        Widget::VarROIOpt m_vro;

    public:
        GfxDupBoard(GfxDupBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),
                  .w = std::move(args.w),
                  .h = std::move(args.h),

                  .attrs = std::move(args.attrs),
                  .parent = std::move(args.parent),
              }}

            , m_getter(std::move(args.getter))
            , m_vro(std::move(args.vro))
        {}

    public:
        Widget *gfxWidget() const
        {
            return Widget::evalGetter(m_getter, this);
        }

        Widget::ROI gfxCropROI() const
        {
            if(m_vro.has_value()){
                return m_vro->roi(this, nullptr);
            }
            else if(const auto gfxPtr = gfxWidget()){
                return gfxPtr->roi();
            }
            else{
                throw fflerror("gfxCropROI");
            }
        }

    private:
        void gridHelper(this auto && self, Widget::ROIMap m, auto && func)
        {
            if(!m.calibrate(std::addressof(self))){
                return;
            }

            if(auto gfxPtr = self.gfxWidget()){
                if(const auto cr = self.gfxCropROI()){
                    if(!gfxPtr->roi().crop(cr)){
                        return;
                    }

                    for(int yi = m.ro->y / cr.h; yi * cr.h < m.ro->y + m.ro->h; ++yi){
                        for(int xi = m.ro->x / cr.w; xi * cr.w < m.ro->x + m.ro->w; ++xi){
                            func(gfxPtr, m.map(xi * cr.w - cr.x, yi * cr.h - cr.y, cr));
                        }
                    }
                }
            }
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            gridHelper(m, [](const auto *widget, const auto &cm){ widget->draw(cm); });
        }

    public:
        bool processEventDefault(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            bool takenEvent = false;
            gridHelper(m, [&e, valid, &takenEvent](auto *widget, const auto &cm)
            {
                takenEvent |= widget->processEvent(e, valid && !takenEvent, cm);
            });

            return takenEvent;
        }
};
