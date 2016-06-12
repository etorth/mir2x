/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 06/11/2016 15:05:09
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

MyHero::MyHero(uint32_t nGUID, uint32_t nUID, uint32_t nGenTime, bool bMale)
	: Hero(nGUID, nUID, nGenTime, bMale)
{}

MyHero::~MyHero()
{}

void MyHero::Update()
{
    Hero::Update();
}
