/*
 * =====================================================================================
 *
 *       Filename: colorf.cpp
 *        Created: 03/31/2016 19:48:57
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

#include <regex>
#include "strf.hpp"
#include "colorf.hpp"
#include "fflerror.hpp"

#define _REGEX_COL_0_255 R"#(((?:0x[0-9a-f]{1,2})|(?:\d)|(?:[^0]\d)|(?:1\d\d)|(?:2[0-4]\d)|(?:25[0-5])))#"
//                           -   ---------------     --     ------     -----     --------     -------
//                           ^          ^            ^         ^         ^          ^            ^
//                           |          |            |         |         |          |            |
//                           |          |            |         |         |          |            +----------  250  ~ 255
//                           |          |            |         |         |          +-----------------------  200  ~ 249
//                           |          |            |         |         +----------------------------------  100  ~ 199
//                           |          |            |         +--------------------------------------------  19   ~ 99
//                           |          |            +------------------------------------------------------  0    ~ 9
//                           |          +-------------------------------------------------------------------  0x00 ~ 0xff
//                           +------------------------------------------------------------------------------  create a group

#define _try_match_named_color(color, colorName, baseColor) \
do{ \
    const static std::regex colorNameRegex("^" colorName "$", std::regex::icase); \
    if(std::regex_match(color, colorNameRegex)){ \
        return baseColor + A_SHF(255); \
    } \
\
    const static std::regex colorNameAlphaRegex("^" colorName R"#(\+)#" _REGEX_COL_0_255 "$", std::regex::icase); \
    if(std::regex_match(color, colorNameAlphaRegex)){ \
        return baseColor + A_SHF(std::stoi(std::strstr(color, "+"), 0, 0)); \
    } \
}while(0)

uint32_t colorf::string2RGBA(const char *color)
{
    fflassert(str_haschar(color));

    // only for some color
    // check www.w3schools.com/cssref/css_colors.asp

    _try_match_named_color(color, "red"    ,  colorf::RED    );
    _try_match_named_color(color, "green"  ,  colorf::GREEN  );
    _try_match_named_color(color, "blue"   ,  colorf::BLUE   );
    _try_match_named_color(color, "yellow" ,  colorf::YELLOW );
    _try_match_named_color(color, "cyan"   ,  colorf::CYAN   );
    _try_match_named_color(color, "magenta",  colorf::MAGENTA);
    _try_match_named_color(color, "black"  ,  colorf::BLACK  );
    _try_match_named_color(color, "grey"   ,  colorf::GREY   );
    _try_match_named_color(color, "white"  ,  colorf::WHITE  );

    std::string colorString = color;
    std::match_results<std::string::iterator> matchResult;

    // matches RGB(xx, xx, xx)
    const static std::regex rgbExpr(R"#(^\s*rgb\s*\(\s*)#" _REGEX_COL_0_255 R"#(\s*,\s*)#" _REGEX_COL_0_255 R"#(\s*,\s*)#" _REGEX_COL_0_255 R"#(\s*\)\s*$)#", std::regex::icase);

    if(std::regex_match(colorString.begin(), colorString.end(), matchResult, rgbExpr)){
        int r = 0;
        int g = 0;
        int b = 0;
        for(int i = 0; const auto &m: matchResult){
            switch(i++){
                case 1 : r = std::stoi(m.str(), 0, 0); break;
                case 2 : g = std::stoi(m.str(), 0, 0); break;
                case 3 : b = std::stoi(m.str(), 0, 0); break;
                default:                               break;
            }
        }

        if(true
                && r >= 0 && r < 256
                && g >= 0 && g < 256
                && b >= 0 && b < 256){ // should always be true if the regex is correct
            return colorf::RGBA(r, g, b, 255);
        }
        throw bad_reach();
    }

    // matches RGBA(xx, xx, xx, xx)
    std::regex rgbaExpr(R"#(^\s*rgba\s*\(\s*)#" _REGEX_COL_0_255 R"#(\s*,\s*)#" _REGEX_COL_0_255 R"#(\s*,\s*)#" _REGEX_COL_0_255 R"#(\s*,\s*)#" _REGEX_COL_0_255 R"#(\s*\)\s*$)#", std::regex::icase);

    if(std::regex_match(colorString.begin(), colorString.end(), matchResult, rgbaExpr)){
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;
        for(int i = 0; const auto &m: matchResult){
            switch(i++){
                case 1 : r = std::stoi(m.str(), 0, 0); break;
                case 2 : g = std::stoi(m.str(), 0, 0); break;
                case 3 : b = std::stoi(m.str(), 0, 0); break;
                case 4 : a = std::stoi(m.str(), 0, 0); break;
                default:                               break;
            }
        }

        if(true
                && r >= 0 && r < 256
                && g >= 0 && g < 256
                && b >= 0 && b < 256
                && a >= 0 && a < 256){ // should always be true if the regex is correct
            return colorf::RGBA(r, g, b, a);
        }
        throw bad_reach();
    }
    throw fflerror("invalid color string: %s", color);
}
