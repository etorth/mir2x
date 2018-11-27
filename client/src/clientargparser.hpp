/*
 * =====================================================================================
 *
 *       Filename: clientargparser.hpp
 *        Created: 05/12/2017 16:33:25
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
#include "argparser.hpp"
#include <cstdint>

struct ClientArgParser
{
    const bool EnableDrawMapGrid;       // "--enable-draw-map-grid"
    const bool EnableDrawCreatureCover; // "--enable-draw-creature-cover"
    const bool EnableDrawMouseLocation; // "--enable-draw-mouse-location"

    bool TraceMove;

    ClientArgParser(const arg_parser &rstCmdParser)
        : EnableDrawMapGrid(rstCmdParser["enable-draw-map-grid"])
        , EnableDrawCreatureCover(rstCmdParser["enable-draw-creature-cover"])
        , EnableDrawMouseLocation(rstCmdParser["enable-draw-mouse-location"])
        , TraceMove(rstCmdParser["trace-move"])
    {}
};
