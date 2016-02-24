/*
 * =====================================================================================
 *
 *       Filename: mapthread.cpp
 *        Created: 02/24/2016 00:16:07
 *  Last Modified: 02/24/2016 00:19:32
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

MapThread(uint16_t nMapID, void *pData)
    : m_MapID(nMapID)
    , m_MonoServer((MonoServer *)pData)
{
}

MapThread::Run()
{
}
