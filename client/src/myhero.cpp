/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 03/31/2017 00:52:33
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

#include "myhero.hpp"

MyHero::MyHero(uint32_t nUID, uint32_t nGUID, bool bMale, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
	: Hero(nUID, nGUID, bMale, pRun, nX, nY, nAction, nDirection, nSpeed)
{}

void MyHero::Update()
{
    Hero::Update();
}
