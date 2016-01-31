/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 01/29/2016 20:41:58
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

class Hero: public Actor
{
    public:
        Hero(int, int, int);
        ~Hero();

    public:
        void Draw();
        void Update();

    public:
        int FrameCount();

    public:
        void UpdateCurrentState();
        void UpdateWithNewState();

    public:
        uint32_t    m_LookID;
};
