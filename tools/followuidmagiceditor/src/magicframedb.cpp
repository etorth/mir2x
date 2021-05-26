/*
 * =====================================================================================
 *
 *       Filename: magicframedb.cpp
 *        Created: 07/26/2017 04:27:57
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

#include <utility>
#include "hexstr.hpp"
#include "magicframedb.hpp"

std::tuple<Fl_Image *, int, int> MagicFrameDB::retrieve(uint32_t texID)
{
    if(auto p = m_cachedFrameList.find(texID); p != m_cachedFrameList.end()){
        return
        {
            p->second.image.get(),
            p->second.dx,
            p->second.dy,
        };
    }

    char keyString[16];
    std::vector<uint8_t> pngBuf;
    MagicFrameDB::CachedFrame cachedPNGFrame;

    if(auto dbFileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(texID, keyString, true), 8, &pngBuf); dbFileName && (std::strlen(dbFileName) >= 18)){
        //
        // [0 ~ 7] [8] [9] [10 ~ 13] [14 ~ 17]
        //  <KEY>  <S> <S>   <+DX>     <+DY>
        //    4    1/2 1/2     2         2
        //
        //   KEY: 3 bytes
        //   S  : sign of DX, take 1 char, 1/2 byte, + for 1, - for 0
        //   S  : sign of DY, take 1 char, 1/2 byte
        //   +DX: abs(DX) take 4 chars, 2 bytes
        //   +DY: abs(DY) take 4 chars, 2 bytes

        cachedPNGFrame.dx = (dbFileName[8] != '0') ? 1 : (-1);
        cachedPNGFrame.dy = (dbFileName[9] != '0') ? 1 : (-1);

        cachedPNGFrame.dx *= to_d(hexstr::to_hex<uint32_t, 2>(dbFileName + 10));
        cachedPNGFrame.dy *= to_d(hexstr::to_hex<uint32_t, 2>(dbFileName + 14));

        cachedPNGFrame.image = std::make_unique<Fl_PNG_Image>(nullptr, pngBuf.data(), pngBuf.size());
    }

    // alwasy put this into the image db
    // otherwise we do endless loading while the given texID is not valid in the database

    auto result = m_cachedFrameList.emplace(texID, std::move(cachedPNGFrame));
    return
    {
        result.first->second.image.get(),
        result.first->second.dx,
        result.first->second.dy,
    };
}
