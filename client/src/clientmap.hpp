/*
 * =====================================================================================
 *
 *       Filename: clientmap.hpp
 *        Created: 06/02/2016 14:01:46
 *  Last Modified: 06/02/2016 23:13:23
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

#include "mir2xmap.hpp"

class ClientMap
{
    private:
        uint32_t m_MapID;
        Mir2xMap m_Mir2xMap;

    public:
        ClientMap()
            : m_MapID(0)
        {}

        ~ClientMap() = default;

    public:
        bool Load(uint32_t);
        bool Valid()
        {
            return m_Mir2xMap.Valid();
        }

        uint32_t ID()
        {
            return m_MapID;
        }

    public:
        void Draw(int nViewX, int nViewY, int nViewW, int nViewH,           // view region
                int nMaxObjW, int nMaxObjH,                                 // operation addition margin
                const std::function<void(int, int, uint32_t)> &fnDrawTile,  //
                const std::function<void(int, int, uint32_t)> &fnDrawObj,   //
                const std::function<void(int, int)> &fnDrawActor,           //
                const std::function<void(int, int)> &fnDrawExt)             //
        {
            m_Mir2xMap.Draw(nViewX, nViewY, nViewW, nViewH,
                    nMaxObjW, nMaxObjH, fnDrawTile, fnDrawObj, fnDrawActor, fnDrawExt);
        }
};
