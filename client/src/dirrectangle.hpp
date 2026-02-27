#include "colorf.hpp"
#include "gfxshapeboard.hpp"

class DirRectangle: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            struct TriangleArgs
            {
                Widget::VarDir   dir = DIR_UP;
                Widget::VarSize maxW = 0; // may shrink if no enough space
                Widget::VarSize    h = 0;
            }
            triangle {};

            struct RectangleArgs
            {
                Widget::VarSize r = 0;
                Widget::VarSize w = 0;
                Widget::VarSize h = 0;
            }
            rectangle {};

            Widget::VarU32 bgColor = colorf::WHITE_A255;
            Widget::VarU32 fgColor = 0U;

            Widget::WADPair parent {};
        };

    public:
        explicit DirRectangle(DirRectangle::InitArgs);
};
