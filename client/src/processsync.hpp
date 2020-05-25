/*
 * =====================================================================================
 *
 *       Filename: processsync.hpp
 *        Created: 08/14/2015 02:47:30
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
#include <SDL2/SDL.h>
#include "process.hpp"
#include "labelboard.hpp"

class ProcessSync: public Process
{
    private:
        int        m_ratio;
        LabelBoard m_processBarInfo;

    public:
        ProcessSync();

    public:
        ~ProcessSync() = default;

    public:
        int ID() const
        {
            return PROCESSID_SYRC;
        }

    public:
        void update(double) override;
        void draw() override;
        void processEvent(const SDL_Event &) override;
};
