#pragma once

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
    BEVENT_HOVER = 0,
    BEVENT_ENTER,
    BEVENT_LEAVE,
    BEVENT_PRESS,
    BEVENT_RELEASE,
};
