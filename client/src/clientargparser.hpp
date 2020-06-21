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
    const bool drawUID;                 // "--draw-uid"
    const bool alwaysDrawName;          // "--always-draw-name"
    const bool drawMapGrid;             // "--draw-map-grid"
    const bool drawCreatureCover;       // "--draw-creature-cover"
    const bool drawMouseLocation;       // "--draw-mouse-location"
    const bool enableClientMonitor;     // "--enable-client-monitor"
    const bool drawTokenFrame;          // "--draw-token-frame"
    const bool drawBoardFrame;          // "--draw-board-frame"
    const bool debugAlphaCover;         // "--debug-alpha-cover"
    const bool debugSlider;             // "--debug-slider"
    const bool drawFPS;                 // "--draw-fps"

    bool traceMove;

    ClientArgParser(const arg_parser &cmdParser)
        : drawUID(cmdParser["draw-uid"])
        , alwaysDrawName(cmdParser["always-draw-name"])
        , drawMapGrid(cmdParser["draw-map-grid"])
        , drawCreatureCover(cmdParser["draw-creature-cover"])
        , drawMouseLocation(cmdParser["draw-mouse-location"])
        , enableClientMonitor(cmdParser["enable-client-monitor"])
        , drawTokenFrame(cmdParser["draw-token-frame"])
        , drawBoardFrame(cmdParser["draw-board-frame"])
        , debugAlphaCover(cmdParser["debug-alpha-cover"])
        , debugSlider(cmdParser["debug-slider"])
        , drawFPS(cmdParser["draw-fps"])
        , traceMove(cmdParser["trace-move"])
    {}
};
