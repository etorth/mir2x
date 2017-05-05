/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.hpp
 *        Created: 03/28/2017 21:13:11
 *  Last Modified: 05/04/2017 19:35:11
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
        ClientPathFinder(bool);
       ~ClientPathFinder() = default;
};
