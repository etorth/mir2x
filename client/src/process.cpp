/*
 * =====================================================================================
 *
 *       Filename: process.cpp
 *        Created: 6/29/2015 8:24:36 PM
 *  Last Modified: 03/17/2016 01:11:02
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

Process::Process()
    : m_NextProcessID(PROCESSID_NULL)
    , m_TotalTime(0.0)
    , m_FPS(60.0)
    , m_InvokeCount(0.0)
    , m_Quit(false)
{}

Process::~Process()
{}

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
    return m_NextProcessID != PROCESSID_NULL;
}
