#pragma once
#include <cstdint>
#include <utility>
#include <optional>
#include "fflerror.hpp"
#include "argparser.hpp"

struct ClientArgParser
{
    const bool disableProfiler;         // "--disable-profiler"
    const bool disableVersionCheck;     // "--disable-version-check"
    const bool disableAudio;            // "--disable-audio"
    const bool drawUID;                 // "--draw-uid"
    const bool alwaysDrawName;          // "--always-draw-name"
    const bool drawMapGrid;             // "--draw-map-grid"
    const bool fillMapBlockGrid;        // "--fill-map-block-grid"
    const bool drawMagicGrid;           // "--draw-magic-grid"
    const bool drawTranspGrid;          // "--draw-transp-grid"
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
    const bool debugClickEvent;         // "--debug-click-event"

    const std::string serverIP;         // "--server-ip"
    const std::string serverPort;       // "--server-port"

    const std::optional<std::string> inputScript;                       // "--input-script"
    const std::optional<std::pair<std::string, std::string>> autoLogin; // "--auto-login"

    bool traceMove;

    ClientArgParser(const arg_parser &cmdParser)
        : disableProfiler(cmdParser["disable-profiler"])
        , disableVersionCheck(cmdParser["disable-version-check"])
        , disableAudio(cmdParser["disable-audio"])
        , drawUID(cmdParser["draw-uid"])
        , alwaysDrawName(cmdParser["always-draw-name"])
        , drawMapGrid(cmdParser["draw-map-grid"])
        , fillMapBlockGrid(cmdParser["fill-map-block-grid"])
        , drawMagicGrid(cmdParser["draw-magic-grid"])
        , drawTranspGrid(cmdParser["draw-transp-grid"])
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
        , debugClickEvent(cmdParser["debug-click-event"])
        , serverIP(cmdParser("server-ip").str())
        , serverPort(cmdParser("server-port").str())
        , inputScript([&cmdParser]() -> std::optional<std::string>
          {
              if(const auto inputScript = cmdParser("input-script").str(); !inputScript.empty()){
                  return inputScript;
              }
              return std::nullopt;
          }())
        , autoLogin([&cmdParser]() -> std::optional<std::pair<std::string, std::string>>
          {
              if(const auto autoLoginStr = cmdParser("auto-login").str(); !autoLoginStr.empty()){
                  const auto pos = autoLoginStr.find(':');

                  if(pos == std::string::npos ||
                     pos == 0                 ||
                     pos == autoLoginStr.size() - 1) throw fflerror("usage: --auto-login=id:password");

                  return std::make_pair(autoLoginStr.substr(0, pos), autoLoginStr.substr(pos + 1));
              }
              return std::nullopt;
          }())
        , traceMove(cmdParser["trace-move"])
    {}
};
