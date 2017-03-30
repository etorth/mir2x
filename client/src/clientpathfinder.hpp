/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.hpp
 *        Created: 03/28/2017 21:13:11
 *  Last Modified: 03/29/2017 00:19:44
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
#include "pathfinder.hpp"

class ClientPathFinder: public AStarPathFinder
{
    private:
        bool m_CheckCreature;

    public:
        ClientPathFinder(bool);
       ~ClientPathFinder() = default;
};
