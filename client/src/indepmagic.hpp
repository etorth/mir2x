/*
 * =====================================================================================
 *
 *       Filename: indepmagic.hpp
 *        Created: 08/07/2017 21:19:44
 *  Last Modified: 08/09/2017 18:31:44
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
#include "magicrecord.hpp"

class IndepMagic final
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
        int m_Stage;

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
        // need mutable for the cache entry
        // but don't put std::atomic here since I use it in single-thread
        mutable const GfxEntry *m_CacheEntry;

    private:
        double m_AccuTime;

    public:
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
        int ID() const
        {
            return m_MagicID;
        }

        int Stage() const
        {
            return m_Stage;
        }

    public:
        int X() const
        {
            return m_X;
        }

        int Y() const
        {
            return m_Y;
        }

        int AimX() const
        {
            return m_AimX;
        }

        int AimY() const
        {
            return m_AimY;
        }

        uint32_t AimUID() const
        {
            return m_AimUID;
        }

    public:
        bool Done() const;
        bool StageDone() const;
        
    public:
        int Frame() const;

    public:
        void Update(double);
        void Draw(int, int);

    private:
        bool RefreshCache() const;
};
