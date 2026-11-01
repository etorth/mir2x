#pragma once
#include <cstdint>
#include <climits>
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "basemagic.hpp"

class ProcessRun;
class FollowUIDMagic: public BaseMagic
{
    protected:
        const int m_startX;
        const int m_startY;
        const int m_flyDirIndex;

    protected:
        int m_x;
        int m_y;
        int m_moveSpeed;

    protected:
        const uint64_t m_uid;
        ProcessRun * const m_process;

    private:
        int m_lastLDistance2 = INT_MAX;
        std::optional<std::tuple<int, int>> m_lastFlyOff;

    private:
        constexpr static double m_rotateDegreeSpeed = 360; // 360 degrees per second

    private:
        // clockwise, rotates the frame around its head, turns toward making tail -> head point to the target
        double m_rotateDegree = 0.0;

    public:
        FollowUIDMagic(
                const char8_t *, // magicName
                const char8_t *, // stageName
                int,             // x
                int,             // y
                int,             // gfxDirIndex, can be [0, m_gfxEntry->gfxDirIndex), used to indexing gfx resource
                int,             // flyDirIndex, flying direction when targetUID is unavailable and lastFlyOff is not set, always use [0, 16), only used to calculate location off, this is not gfxDirIndex
                int,             // moveSpeed
                uint64_t,        // targetUID
                ProcessRun *);   // ProcessRun

    public:
        bool done() const override;

    public:
        bool update(double) override;

    public:
        virtual void drawViewOff(int, int, uint32_t) const;

    public:
        // head of the magic APNG, not from raw mir2 database
        // the frame rotates around it, so it's never moved by the rotation
        std::tuple<int, int> targetOff() const;

    private:
        void updateRotateDegree(int, int, double);

    public:
        std::tuple<int, int> targetPLoc() const
        {
            const auto [txOff, tyOff] = targetOff();
            return
            {
                m_x + txOff,
                m_y + tyOff,
            };
        }

    public:
        std::tuple<int, int> location() const
        {
            return
            {
                m_x / SYS_MAPGRIDXP,
                m_y / SYS_MAPGRIDYP,
            };
        }

    public:
        std::tuple<int, int> getSoundEffectPosition() const;
};
