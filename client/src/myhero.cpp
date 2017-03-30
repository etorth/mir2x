/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 03/29/2017 15:39:10
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

MyHero::MyHero(uint32_t nGUID, uint32_t nUID, bool bMale, ProcessRun *pRun)
	: Hero(nGUID, nUID, bMale, pRun)
{}

MyHero::~MyHero()
{}

void MyHero::Update()
{
    Hero::Update();
}
