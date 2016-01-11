/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 8/31/2015 8:52:57 PM
 *  Last Modified: 09/09/2015 7:52:38 PM
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
#include <SDL.h>

MyHero::MyHero(int nSID, int nUID, int nGenTime)
	: Hero(nSID, nUID, nGenTime)
{}

MyHero::~MyHero()
{}

void MyHero::Update()
{
    Hero::Update();
}
