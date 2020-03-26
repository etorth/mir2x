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
    const bool enableDrawUID;           // "--enable-draw-uid"
    const bool alwaysDrawName;          // "--always-draw-name"
    const bool EnableDrawMapGrid;       // "--enable-draw-map-grid"
    const bool EnableDrawCreatureCover; // "--enable-draw-creature-cover"
    const bool EnableDrawMouseLocation; // "--enable-draw-mouse-location"
    const bool EnableClientMonitor;     // "--enable-client-monitor"
    const bool drawTokenFrame;          // "--draw-token-frame"
    const bool drawBoardFrame;          // "--draw-board-frame"

    bool traceMove;

    ClientArgParser(const arg_parser &cmdParser)
        : enableDrawUID(cmdParser["enable-draw-uid"])
        , alwaysDrawName(cmdParser["always-draw-name"])
        , EnableDrawMapGrid(cmdParser["enable-draw-map-grid"])
        , EnableDrawCreatureCover(cmdParser["enable-draw-creature-cover"])
        , EnableDrawMouseLocation(cmdParser["enable-draw-mouse-location"])
        , EnableClientMonitor(cmdParser["enable-client-monitor"])
        , drawTokenFrame(cmdParser["draw-token-frame"])
        , drawBoardFrame(cmdParser["draw-board-frame"])
        , traceMove(cmdParser["trace-move"])
    {}
};
