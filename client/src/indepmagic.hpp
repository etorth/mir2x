/*
 * =====================================================================================
 *
 *       Filename: indepmagic.hpp
 *        Created: 08/07/2017 21:19:44
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
#include <cstdint>
#include "magicbase.hpp"
#include "magicrecord.hpp"

class IndepMagic: public MagicBase
{
    // don't have a m_mapID filed
    // independent magic should be bound to map
    // if bound to creture then use effect to present

    private:
        const uint64_t m_UID;

    private:
        int m_direction;

    private:
        int m_x;
        int m_y;

    private:
        int m_aimX;
        int m_aimY;

    private:
        const uint64_t m_aimUID;

    public:
        IndepMagic(uint64_t,    // UID
                int,            // MagicID
                int,            // MagicParam
                int,            // MagicStage
                int,            // Direction
                int,            // X
                int,            // Y
                int,            // AimX
                int,            // AimY
                uint64_t);      // AimUID

    public:
        IndepMagic(uint64_t,    // UID
                int,            // MagicID
                int,            // MagicParam
                int,            // MagicStage
                int,            // Direction
                int,            // X
                int,            // Y
                int,            // AimX
                int);           // AimY

        IndepMagic(uint64_t,    // UID
                int,            // MagicID
                int,            // MagicParam
                int,            // MagicStage
                int,            // Direction
                int,            // X
                int,            // Y
                uint64_t);      // AimUID

    private:
        bool DrawPLoc(int *, int *) const;

    public:
        int DrawPX() const
        {
            int nPX = -1;
            return DrawPLoc(&nPX, nullptr) ? nPX : -1;
        }

        int DrawPY() const
        {
            int nPY = -1;
            return DrawPLoc(nullptr, &nPY) ? nPY : -1;
        }

    public:
        int X() const
        {
            return m_x;
        }

        int Y() const
        {
            return m_y;
        }

        int AimX() const
        {
            return m_aimX;
        }

        int AimY() const
        {
            return m_aimY;
        }

        uint64_t AimUID() const
        {
            return m_aimUID;
        }

    public:
        bool Done() const;
        
    public:
        void Update(double);
        void Draw(int, int);
};
