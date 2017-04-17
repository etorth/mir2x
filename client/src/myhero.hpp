/*
 * =====================================================================================
 *
 *       Filename: myhero.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/16/2017 23:24:35
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
#include "actionnode.hpp"

class MyHero: public Hero
{
    public:
        MyHero(uint32_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

    public:
       ~MyHero() = default;

    public:
        bool RequestMove(int, int);

    public:
        bool Update();
};
