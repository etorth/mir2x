/*
 * =====================================================================================
 *
 *       Filename: process.cpp
 *        Created: 6/29/2015 8:24:36 PM
 *  Last Modified: 09/03/2015 7:33:33 AM
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


// currently nothing to do
//

#include <cstdio>
#include "process.hpp"

Process::Process(int nProcessID, Game *pGame)
    : m_ProcessID(nProcessID)
    , m_Game(pGame)
    , m_NextProcessID(Process::PROCESSID_NULL)
    , m_StartTime(0)
    , m_FPS(60)
    , m_InvokeCount(0)
    , m_Quit(false)
{}

Process::~Process()
{}

void Process::Enter()
{
    m_StartTime   = SDL_GetTicks();
    m_InvokeCount = 0;
}

void Process::Exit()
{
    // clear those data needed when switching
    m_NextProcessID = Process::PROCESSID_NULL;
}

void Process::EventDelay()
{
    double fEstimateTime   = m_InvokeCount * 1000.0 / m_FPS;
    Uint32 nNextUpdateTime = m_StartTime + (Uint32)fEstimateTime;

    while(true){
        SDL_Event stEvent;
        while(SDL_PollEvent(&stEvent)){
            HandleEvent(&stEvent);
        }

        auto nNowTicks = SDL_GetTicks();
        if(nNextUpdateTime > nNowTicks){
            // static int d = 0;
            // printf("delay here: %d\n", d++);
            if(SDL_WaitEventTimeout(&stEvent, (int)(nNextUpdateTime - nNowTicks))){
                HandleEvent(&stEvent);
            }
        }else{
            break;
        }
    }
}

void Process::ClearEvent()
{
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
}

int Process::ProcessID()
{
    return m_ProcessID;
}

bool Process::RequestQuit()
{
	return m_Quit;
}

int Process::NextProcessID()
{
    return m_NextProcessID;
}

bool Process::RequestNewProcess()
{
    return m_NextProcessID != Process::PROCESSID_NULL;
}

void Process::Update()
{
    m_InvokeCount++;
}
