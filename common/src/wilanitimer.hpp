/*
 * =====================================================================================
 *
 *       Filename: wilanitimer.hpp
 *        Created: 06/08/2021 22:17:08
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
#include <cstring>
#include "fflerror.hpp"

class WilAniTimer
{
    private:
        uint32_t m_aniSaveTime [8];
        uint8_t  m_aniTileFrame[8][16];

    public:
        WilAniTimer()
        {
            clear();
        }

    public:
        void update(uint32_t loopTime)
        {
            const static uint32_t delayTick[]
            {
                150,
                200,
                250,
                300,
                350,
                400,
                420,
                450,
            };

            // m_aniTileFrame[i][j]:
            //   i: denotes how fast the animation is.
            //   j: denotes how many frames the animation has.

            for(int i = 0; i < 8; ++i){
                m_aniSaveTime[i] += loopTime;
                if(m_aniSaveTime[i] > delayTick[i]){
                    for(int j = 0; j < 16; ++j){
                        m_aniTileFrame[i][j]++;
                        if(m_aniTileFrame[i][j] >= j){
                            m_aniTileFrame[i][j] = 0;
                        }
                    }
                    m_aniSaveTime[i] = 0;
                }
            }
        }

        uint8_t frame(size_t mspf, size_t frameCount) const
        {
            fflassert(mspf < 8);
            fflassert(frameCount < 16);
            return m_aniTileFrame[mspf][frameCount];
        }

    public:
        void clear()
        {
            std::memset(m_aniSaveTime,  0, sizeof(m_aniSaveTime));
            std::memset(m_aniTileFrame, 0, sizeof(m_aniTileFrame));
        }
};
