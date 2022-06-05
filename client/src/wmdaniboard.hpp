#pragma once
#include "texaniboard.hpp"
class WMDAniBoard: public TexAniBoard
{
    public:
        WMDAniBoard(dir8_t dir, int x, int y, Widget * pwidget = nullptr, bool autoDelete = false)
            : TexAniBoard(dir, x, y, 0X04000010, 10, 8, true, true, pwidget, autoDelete)
        {}
};
