/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/03/2017 17:11:09
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

MyHero::MyHero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
	: Hero(nUID, nDBID, bMale, nDressID, pRun, nX, nY, nAction, nDirection, nSpeed)
{}

void MyHero::Update()
{
    Hero::Update();
}
