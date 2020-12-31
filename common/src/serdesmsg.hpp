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

struct SDNPCXMLLayout
{
    uint64_t npcUID = 0;
    std::string xmlLayout;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, xmlLayout);
    }
};

struct SDNPCSell
{
    uint64_t npcUID = 0;
    std::vector<uint32_t> itemList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, itemList);
    }
};
