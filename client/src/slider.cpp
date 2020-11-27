/*
 * =====================================================================================
 *
 *       Filename: slider.cpp
 *        Created: 08/12/2015 09:59:15
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "widget.hpp"
#include "slider.hpp"
#include "bevent.hpp"

bool Slider::processEvent(const SDL_Event &e, bool valid)
{
    if(!valid){
        return focusConsumer(this, false);
    }

    auto fnInSlider = [this](int eventX, int eventY) -> bool
    {
        const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();
        return mathf::pointInRectangle<int>(eventX, eventY, sliderX, sliderY, sliderW, sliderH);
    };

    switch(e.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(fnInSlider(e.button.x, e.button.y)){
                    m_sliderState = BEVENT_DOWN;
                    return focusConsumer(this, true);
                }
                else if(in(e.button.x, e.button.y)){
                    m_sliderState = BEVENT_ON;
                    setValue([&e, this]() -> float
                    {
                        if(m_hslider){
                            return ((e.button.x - x()) * 1.0f) / w();
                        }
                        return ((e.button.y - y()) * 1.0f) / h();
                    }());
                    return focusConsumer(this, true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return focusConsumer(this, false);
                }
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(fnInSlider(e.button.x, e.button.y)){
                    m_sliderState = BEVENT_ON;
                    return focusConsumer(this, true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return focusConsumer(this, false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(e.motion.state & SDL_BUTTON_LMASK){
                    if(fnInSlider(e.motion.x, e.motion.y) || focus()){
                        if(m_hslider){
                            addValue(pixel2Value(e.motion.xrel));
                        }
                        else{
                            addValue(pixel2Value(e.motion.yrel));
                        }
                        m_sliderState = BEVENT_DOWN;
                        return focusConsumer(this, true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return focusConsumer(this, false);
                    }
                }
                else{
                    if(fnInSlider(e.motion.x, e.motion.y)){
                        m_sliderState = BEVENT_ON;
                        return focusConsumer(this, true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return focusConsumer(this, false);
                    }
                }
            }
        default:
            {
                return focusConsumer(this, false);
            }
    }
}
