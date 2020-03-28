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
#include "tokenboard.hpp"

class ProcessSync: public Process
{
    private:
        int        m_Ratio;
        labelBoard m_ProcessBarInfo;

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
        void Update(double);
        void Draw();
        void processEvent(const SDL_Event &);
};
