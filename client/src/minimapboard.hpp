/*
 * =====================================================================================
 *
 *       Filename: minimapboard.hpp
 *        Created: 10/08/2017 19:22:30
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
        void drawFrame() const;
        void drawMiniMapTexture() const;

    private:
        int getFrameSize() const;
};
