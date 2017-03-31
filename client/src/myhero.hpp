/*
 * =====================================================================================
 *
 *       Filename: myhero.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 03/31/2017 00:50:32
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

#pragma once
#include "hero.hpp"

class MyHero: public Hero
{
    public:
        MyHero(uint32_t, uint32_t, bool, ProcessRun *, int, int, int, int, int);
       ~MyHero() = default;

    public:
        void Update();
};
