/*
 * =====================================================================================
 *
 *       Filename: indepmagic.hpp
 *        Created: 08/07/2017 21:19:44
 *  Last Modified: 08/08/2017 12:20:42
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
#include <cstdint>

class IndepMagic
{
    // don't have a m_MapID filed
    // independent magic should be bound to map
    // if bound to creture then use effect to present

    private:
        const uint32_t m_UID;

    private:
        const int m_MagicID;
        const int m_MagicParam;

    private:
        int m_Speed;
        int m_Direction;

    private:
        int m_X;
        int m_Y;

    private:
        int m_AimX;
        int m_AimY;

    private:
        const uint32_t m_AimUID;

    private:
        IndepMagic(uint32_t,    // UID
                int,            // Magic
                int,            // MagicParam
                int,            // Speed
                int,            // Direction
                int,            // X
                int,            // Y
                int,            // AimX
                int,            // AimY
                uint32_t);      // AimUID

    public:
        IndepMagic(uint32_t,    // UID
                int,            // Magic
                int,            // MagicParam
                int,            // Speed
                int,            // Direction
                int,            // X
                int,            // Y
                int,            // AimX
                int);           // AimY

        IndepMagic(uint32_t,    // UID
                int,            // Magic
                int,            // MagicParam
                int,            // Speed
                int,            // Direction
                int,            // X
                int,            // Y
                uint32_t);      // AimUID

    public:
        ~IndepMagic() = default;

    public:
        int X() const { return m_X; }
        int Y() const { return m_Y; }
};
