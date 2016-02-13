/*
 * =====================================================================================
 *
 *       Filename: process.cpp
 *        Created: 6/29/2015 8:24:36 PM
 *  Last Modified: 01/23/2016 03:53:26
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




