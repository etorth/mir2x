#pragma once

enum MouseInGfxButtonStateType: int
{
    BEVENT_OFF = 0,
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
