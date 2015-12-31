/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 8/29/2015 10:08:06 PM
 *  Last Modified: 09/03/2015 6:37:57 PM
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
#include "actor.hpp"

class Monster: public Actor
{
    public:
        Monster(int, int, int);
        ~Monster();

    public:
        void Update();
};
