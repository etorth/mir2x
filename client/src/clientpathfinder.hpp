/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.hpp
 *        Created: 03/28/2017 21:13:11
 *  Last Modified: 05/18/2017 17:18:59
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
    public:
        ClientPathFinder(bool, bool);
       ~ClientPathFinder() = default;
};
