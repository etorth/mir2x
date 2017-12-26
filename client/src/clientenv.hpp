/*
 * =====================================================================================
 *
 *       Filename: clientenv.hpp
 *        Created: 05/12/2017 16:33:25
 *  Last Modified: 12/25/2017 19:17:04
 *
 *    Description: use environment to setup the runtime message report:
 *
 *                 MIR2X_INFO_XXXX      : configured one by one
 *                 MIR2X_CONFIG_XXXX    : configured one by one
 *
 *                 MIR2X_DEBUG_XXXX     : enable automatically if MIR2X_DEBUG level OK
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
#include <cstdint>

struct ClientEnv
{
    const std::string DebugArgs;

    const bool EnableDrawMapGrid;       // "--enable-draw-map-grid"
    const bool EnableDrawCreatureCover; // "--enable-draw-creature-cover"
    const bool EnableDrawMouseLocation; // "--enable-draw-mouse-location"

    bool TraceMove;

    ClientEnv()
        : DebugArgs([]() -> std::string
          {
              if(auto pDebugArgs = std::getenv("MIR2X_DEBUG_ARGS")){
                  return std::string(pDebugArgs);
              }else{
                  return std::string("");
              }
          }())
        , EnableDrawMapGrid(CheckBoolArg("--enable-draw-map-grid"))
        , EnableDrawCreatureCover(CheckBoolArg("--enable-draw-creature-cover"))
        , EnableDrawMouseLocation(CheckBoolArg("--enable-draw-mouse-location"))
        , TraceMove(CheckBoolArg("--trace-move"))
    {}

    bool CheckBoolArg(const std::string &szArgName)
    {
        if(szArgName.empty()){
            return false;
        }

        size_t nPos = 0;
        while(true){
            nPos = DebugArgs.find(szArgName, nPos);
            if(nPos == std::string::npos){
                return false;
            }

            // if found, we need to make sure
            auto pEnd = DebugArgs.begin() + nPos + szArgName.size();
            if(pEnd == DebugArgs.end()){
                return true;
            }

            switch(*pEnd){
                case ' ' :
                case '\0':
                case '\t':
                case '\n':
                    {
                        return true;
                    }
                default:
                    {
                        break;
                    }
            }

            // continue to check
            nPos += szArgName.size();
        }

        return false;
    }
};
