/*
 * =====================================================================================
 *
 *       Filename: clientmap.hpp
 *        Created: 06/02/2016 14:01:46
 *  Last Modified: 06/02/2016 14:14:45
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
        Mir2xMap m_Mir2xMap;

    public:
        bool Load(uint32_t);
        bool Valid()
        {
            return m_Mir2xMap.Valid();
        }
};
