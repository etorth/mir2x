/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 05/02/2016 22:55:05
 *
 *    Description: at the beginning I was thinking to init region monitro first, to
 *                 set all region/neighbor, and then call Activate(), then I found
 *                 that ``when you have the address, you already activated it", so
 *                 I need to use message to pass the address to it. Since so, I just
 *                 put all initialization work into message.
 *
 *                 RegionMonitor is an transponder, which has no UID()/AddTime(), but
 *                 it has an actor pod to response to message
 *
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
#include <array>
#include <cassert>
#include <Theron/Theron.h>

#include "transponder.hpp"

class RegionMonitor: public Transponder
{
    private:
        Theron::Address m_MapAddress;

    private:
        // +---+---+---+
        // | 0 | 1 | 2 |     arrange it in the form:
        // +---+---+---+        for(nDY = -1; nDY <= 1; ++nDY){
        // | 3 | x | 4 |            for(nDX = -1; nDX <= 1; ++nDX){
        // +---+---+---+                ....
        // | 5 | 6 | 7 |
        // +---+---+---+
        //
        std::array<std::array<Theron::Address, 3>, 3> m_NeighborV2D;

    private:
        // region it takes in charge
        int     m_X;
        int     m_Y;
        int     m_W;
        int     m_H;

        bool    m_RegionDone;
        bool    m_NeighborDone;

    public:
        RegionMonitor(const Theron::Address &rstMapAddr)
            : Transponder()
            , m_MapAddress(rstMapAddr)
            , m_X(0)
            , m_Y(0)
            , m_W(0)
            , m_H(0)
            , m_RegionDone(false)
            , m_NeighborDone(0)
        {
            for(size_t nY = 0; nY < 3; ++nY){
                for(size_t nX = 0; nX < 3; ++nX){
                    m_NeighborV2D[nY][nX] = Theron::Address::Null();
                }
            }
        }

        virtual ~RegionMonitor() = default;

    public:
        Theron::Address Activate();
        void Operate(const MessagePack &, const Theron::Address &);
};
