/*
 * =====================================================================================
 *
 *       Filename: mapthread.cpp
 *        Created: 02/24/2016 00:16:07
 *  Last Modified: 02/27/2016 20:15:59
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


#include "mapthread.hpp"
#include "monoserver.hpp"

MapThread::MapThread()
{
}

MapThread::~MapThread()
{
}

bool MapThread::Load(uint16_t nMapID, void *pData)
{
    m_MapID      = nMapID;
    m_MonoServer = (MonoServer *)pData;

    return true;
}

void MapThread::Run()
{
}
