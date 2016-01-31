/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 8/31/2015 8:26:19 PM
 *  Last Modified: 09/08/2015 3:28:59 AM
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
        int  GenTime();

    public:
        void Draw();
        int  FrameCount();

    public:
        void UpdateCurrentState();
        void UpdateWithNewState();
};
