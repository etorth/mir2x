#include <SDL2/SDL.h>
#include "slider.hpp"

bool Slider::processEvent(const SDL_Event &e, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!active()){
        return consumeFocus(false);
    }

    const auto fnInSlider = [this](int eventX, int eventY) -> bool
    {
        const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();
        return mathf::pointInRectangle<int>(eventX, eventY, sliderX, sliderY, sliderW, sliderH);
    };

    switch(e.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(fnInSlider(e.button.x, e.button.y)){
                    m_sliderState = BEVENT_DOWN;
                    return consumeFocus(true);
                }
                else if(in(e.button.x, e.button.y)){
                    m_sliderState = BEVENT_ON;
                    setValue([&e, this]() -> float
                    {
                        if(m_hslider){
                            return ((e.button.x - x()) * 1.0f) / std::max<int>(1, w());
                        }
                        else{
                            return ((e.button.y - y()) * 1.0f) / std::max<int>(1, h());
                        }
                    }(), true);
                    return consumeFocus(true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(fnInSlider(e.button.x, e.button.y)){
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
                if(e.motion.state & SDL_BUTTON_LMASK){
                    if(fnInSlider(e.motion.x, e.motion.y) || focus()){
                        m_sliderState = BEVENT_DOWN;
                        if(m_hslider){
                            addValue(pixel2Value(e.motion.xrel), true);
                        }
                        else{
                            addValue(pixel2Value(e.motion.yrel), true);
                        }
                        return consumeFocus(true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return consumeFocus(false);
                    }
                }
                else{
                    if(fnInSlider(e.motion.x, e.motion.y)){
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
