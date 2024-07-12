#pragma once
#include "widget.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class MiniMapBoard: public Widget
{
    private:
        bool m_alphaOn  = false;
        bool m_extended = false;

    private:
        ProcessRun *m_processRun;

    private:
        TritexButton m_buttonAlpha;
        TritexButton m_buttonExtend;

    public:
        MiniMapBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void setPLoc();
        void flipExtended();
        void flipMiniMapShow();
        SDL_Texture *getMiniMapTexture() const;

    private:
        void drawMiniMapTexture() const;

    private:
        int getFrameSize() const;

    private:
        std::tuple<int, int> mouseOnMapGLoc(int, int) const;
};
