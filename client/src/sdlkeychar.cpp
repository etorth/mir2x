/*
 * =====================================================================================
 *
 *       Filename: sdlkeychar.cpp
 *        Created: 03/12/2016 19:31:23
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

#include <unordered_map>
#include "sdlkeychar.hpp"

char sdlKeyChar(const SDL_Event &event)
{
    const static std::unordered_map<SDL_Keycode, const char *> s_lookupTable
    {
        {SDLK_SPACE,        " "   " " },
        {SDLK_QUOTE,        "'"   "\""},
        {SDLK_COMMA,        ","   "<" },
        {SDLK_MINUS,        "-"   "_" },
        {SDLK_PERIOD,       "."   ">" },
        {SDLK_SLASH,        "/"   "?" },
        {SDLK_0,            "0"   ")" },
        {SDLK_1,            "1"   "!" },
        {SDLK_2,            "2"   "@" },
        {SDLK_3,            "3"   "#" },
        {SDLK_4,            "4"   "$" },
        {SDLK_5,            "5"   "%" },
        {SDLK_6,            "6"   "^" },
        {SDLK_7,            "7"   "&" },
        {SDLK_8,            "8"   "*" },
        {SDLK_9,            "9"   "(" },
        {SDLK_SEMICOLON,    ";"   ":" },
        {SDLK_EQUALS,       "="   "+" },
        {SDLK_LEFTBRACKET,  "["   "{" },
        {SDLK_BACKSLASH,    "\\"  "|" },
        {SDLK_RIGHTBRACKET, "]"   "}" },
        {SDLK_BACKQUOTE,    "`"   "~" },
        {SDLK_a,            "a"   "A" },
        {SDLK_b,            "b"   "B" },
        {SDLK_c,            "c"   "C" },
        {SDLK_d,            "d"   "D" },
        {SDLK_e,            "e"   "E" },
        {SDLK_f,            "f"   "F" },
        {SDLK_g,            "g"   "G" },
        {SDLK_h,            "h"   "H" },
        {SDLK_i,            "i"   "I" },
        {SDLK_j,            "j"   "J" },
        {SDLK_k,            "k"   "K" },
        {SDLK_l,            "l"   "L" },
        {SDLK_m,            "m"   "M" },
        {SDLK_n,            "n"   "N" },
        {SDLK_o,            "o"   "O" },
        {SDLK_p,            "p"   "P" },
        {SDLK_q,            "q"   "Q" },
        {SDLK_r,            "r"   "R" },
        {SDLK_s,            "s"   "S" },
        {SDLK_t,            "t"   "T" },
        {SDLK_u,            "u"   "U" },
        {SDLK_v,            "v"   "V" },
        {SDLK_w,            "w"   "W" },
        {SDLK_x,            "x"   "X" },
        {SDLK_y,            "y"   "Y" },
        {SDLK_z,            "z"   "Z" },
    };

    if(const auto p = s_lookupTable.find(event.key.keysym.sym); p != s_lookupTable.end()){
        return p->second[((event.key.keysym.mod & KMOD_LSHIFT) || (event.key.keysym.mod & KMOD_RSHIFT)) ? 1 : 0];
    }
    return '\0';
}
