/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/06/2016 22:30:40
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
