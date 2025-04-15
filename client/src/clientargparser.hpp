#pragma once
#include <cstdint>
#include <utility>
#include <optional>
#include "fflerror.hpp"
#include "argf.hpp"

struct ClientArgParser
{
    const bool disableProfiler;         // "--disable-profiler"
    const bool disableVersionCheck;     // "--disable-version-check"
    const bool disableAudio;            // "--disable-audio"
    const bool disableTypesetCache;     // "--disable-typeset-cache"
    const bool traceMove;               // "--trace-move"
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
    const int  screenMode;              // "--screen-mode"

    const std::string serverIP;            // "--server-ip"
    const std::pair<int, bool> serverPort; // "--server-port"
                                           //
    const std::string inputScript;                                      // "--input-script"
    const std::optional<std::pair<std::string, std::string>> autoLogin; // "--auto-login"

    ClientArgParser(const argf::parser &parser)
        : disableProfiler      (argf::parseInteger<bool>(parseString(parser, "--disable-profiler",         argf::OPT, true), "disable-profiler",         argf::checkPass<bool>, false, true).first)
        , disableVersionCheck  (argf::parseInteger<bool>(parseString(parser, "--disable-version-check",    argf::OPT, true), "disable-version-check",    argf::checkPass<bool>, false, true).first)
        , disableAudio         (argf::parseInteger<bool>(parseString(parser, "--disable-audio",            argf::OPT, true), "disable-audio",            argf::checkPass<bool>, false, true).first)
        , disableTypesetCache  (argf::parseInteger<bool>(parseString(parser, "--disable-typeset-cache",    argf::OPT, true), "disable-typeset-cache",    argf::checkPass<bool>, false, true).first)
        , traceMove            (argf::parseInteger<bool>(parseString(parser, "--trace-move",               argf::OPT, true), "trace-move",               argf::checkPass<bool>, false, true).first)
        , drawUID              (argf::parseInteger<bool>(parseString(parser, "--draw-uid",                 argf::OPT, true), "draw-uid",                 argf::checkPass<bool>, false, true).first)
        , alwaysDrawName       (argf::parseInteger<bool>(parseString(parser, "--always-draw-name",         argf::OPT, true), "always-draw-name",         argf::checkPass<bool>, false, true).first)
        , drawMapGrid          (argf::parseInteger<bool>(parseString(parser, "--draw-map-grid",            argf::OPT, true), "draw-map-grid",            argf::checkPass<bool>, false, true).first)
        , fillMapBlockGrid     (argf::parseInteger<bool>(parseString(parser, "--fill-map-block-grid",      argf::OPT, true), "fill-map-block-grid",      argf::checkPass<bool>, false, true).first)
        , drawMagicGrid        (argf::parseInteger<bool>(parseString(parser, "--draw-magic-grid",          argf::OPT, true), "draw-magic-grid",          argf::checkPass<bool>, false, true).first)
        , drawTranspGrid       (argf::parseInteger<bool>(parseString(parser, "--draw-transp-grid",         argf::OPT, true), "draw-transp-grid",         argf::checkPass<bool>, false, true).first)
        , drawHPBar            (argf::parseInteger<bool>(parseString(parser, "--draw-hp-bar",              argf::OPT, true), "draw-hp-bar",              argf::checkPass<bool>, false, true).first)
        , drawCreatureCover    (argf::parseInteger<bool>(parseString(parser, "--draw-creature-cover",      argf::OPT, true), "draw-creature-cover",      argf::checkPass<bool>, false, true).first)
        , drawMouseLocation    (argf::parseInteger<bool>(parseString(parser, "--draw-mouse-location",      argf::OPT, true), "draw-mouse-location",      argf::checkPass<bool>, false, true).first)
        , enableClientMonitor  (argf::parseInteger<bool>(parseString(parser, "--enable-client-monitor",    argf::OPT, true), "enable-client-monitor",    argf::checkPass<bool>, false, true).first)
        , drawTokenFrame       (argf::parseInteger<bool>(parseString(parser, "--draw-token-frame",         argf::OPT, true), "draw-token-frame",         argf::checkPass<bool>, false, true).first)
        , drawTextureAlignLine (argf::parseInteger<bool>(parseString(parser, "--draw-texture-align-line",  argf::OPT, true), "draw-texture-align-line",  argf::checkPass<bool>, false, true).first)
        , drawBoardFrame       (argf::parseInteger<bool>(parseString(parser, "--draw-board-frame",         argf::OPT, true), "draw-board-frame",         argf::checkPass<bool>, false, true).first)
        , drawTargetBox        (argf::parseInteger<bool>(parseString(parser, "--draw-target-box",          argf::OPT, true), "draw-target-box",          argf::checkPass<bool>, false, true).first)
        , debugAlphaCover      (argf::parseInteger<bool>(parseString(parser, "--debug-alpha-cover",        argf::OPT, true), "debug-alpha-cover",        argf::checkPass<bool>, false, true).first)
        , debugDrawTexture     (argf::parseInteger<bool>(parseString(parser, "--debug-draw-texture",       argf::OPT, true), "debug-draw-texture",       argf::checkPass<bool>, false, true).first)
        , debugDrawInputLine   (argf::parseInteger<bool>(parseString(parser, "--debug-draw-input-line",    argf::OPT, true), "debug-draw-input-line",    argf::checkPass<bool>, false, true).first)
        , debugPlayerStateBoard(argf::parseInteger<bool>(parseString(parser, "--debug-player-state-board", argf::OPT, true), "debug-player-state-board", argf::checkPass<bool>, false, true).first)
        , debugSlider          (argf::parseInteger<bool>(parseString(parser, "--debug-slider",             argf::OPT, true), "debug-slider",             argf::checkPass<bool>, false, true).first)
        , debugClickEvent      (argf::parseInteger<bool>(parseString(parser, "--debug-click-event",        argf::OPT, true), "debug-click-event",        argf::checkPass<bool>, false, true).first)
        , screenMode           (argf::parseInteger<int> (parseString(parser, "--screen-mode",              argf::OPT      ), "screen-mode",              checkScreenMode,           0,    0).first)

        , serverIP(parseString(parser, "--server-ip", argf::REQ).value())
        , serverPort(argf::parseInteger<int>(parseString(parser, "--server-port", argf::OPT), "server-port", argf::checkUserListenPort(false), argf::defVal::clientPort))

        , inputScript(parseString(parser, "--input-script", argf::OPT).value_or(std::string{}))
        , autoLogin([&parser]() -> std::optional<std::pair<std::string, std::string>>
          {
              if(const auto autoLoginStr = parseString(parser, "--auto-login", argf::OPT); autoLoginStr.has_value()){
                  const auto pos = autoLoginStr.value().find(':');

                  if(pos == std::string::npos ||
                     pos == 0                 ||
                     pos == autoLoginStr.value().size() - 1) throw fflerror("usage: --auto-login=id:password");

                  return std::make_pair(autoLoginStr.value().substr(0, pos), autoLoginStr.value().substr(pos + 1));
              }
              return std::nullopt;
          }())
    {}

    static int checkScreenMode(const char *, int screenMode)
    {
        switch(screenMode){
            case 0:
            case 1:
            case 2: return screenMode;
            default: throw fflerror("invalid screen mode: %d", screenMode);
        }
    }

    static std::optional<std::string> parseString(const argf::parser &parser, const std::string &opt, int optChoice, bool allowEmpty = false)
    {
        fflassert(str_haschar(opt));
        const auto optVal = parser.get_option(opt);

        switch(optChoice){
            case argf::REQ:
                {
                    if(!optVal.has_value()){
                        throw fflerror("missing required option: %s", opt.c_str());
                    }
                    break;
                }
            case argf::BAN:
                {
                    if(optVal.has_value()){
                        throw fflerror("invalid option: %s", opt.c_str());
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(optVal.has_value() && optVal.value().empty() && !allowEmpty){
            throw fflerror("invalid empty option value: %s", opt.c_str());
        }
        return optVal;
    }
};
