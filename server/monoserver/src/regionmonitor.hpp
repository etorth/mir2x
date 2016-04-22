/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 04/22/2016 10:53:36
 *
 *    Description: at the beginning I was thinking to init region monitro first, to
 *                 set all region/neighbor, and then call Activate(), then I found
 *                 that ``when you have the address, you already activated it", so
 *                 I need to use message to pass the address to it. Since so, I just
 *                 put all initialization work into message.
 *
 *                 but object with ObjectPod is something init first, and then sumbit
 *                 to interal thread pool. and other thread can't touch this object
 *                 anymore.
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

#include "monitorbase.hpp"

class RegionMonitor: public MonitorBase
{
    private:
        Theron::Address m_MapAddress;

    private:
        // +---+---+---+
        // | 0 | 1 | 2 |
        // +---+---+---+
        // | 7 | x | 3 |
        // +---+---+---+
        // | 6 | 5 | 4 |
        // +---+---+---+
        std::array<Theron::Address, 8> m_NeighborV;

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
            : MonitorBase()
            , m_MapAddress(rstMapAddr)
            , m_X(0)
            , m_Y(0)
            , m_W(0)
            , m_H(0)
            , m_RegionDone(false)
            , m_NeighborDone(0)
        {
            m_NeighborV.fill(Theron::Address::Null());
        }

        virtual ~RegionMonitor()
        {
            delete m_ObjectPod;
        }

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    public:
        Theron::Address Activate()
        {
            auto stAddr = MonitorBase::Activate();
            if(stAddr != Theron::Address::Null()){
                m_ObjectPod->Send(MessagePack(MPK_ACTIVATE), m_MapAddress);
            }
            return stAddr;
        }
};
