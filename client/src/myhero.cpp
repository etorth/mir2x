/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 8/31/2015 8:52:57 PM
 *  Last Modified: 01/24/2016 21:48:27
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

MyHero::MyHero(int nSID, int nUID, int nGenTime)
	: Hero(nSID, nUID, nGenTime)
{}

MyHero::~MyHero()
{}

void MyHero::Update()
{
    Hero::Update();
}
