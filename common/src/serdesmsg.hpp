/*
 * =====================================================================================
 *
 *       Filename: serdesmsg.hpp
 *        Created: 01/24/2016 19:30:45
 *    Description: net message used by client and mono-server
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
#include <string>
#include "cerealf.hpp"

struct NPCXMLLayout
{
    uint64_t npcUID;
    std::string xmlLayout;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, xmlLayout);
    }
};
