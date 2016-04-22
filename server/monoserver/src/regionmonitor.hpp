/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 04/22/2016 01:46:11
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
#include <cassert>
#include "monitorbase.hpp"

class RegionMonitor: public MonitorBase
{
    public:
        RegionMonitor(const Theron::Address &rstMapAddr)
            : MonitorBase()
            , m_MapAddress(rstMapAddr)
        {}

        virtual ~RegionMonitor() = default;

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    private:
        Theron::Address m_MapAddress;

    private:
        // region it takes in charge
        int     m_X;
        int     m_Y;
        int     m_W;
        int     m_H;

        virtual ~RegionMonitor() = default;

    public:
        void Activate();

    protected:
        // check the monitor is fully inited
        bool Valid()
        {
        }

    public:
        void SetRegion(int nX, int nY, int nW, int nH)
        {
            m_X = nX;
            m_Y = nY;
            m_W = nW;
            m_H = nH;
        }

        // +---+---+---+
        // | 0 | 1 | 2 |
        // +---+---+---+
        // | 7 | x | 3 |
        // +---+---+---+
        // | 6 | 5 | 4 |
        // +---+---+---+
        template<typename T>
        void SetNeighbor(const T< &rstT)
        {
            auto pBegin = rstT.begin();
            int nIndex = 0;

            while(pBegin != rstT.end() && )
        }
};
