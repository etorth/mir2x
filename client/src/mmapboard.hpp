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
        enum MMapStatus: int
        {
            MMAP_OFF = 0,
            MMAP_ON,
            MMAP_EXTENDED,
            MMAP_FULLSCREEN,
        };
        MMapStatus m_status = MMAP_OFF;

    private:
        bool m_alphaOn = false;

    private:
        ProcessRun *m_processRun;

    public:
        MMapBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void next()
        {
            switch(m_status){
                case MMAP_OFF       : m_status = MMAP_ON        ; return;
                case MMAP_ON        : m_status = MMAP_FULLSCREEN; return;
                case MMAP_EXTENDED  : m_status = MMAP_FULLSCREEN; return;
                case MMAP_FULLSCREEN: m_status = MMAP_OFF       ; return;
                default: throw bad_reach();
            }
        }
};
