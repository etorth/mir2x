/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 9/3/2015 3:49:00 AM
 *  Last Modified: 03/31/2017 00:52:04
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

#include "hero.hpp"

Hero::Hero(uint32_t nUID, uint32_t nGUID, bool bMale, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : Creature(nUID, pRun, nX, nY, nAction, nDirection, nSpeed)
    , m_GUID(nGUID)
    , m_Male(bMale)
{}

void Hero::Draw(int, int)
{
}

void Hero::Update()
{
}
