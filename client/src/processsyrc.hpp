/*
 * =====================================================================================
 *
 *       Filename: processsyrc.hpp
 *        Created: 8/14/2015 2:47:30 PM
 *  Last Modified: 07/11/2017 15:33:23
 *
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
#include "label.hpp"
#include "process.hpp"
#include "tokenboard.hpp"

class ProcessSyrc: public Process
{
    private:
        int     m_Ratio;
        Label   m_Info;


    public:
        ProcessSyrc();
        virtual ~ProcessSyrc();

    public:
        int ID()
        {
            return PROCESSID_SYRC;
        }

    public:
        void Update(double);
        void Draw();
        void ProcessEvent(const SDL_Event &);
};
