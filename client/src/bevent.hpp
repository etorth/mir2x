#pragma once
#include "totype.hpp"
#include "fflerror.hpp"

enum MouseInGfxButtonStateType: int
{
    // guarantee the off/on/down to be 0/1/2
    // then setup texID as:
    //
    //     return std::array{0x00, 0x01, 0x02}[state];
    //
    // without this guarantee, the texID has to be setup as:
    //
    //     switch(state){
    //          case BEVENT_OFF:  return 0x00;
    //          case BEVENT_ON:   return 0x01;
    //          case BEVENT_DOWN: return 0x02;
    //     }

    BEVENT_BEGIN = 0,
    BEVENT_OFF   = 0,
    BEVENT_ON,
    BEVENT_DOWN,
};

enum MouseInGfxButtonEventType: int
{
    BEVENT_ENTER = 0,
    BEVENT_LEAVE,
    BEVENT_PRESS,
    BEVENT_RELEASE,
};

namespace bevent
{
    inline int to_event(int from, int to)
    {
        if     (from == BEVENT_OFF  && to == BEVENT_ON  ){ return BEVENT_ENTER  ; }
        else if(from == BEVENT_ON   && to == BEVENT_OFF ){ return BEVENT_LEAVE  ; }
        else if(from == BEVENT_ON   && to == BEVENT_DOWN){ return BEVENT_PRESS  ; }
        else if(from == BEVENT_OFF  && to == BEVENT_DOWN){ return BEVENT_PRESS  ; } // special case: mouse is right over the button without moving
        else if(from == BEVENT_DOWN && to == BEVENT_ON  ){ return BEVENT_RELEASE; }
        else{
            throw fflpanic("invalid state: {} -> {}", to_d(from), to_d(to));
        }
    }

    inline int to_state(int event)
    {
        switch(event){
            case BEVENT_ENTER  : return BEVENT_ON  ;
            case BEVENT_LEAVE  : return BEVENT_OFF ;
            case BEVENT_PRESS  : return BEVENT_DOWN;
            case BEVENT_RELEASE: return BEVENT_ON  ;
            default:
                {
                    throw fflpanic("invalid event: {}", to_d(event));
                }
        }
    }
}
