/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 08/31/2015 03:42:07 AM
 *  Last Modified: 05/31/2016 18:48:46
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
#include "process.hpp"
#include "message.hpp"
#include "mir2xmap.hpp"

class ProcessRun: public Process
{
    private:
        Mir2xMap    m_Map;
        int         m_ViewX;
        int         m_ViewY;

    public:
        ProcessRun();
        virtual ~ProcessRun() = default;

    public:
        virtual int ID()
        {
            return PROCESSID_RUN;
        }

    public:
        virtual void Update(double);
        virtual void Draw();
        virtual void ProcessEvent(const SDL_Event &);

    public:
        void Net_LoginOK(const uint8_t *, size_t);
        void Net_MotionState(const uint8_t *, size_t);
};
