/*
 * =====================================================================================
 *
 *       Filename: shadow.hpp
 *        Created: 06/22/2017 17:02:45
 *  Last Modified: 06/22/2017 17:42:57
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
#include <cstdint>
#include <cinttypes>

namespace Shadow
{
    uint32_t *MakeShadow(uint32_t *,    // dst buffer, allocate new if null provided
            bool,                       // project
            const uint32_t *,           // src buffer
            int,                        // src width
            int,                        // src height
            int *,                      // new width
            int *,                      // new height
            uint32_t);                  // shadow pixel color
}
