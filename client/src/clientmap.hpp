/*
 * =====================================================================================
 *
 *       Filename: clientmap.hpp
 *        Created: 06/02/2016 14:01:46
 *  Last Modified: 03/26/2017 02:32:47
 *
 *    Description: wrapper of mir2xmap for client
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
#include "mir2xmapdata.hpp"

class ClientMap
{
    private:
        uint32_t m_MapID;
        Mir2xMapData m_Mir2xMapData;

    public:
        ClientMap()
            : m_MapID(0)
        {}

       ~ClientMap() = default;

    public:
        uint32_t ID()
        {
            return m_MapID;
        }

    public:
        bool Load(uint32_t);

    public:
        bool Valid()
        {
            return ID() && m_Mir2xMapData.Valid();
        }

        bool ValidP(int nX, int nY) { return ID() && m_Mir2xMapData.ValidP(nX, nY); }
        bool ValidC(int nX, int nY) { return ID() && m_Mir2xMapData.ValidC(nX, nY); }

    public:
        void Draw(int, int, int, int, int, int);
};
