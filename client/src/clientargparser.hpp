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
#include <cstdint>
#include "argparser.hpp"

struct ClientArgParser
{
    const bool disableProfiler;         // "--disable-profiler"
    const bool disableVersionCheck;     // "--disable-version-check"
    const bool drawUID;                 // "--draw-uid"
    const bool alwaysDrawName;          // "--always-draw-name"
    const bool drawMapGrid;             // "--draw-map-grid"
    const bool drawMagicGrid;           // "--draw-magic-grid"
    const bool drawHPBar;               // "--draw-hp-bar"
    const bool drawCreatureCover;       // "--draw-creature-cover"
    const bool drawMouseLocation;       // "--draw-mouse-location"
    const bool enableClientMonitor;     // "--enable-client-monitor"
    const bool drawTokenFrame;          // "--draw-token-frame"
    const bool drawTextureAlignLine;    // "--draw-texture-align-line"
    const bool drawBoardFrame;          // "--draw-board-frame"
    const bool drawTargetBox;           // "--draw-target-box"
    const bool debugAlphaCover;         // "--debug-alpha-cover"
    const bool debugDrawTexture;        // "--debug-draw-texture"
    const bool debugDrawInputLine;      // "--debug-draw-input-line"
    const bool debugPlayerStateBoard;   // "--debug-player-state-board"
    const bool debugSlider;             // "--debug-slider"
    const bool drawFPS;                 // "--draw-fps"

    const std::string serverIP;         // "--server-ip"
    const std::string serverPort;       // "--server-port"

    bool traceMove;

    ClientArgParser(const arg_parser &cmdParser)
        : disableProfiler(cmdParser["disable-profiler"])
        , disableVersionCheck(cmdParser["disable-version-check"])
        , drawUID(cmdParser["draw-uid"])
        , alwaysDrawName(cmdParser["always-draw-name"])
        , drawMapGrid(cmdParser["draw-map-grid"])
        , drawMagicGrid(cmdParser["draw-magic-grid"])
        , drawHPBar(cmdParser["draw-hp-bar"])
        , drawCreatureCover(cmdParser["draw-creature-cover"])
        , drawMouseLocation(cmdParser["draw-mouse-location"])
        , enableClientMonitor(cmdParser["enable-client-monitor"])
        , drawTokenFrame(cmdParser["draw-token-frame"])
        , drawTextureAlignLine(cmdParser["draw-texture-align-line"])
        , drawBoardFrame(cmdParser["draw-board-frame"])
        , drawTargetBox(cmdParser["draw-target-box"])
        , debugAlphaCover(cmdParser["debug-alpha-cover"])
        , debugDrawTexture(cmdParser["debug-draw-texture"])
        , debugDrawInputLine(cmdParser["debug-draw-input-line"])
        , debugPlayerStateBoard(cmdParser["debug-player-state-board"])
        , debugSlider(cmdParser["debug-slider"])
        , drawFPS(cmdParser["draw-fps"])
        , serverIP(cmdParser("server-ip").str())
        , serverPort(cmdParser("server-port").str())
        , traceMove(cmdParser["trace-move"])
    {}
};
