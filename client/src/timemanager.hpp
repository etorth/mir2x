/*
 * =====================================================================================
 *
 *       Filename: timemanager.hpp
 *        Created: 01/13/2016 23:12:39
 *  Last Modified: 01/13/2016 23:23:20
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
#include <SDL.h>
#include <limits>
#include <cstdint>
#include "devicemanager.hpp"

class TimeManager final
{
    private:
        TimeManager();
        ~TimeManager();

    public:
        bool Ready()
        {
            return GetDeviceManager()->Ready();
        }

        uint32_t Now()
        {
            if(Ready()){
                return (uint32_t)SDL_GetTicks();
            }else{
                return std::numeric_limits<uint32_t>::max();
            }
        }
};
