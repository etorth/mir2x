#include <SDL2/SDL.h>
#include "sdldevice.hpp"
#include "sliderbase.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

SliderBase::SliderBase(SliderBase::InitArgs args)
    : Widget
      {{
          .parent = std::move(args.parent),
      }}

    , m_value(fflcheck(args.value, args.value >= 0.0f && args.value <= 1.0f))
    , m_checkFunc(std::move(args.checkFunc))

    , m_barArgs(std::move(args.bar))
    , m_sliderArgs(std::move(args.slider))

    , m_onChange(std::move(args.onChange))
    , m_bar
      {{
          .dir = std::move(args.bar.dir),

          .x = [this]{ return -1 * widgetXFromBar(0); },
          .y = [this]{ return -1 * widgetYFromBar(0); },

          .w = [this]{ return Widget::evalSize(m_barArgs.w, this); },
          .h = [this]{ return Widget::evalSize(m_barArgs.h, this); },

          .contained = std::move(args.barWidget),
          .parent{this},
      }}

    , m_slider
      {{
          .x = [this]{ return sliderXAtValueFromBar(getValue(), m_bar.dx()); },
          .y = [this]{ return sliderYAtValueFromBar(getValue(), m_bar.dy()); },

          .w = [this]{ return Widget::evalSize(m_sliderArgs.w, this); },
          .h = [this]{ return Widget::evalSize(m_sliderArgs.h, this); },

          .contained = std::move(args.sliderWidget),
          .parent{this},
      }}

    , m_debugDraw
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(g_clientArgParser->debugSlider){
                  g_sdlDevice->drawRectangle(colorf::GREEN_A255, drawDstX, drawDstY, self->w(), self->h());
                  g_sdlDevice->drawRectangle(colorf::BLUE_A255, drawDstX + m_bar.dx(), drawDstY + m_bar.dy(), m_bar.w(), m_bar.h());

                  const auto r = getSliderROI(drawDstX, drawDstY);
                  const auto [cx, cy] = getValueCenter(drawDstX, drawDstY);

                  g_sdlDevice->drawLine(colorf::YELLOW_A255, r.x, r.y, cx, cy);
                  g_sdlDevice->drawRectangle(colorf::RED_A255, r.x, r.y, r.w, r.h);
              }
          },

          .parent{this},
      }}
{
    moveTo([this]{ return widgetXFromBar(Widget::evalInt(m_barArgs.x, this)); },
           [this]{ return widgetYFromBar(Widget::evalInt(m_barArgs.y, this)); });

    setSize([this]{ return std::max<int>(Widget::evalSize(m_barArgs.w, this), sliderXAtValueFromBar(1.0f, 0) + m_slider.w()) - widgetXFromBar(0); },
            [this]{ return std::max<int>(Widget::evalSize(m_barArgs.h, this), sliderYAtValueFromBar(1.0f, 0) + m_slider.h()) - widgetYFromBar(0); });

    if(args.bgWidget.widget){
        setBarBgWidget(std::move(args.bgWidget.ox), std::move(args.bgWidget.oy), args.bgWidget.widget, args.bgWidget.autoDelete);
    }
}

bool SliderBase::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return consumeFocus(false);
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(!active()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(inSlider(event.button.x, event.button.y, m)){
                    m_sliderState = BEVENT_DOWN;
                    return consumeFocus(true);
                }
                else if(const auto mbar = m.create(m_bar.roi(this)); mbar.in(event.button.x, event.button.y)){
                    m_sliderState = BEVENT_ON;
                    if(const auto newValue = std::clamp<float>([&event, startDstX = mbar.x - mbar.ro->x, startDstY = mbar.y - mbar.ro->y, this]() -> float
                    {
                        if(vbar()){
                            return ((event.button.y - startDstY) * 1.0f) / std::max<int>(1, m_bar.h());
                        }
                        else{
                            return ((event.button.x - startDstX) * 1.0f) / std::max<int>(1, m_bar.w());
                        }
                    }(), 0.0f, 1.0f);

                    Widget::execCheckFunc<float>(m_checkFunc, this, newValue)){
                        setValue(newValue, true);
                    }
                    return consumeFocus(true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(inSlider(event.button.x, event.button.y, m)){
                    m_sliderState = BEVENT_ON;
                    return consumeFocus(true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(event.motion.state & SDL_BUTTON_LMASK){
                    if(inSlider(event.motion.x, event.motion.y, m) || focus()){
                        m_sliderState = BEVENT_DOWN;
                        if(const auto newValue = std::clamp<float>(getValue() + [&event, this]() -> float
                        {
                            if(vbar()){
                                return pixel2Value(event.motion.yrel);
                            }
                            else{
                                return pixel2Value(event.motion.xrel);
                            }
                        }(), 0.0f, 1.0f);

                        Widget::execCheckFunc<float>(m_checkFunc, this, newValue)){
                            setValue(newValue, true);
                        }
                        return consumeFocus(true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return consumeFocus(false);
                    }
                }
                else{
                    if(inSlider(event.motion.x, event.motion.y, m)){
                        m_sliderState = BEVENT_ON;
                        return consumeFocus(true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return consumeFocus(false);
                    }
                }
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void SliderBase::setValue(float value, bool triggerCallback)
{
    if(const auto newValue = std::clamp<float>(value, 0.0f, 1.0f); newValue != getValue()){
        m_value = newValue; // can change value to outside of range [min, max]
        if(triggerCallback && Widget::hasUpdateFunc(m_onChange)){
            Widget::execUpdateFunc(m_onChange, this, getValue());
        }
    }
}

void SliderBase::addValue(float diff, bool triggerCallback)
{
    setValue(m_value + diff, triggerCallback);
}

float SliderBase::pixel2Value(int pixel) const
{
    return pixel * 1.0f / std::max<int>(vbar() ? (m_bar.h() - 1) : (m_bar.w() - 1), 1);
}

Widget::ROI SliderBase::getBarROI(int startDstX, int startDstY) const
{
    return Widget::ROI
    {
        .x = startDstX + m_bar.dx(),
        .y = startDstY + m_bar.dy(),
        .w =             m_bar. w(),
        .h =             m_bar. h(),
    };
}

Widget::ROI SliderBase::getSliderROI(int startDstX, int startDstY) const
{
    return Widget::ROI
    {
        .x = startDstX + m_slider.dx(),
        .y = startDstY + m_slider.dy(),
        .w =             m_slider. w(),
        .h =             m_slider. h(),
    };
}

std::tuple<int, int> SliderBase::getValueCenter(int startDstX, int startDstY) const
{
    return
    {
        startDstX + m_slider.dx() + Widget::evalInt(m_sliderArgs.cx.value_or(m_slider.w() / 2), this),
        startDstY + m_slider.dy() + Widget::evalInt(m_sliderArgs.cy.value_or(m_slider.h() / 2), this),
    };
}

bool SliderBase::inSlider(int eventX, int eventY, Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return false;
    }
    return m.create(m_slider.roi(this)).in(eventX, eventY);
}

void SliderBase::setBarBgWidget(Widget::VarInt ox, Widget::VarInt oy, Widget *bgWidget, bool autoDelete)
{
    if(bgWidget){
        m_bgOff = std::make_optional(std::make_pair(std::move(ox), std::move(oy)));
        addChildAt(bgWidget, DIR_UPLEFT,
                [this]{ return -1 * widgetXFromBar(0) - Widget::evalInt(m_bgOff->first , this); },
                [this]{ return -1 * widgetYFromBar(0) - Widget::evalInt(m_bgOff->second, this); }, autoDelete);
        moveFront(bgWidget);
    }
}

int SliderBase::sliderXAtValueFromBar(float value, int barX) const
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto barW = Widget::evalSize(m_barArgs.w, this);
    const auto sliderW = Widget::evalSize(m_sliderArgs.w, this);
    const auto sliderCX = Widget::evalInt(m_sliderArgs.cx.value_or(sliderW / 2), this);

    return vbar()
        ? (barX - sliderCX + barW / 2)
        : (barX - sliderCX + to_dround(value * (barW - 1)));
}

int SliderBase::sliderYAtValueFromBar(float value, int barY) const
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto barH = Widget::evalSize(m_barArgs.h, this);
    const auto sliderH = Widget::evalSize(m_sliderArgs.h, this);
    const auto sliderCY = Widget::evalInt(m_sliderArgs.cy.value_or(sliderH / 2), this);

    return vbar()
        ? (barY - sliderCY + to_dround(value * (barH - 1)))
        : (barY - sliderCY + barH / 2);
}

std::optional<int> SliderBase::bgXFromBar(int barX) const
{
    if(m_bgOff.has_value()){
        return barX - Widget::evalInt(m_bgOff->first, this);
    }
    return std::nullopt;
}

std::optional<int> SliderBase::bgYFromBar(int barY) const
{
    if(m_bgOff.has_value()){
        return barY - Widget::evalInt(m_bgOff->second, this);
    }
    return std::nullopt;
}
