/*
 * =====================================================================================
 *
 *       Filename: mmapboard.hpp
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
class MMapBoard: public Widget
{
    private:
        enum MMapState: int
        {
            MMAP_OFF = 0,
            MMAP_ON,
            MMAP_EXTENDED,
            MMAP_FULLSCREEN,
        };
        MMapState m_state = MMAP_ON;

    private:
        bool m_alphaOn = false;

    private:
        ProcessRun *m_processRun;

    private:
        TritexButton m_buttonAlpha;
        TritexButton m_buttonExtend;

    public:
        MMapBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void next()
        {
            switch(m_state){
                case MMAP_OFF:
                    {
                        setState(MMAP_ON);
                        return;
                    }
                case MMAP_ON:
                case MMAP_EXTENDED:
                    {
                        setState(MMAP_FULLSCREEN);
                        return;
                    }
                case MMAP_FULLSCREEN:
                    {
                        setState(MMAP_OFF);
                        return;
                    }
                default:
                    {
                        throw bad_reach();
                    }
            }
        }

    public:
        void setLoc();

    private:
        void setState(MMapState);
};
