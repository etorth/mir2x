/*
 * =====================================================================================
 *
 *       Filename: sdlkeyeventchar.cpp
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
#include "sdlkeyeventchar.hpp"
    
char SDLKeyEventChar(const SDL_Event &stEvent)
{
    static const std::unordered_map<SDL_Keycode, const char *> stLoopupTable
    {
        {SDLK_SPACE, "  "},
        {SDLK_QUOTE, "'\""},
        {SDLK_COMMA, ",<"},
        {SDLK_MINUS, "-_"},
        {SDLK_PERIOD, ".>"},
        {SDLK_SLASH, "/?"},
        {SDLK_0, "0)"},
        {SDLK_1, "1!"},
        {SDLK_2, "2@"},
        {SDLK_3, "3#"},
        {SDLK_4, "4$"},
        {SDLK_5, "5%"},
        {SDLK_6, "6^"},
        {SDLK_7, "7&"},
        {SDLK_8, "8*"},
        {SDLK_9, "9("},
        {SDLK_SEMICOLON, ";:"},
        {SDLK_EQUALS, "=+"},
        {SDLK_LEFTBRACKET, "[{"},
        {SDLK_BACKSLASH, "\\|"},
        {SDLK_RIGHTBRACKET, "]}"},
        {SDLK_BACKQUOTE, "`~"},
        {SDLK_a, "aA"},
        {SDLK_b, "bB"},
        {SDLK_c, "cC"},
        {SDLK_d, "dD"},
        {SDLK_e, "eE"},
        {SDLK_f, "fF"},
        {SDLK_g, "gG"},
        {SDLK_h, "hH"},
        {SDLK_i, "iI"},
        {SDLK_j, "jJ"},
        {SDLK_k, "kK"},
        {SDLK_l, "lL"},
        {SDLK_m, "mM"},
        {SDLK_n, "nN"},
        {SDLK_o, "oO"},
        {SDLK_p, "pP"},
        {SDLK_q, "qQ"},
        {SDLK_r, "rR"},
        {SDLK_s, "sS"},
        {SDLK_t, "tT"},
        {SDLK_u, "uU"},
        {SDLK_v, "vV"},
        {SDLK_w, "wW"},
        {SDLK_x, "xX"},
        {SDLK_y, "yY"},
        {SDLK_z, "zZ"},
    };

    auto pRecordInst = stLoopupTable.find(stEvent.key.keysym.sym);
    if(pRecordInst != stLoopupTable.end()){
        if((stEvent.key.keysym.mod & KMOD_LSHIFT) || (stEvent.key.keysym.mod & KMOD_RSHIFT)){
            return pRecordInst->second[1];
        }else{
            return pRecordInst->second[0];
        }
    }
    return '\0';
}
