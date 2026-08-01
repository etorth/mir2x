#include <algorithm>
#include <utility>
#include <SDL3/SDL.h>
#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "scrollcontainer.hpp"

extern SDLDevice *g_sdlDevice;
ScrollContainer::ScrollContainer(ScrollContainer::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .childList
          {
              Widget::AddChildArgs
              {
                  .widget = new Widget
                  {{
                      .attrs
                      {
                          .inst
                          {
                              .name = "Canvas",
                              .moveOnFocus = false,
                          },
                      },
                  }},
                  .autoDelete = true,
              },
          },

          .attrs
          {
              .type
              {
                  .setSize     = false,
                  .addChild    = false,
                  .removeChild = false,
              },
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_canvas(firstChild())
    , m_viewport(new GfxCropBoard
      {{
          .x = 0,
          .y = 0,

          .getter = std::move(args.getter),

          .vr = Widget::VarROI
          (
              Widget::VarInt ([this]{ return m_scrollX;   }),
              Widget::VarInt ([this]{ return m_scrollY;   }),
              Widget::VarSize([this]{ return viewportW(); }),
              Widget::VarSize([this]{ return viewportH(); })
          ),

          .bgDrawFunc = std::move(args.bgDrawFunc),
          .fgDrawFunc = std::move(args.fgDrawFunc),

          .attrs
          {
              .inst
              {
                  .name = "ScrollViewport",
              },
          },
      }})

    , m_viewportW(std::move(args.vpw))
    , m_viewportH(std::move(args.vph))

    , m_hScroll(std::move(args.hScroll))
    , m_vScroll(std::move(args.vScroll))

    , m_barSize     (args.barSize)
    , m_minThumbSize(args.minThumbSize)
    , m_scrollStep  (args.scrollStep)

    , m_trackColor(std::move(args.trackColor))

    , m_thumbColor(std::move(args.thumbColor))
    , m_thumbHoverColor(std::move(args.thumbHoverColor))
    , m_thumbDragColor(std::move(args.thumbDragColor))

    , m_arrowColor(std::move(args.arrowColor))
    , m_arrowBoxColor(std::move(args.arrowBoxColor))
{
    fflassert(m_canvas);
    fflassert(m_viewport);

    m_canvas->addChild(m_viewport, true);
    m_canvas->setSize([this]{ return viewportW() + (vBarEnabled() ? m_barSize : 0); },
                      [this]{ return viewportH() + (hBarEnabled() ? m_barSize : 0); });
}

bool ScrollContainer::hBarEnabled() const
{
    if(!Widget::evalBool(m_hScroll, this)){
        return false;
    }

    const int cw = contained() ? contained()->w() : 0;
    return cw > viewportW();
}

bool ScrollContainer::vBarEnabled() const
{
    if(!Widget::evalBool(m_vScroll, this)){
        return false;
    }

    const int ch = contained() ? contained()->h() : 0;
    return ch > viewportH();
}

Widget::ROI ScrollContainer::vBarROI() const
{
    if(!vBarEnabled()){
        return {};
    }

    return Widget::ROI
    {
        .x = std::max<int>(0, w() - m_barSize),
        .y = 0,
        .w = std::min<int>(m_barSize, w()),
        .h = viewportH(),
    };
}

Widget::ROI ScrollContainer::hBarROI() const
{
    if(!hBarEnabled()){
        return {};
    }

    return Widget::ROI
    {
        .x = 0,
        .y = std::max<int>(0, h() - m_barSize),
        .w = viewportW(),
        .h = std::min<int>(m_barSize, h()),
    };
}

Widget::ROI ScrollContainer::vUpArrowROI() const
{
    const auto bar = vBarROI();
    if(bar.empty() || bar.h < m_barSize){
        return {};
    }
    return Widget::ROI{bar.x, bar.y, bar.w, m_barSize};
}

Widget::ROI ScrollContainer::vDownArrowROI() const
{
    const auto bar = vBarROI();
    if(bar.empty() || bar.h < m_barSize){
        return {};
    }
    return Widget::ROI{bar.x, bar.y + bar.h - m_barSize, bar.w, m_barSize};
}

Widget::ROI ScrollContainer::hLeftArrowROI() const
{
    const auto bar = hBarROI();
    if(bar.empty() || bar.w < m_barSize){
        return {};
    }
    return Widget::ROI{bar.x, bar.y, m_barSize, bar.h};
}

Widget::ROI ScrollContainer::hRightArrowROI() const
{
    const auto bar = hBarROI();
    if(bar.empty() || bar.w < m_barSize){
        return {};
    }
    return Widget::ROI{bar.x + bar.w - m_barSize, bar.y, m_barSize, bar.h};
}

Widget::ROI ScrollContainer::vTrackROI() const
{
    const auto bar = vBarROI();
    if(bar.empty()){
        return {};
    }

    const auto inset = std::min<int>(m_barSize, bar.h / 2);
    return Widget::ROI
    {
        .x = bar.x,
        .y = bar.y + inset,
        .w = bar.w,
        .h = std::max<int>(0, bar.h - 2 * inset),
    };
}

Widget::ROI ScrollContainer::hTrackROI() const
{
    const auto bar = hBarROI();
    if(bar.empty()){
        return {};
    }

    const auto inset = std::min<int>(m_barSize, bar.w / 2);
    return Widget::ROI
    {
        .x = bar.x + inset,
        .y = bar.y,
        .w = std::max<int>(0, bar.w - 2 * inset),
        .h = bar.h,
    };
}

Widget::ROI ScrollContainer::vThumbROI() const
{
    const auto track = vTrackROI();
    if(track.empty()){
        return {};
    }

    const int ch = contained() ? contained()->h() : 0;
    const int vh = viewportH();
    if(ch <= 0 || vh <= 0){
        return track;
    }

    const int thumbH = std::clamp<int>((track.h * vh) / ch, std::min<int>(m_minThumbSize, track.h), track.h);
    const int maxScroll = maxScrollY();
    const int offset = maxScroll > 0 ? (track.h - thumbH) * m_scrollY / maxScroll : 0;

    return Widget::ROI{track.x, track.y + offset, track.w, thumbH};
}

Widget::ROI ScrollContainer::hThumbROI() const
{
    const auto track = hTrackROI();
    if(track.empty()){
        return {};
    }

    const int cw = contained() ? contained()->w() : 0;
    const int vw = viewportW();
    if(cw <= 0 || vw <= 0){
        return track;
    }

    const int thumbW = std::clamp<int>((track.w * vw) / cw, std::min<int>(m_minThumbSize, track.w), track.w);
    const int maxScroll = maxScrollX();
    const int offset = maxScroll > 0 ? (track.w - thumbW) * m_scrollX / maxScroll : 0;

    return Widget::ROI{track.x + offset, track.y, thumbW, track.h};
}

void ScrollContainer::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    Widget::drawDefault(m);

    const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
    const int sX = m.x - m.ro->x;
    const int sY = m.y - m.ro->y;

    const auto drawFillLocal = [sX, sY](uint32_t color, const Widget::ROI &r)
    {
        if(!r.empty()){
            g_sdlDevice->fillRectangle(color, sX + r.x, sY + r.y, r.w, r.h);
        }
    };

    const auto drawTriangleLocal = [sX, sY](uint32_t color, int x0, int y0, int x1, int y1, int x2, int y2)
    {
        g_sdlDevice->fillTriangle(color,
            sX + x0, sY + y0,
            sX + x1, sY + y1,
            sX + x2, sY + y2);
    };

    const auto vBar = vBarROI();
    const auto hBar = hBarROI();

    if(!vBar.empty()){
        drawFillLocal(Widget::evalU32(m_trackColor, this), vTrackROI());

        const auto up = vUpArrowROI();
        const auto down = vDownArrowROI();

        const auto arrowColor = Widget::evalU32(m_arrowColor, this);
        const auto arrowBoxColor = Widget::evalU32(m_arrowBoxColor, this);

        drawFillLocal(arrowBoxColor, up);
        drawFillLocal(arrowBoxColor, down);

        if(!up.empty()){
            const int cx    = up.x + up.w / 2;
            const int inset = std::max<int>(3, up.w / 4);
            drawTriangleLocal(arrowColor,
                cx                , up.y + inset,
                up.x + inset      , up.y + up.h - inset,
                up.x + up.w - inset, up.y + up.h - inset);
        }
        if(!down.empty()){
            const int cx    = down.x + down.w / 2;
            const int inset = std::max<int>(3, down.w / 4);
            drawTriangleLocal(arrowColor,
                down.x + inset          , down.y + inset,
                down.x + down.w - inset , down.y + inset,
                cx                      , down.y + down.h - inset);
        }

        const uint32_t thumbColor = (m_drag == DRAG_V_THUMB)
            ? Widget::evalU32(m_thumbDragColor , this)
            : Widget::evalU32(m_thumbColor     , this);
        drawFillLocal(thumbColor, vThumbROI());
    }

    if(!hBar.empty()){
        drawFillLocal(Widget::evalU32(m_trackColor, this), hTrackROI());

        const auto left = hLeftArrowROI();
        const auto right = hRightArrowROI();

        const auto arrowColor = Widget::evalU32(m_arrowColor, this);
        const auto arrowBoxColor = Widget::evalU32(m_arrowBoxColor, this);

        drawFillLocal(arrowBoxColor, left);
        drawFillLocal(arrowBoxColor, right);

        if(!left.empty()){
            const int cy    = left.y + left.h / 2;
            const int inset = std::max<int>(3, left.h / 4);
            drawTriangleLocal(arrowColor,
                left.x + inset          , cy,
                left.x + left.w - inset , left.y + inset,
                left.x + left.w - inset , left.y + left.h - inset);
        }
        if(!right.empty()){
            const int cy    = right.y + right.h / 2;
            const int inset = std::max<int>(3, right.h / 4);
            drawTriangleLocal(arrowColor,
                right.x + inset            , right.y + inset,
                right.x + inset            , right.y + right.h - inset,
                right.x + right.w - inset  , cy);
        }

        const uint32_t thumbColor = (m_drag == DRAG_H_THUMB)
            ? Widget::evalU32(m_thumbDragColor , this)
            : Widget::evalU32(m_thumbColor     , this);
        drawFillLocal(thumbColor, hThumbROI());
    }

    if(!vBar.empty() && !hBar.empty()){
        g_sdlDevice->fillRectangle(Widget::evalU32(m_trackColor, this), sX + vBar.x, sY + hBar.y, vBar.w, hBar.h);
    }
}

bool ScrollContainer::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        m_drag = DRAG_NONE;
        return false;
    }

    if(m_drag != DRAG_NONE){
        switch(event.type){
            case SDL_EVENT_MOUSE_MOTION:
                {
                    if(m_drag == DRAG_V_THUMB){
                        const auto track = vTrackROI();
                        const auto thumb = vThumbROI();
                        const int travel = std::max<int>(1, track.h - thumb.h);
                        const int dMouse = to_d(event.motion.y) - m_dragMouseStart;
                        scrollTo(m_scrollX, m_dragScrollStart + dMouse * maxScrollY() / travel);
                    }
                    else{
                        const auto track = hTrackROI();
                        const auto thumb = hThumbROI();
                        const int travel = std::max<int>(1, track.w - thumb.w);
                        const int dMouse = to_d(event.motion.x) - m_dragMouseStart;
                        scrollTo(m_dragScrollStart + dMouse * maxScrollX() / travel, m_scrollY);
                    }
                    return true;
                }
            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    m_drag = DRAG_NONE;
                    return true;
                }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    return true; // swallow, don't let content see button events during a drag
                }
            default:
                {
                    break;
                }
        }
    }

    if(event.type == SDL_EVENT_MOUSE_WHEEL){
        if(m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y))){
            if(SDL_GetModState() & SDL_KMOD_SHIFT){
                if(!hBarEnabled()){
                    return false;
                }
                scrollBy(-to_d(event.wheel.y) * m_scrollStep, 0);
            }
            else{
                if(!vBarEnabled()){
                    return false;
                }
                scrollBy(0, -to_d(event.wheel.y) * m_scrollStep);
            }
            return true;
        }
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
        const int localX = to_d(event.button.x) - m.x + m.ro->x;
        const int localY = to_d(event.button.y) - m.y + m.ro->y;

        if(const auto bar = vBarROI(); !bar.empty() && bar.in(localX, localY)){
            if(vUpArrowROI().in(localX, localY)){
                scrollBy(0, -m_scrollStep);
                return true;
            }

            if(vDownArrowROI().in(localX, localY)){
                scrollBy(0, m_scrollStep);
                return true;
            }

            if(const auto thumb = vThumbROI(); thumb.in(localX, localY)){
                m_drag = DRAG_V_THUMB;
                m_dragMouseStart = to_d(event.button.y); // capture raw screen coord for delta math
                m_dragScrollStart = m_scrollY;
                return true;
            }
            else if(vTrackROI().in(localX, localY)){
                if(localY < thumb.y){
                    scrollBy(0, -viewportH());
                }
                else{
                    scrollBy(0, viewportH());
                }
                return true;
            }
            return true; // anywhere else on the bar strip — consume silently
        }

        if(const auto bar = hBarROI(); !bar.empty() && bar.in(localX, localY)){
            if(hLeftArrowROI().in(localX, localY)){
                scrollBy(-m_scrollStep, 0);
                return true;
            }

            if(hRightArrowROI().in(localX, localY)){
                scrollBy(m_scrollStep, 0);
                return true;
            }

            if(const auto thumb = hThumbROI(); thumb.in(localX, localY)){
                m_drag = DRAG_H_THUMB;
                m_dragMouseStart = to_d(event.button.x);
                m_dragScrollStart = m_scrollX;
                return true;
            }
            else if(hTrackROI().in(localX, localY)){
                if(localX < thumb.x){
                    scrollBy(-viewportW(), 0);
                }
                else{
                    scrollBy(viewportW(), 0);
                }
                return true;
            }
            return true;
        }
    }

    return Widget::processEventDefault(event, valid, m);
}
