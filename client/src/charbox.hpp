#include "widget.hpp"

class CharBox: public Widget
{
    public:
        CharBox();
        virtual ~CharBox();
    public:
        void Set(const char *);
        void Update(Uint32);
        void Draw();
        bool HandleEvent(SDL_Event &);
};
