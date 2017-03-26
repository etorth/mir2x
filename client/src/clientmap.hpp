/*
 * =====================================================================================
 *
 *       Filename: clientmap.hpp
 *        Created: 06/02/2016 14:01:46
 *  Last Modified: 03/25/2017 12:19:25
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
        bool Valid()
        {
            return ID() && m_Mir2xMap.Valid();
        }

    public:
        void Draw(int, int, int, int, int, int,                     // region and operation margin
                const std::function<void(int, int, uint32_t)> &,    // draw tile
                const std::function<void(int, int, uint32_t)> &,    // draw object
                const std::function<void(int, int)> &,              // draw actor
                const std::function<void(int, int)> &);             // draw ext
};
