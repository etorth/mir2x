/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 04/03/2017 10:41:08
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
#include "creature.hpp"

class Hero: public Creature
{
    protected:
        uint32_t m_DBID;

    protected:
        bool m_Male;

    public:
        Hero(uint32_t, uint32_t, bool, ProcessRun *, int, int, int, int, int);
       ~Hero() = default;

    public:
        void Draw(int, int);
        void Update();

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

        size_t FrameCount()
        {
            return 1;
        }

    public:
        bool ValidG()
        {
            return true;
        }
};
