/*
 * =====================================================================================
 *
 *       Filename: label.hpp
 *        Created: 8/20/2015 8:59:11 PM
 *  Last Modified: 08/21/2015 7:02:41 PM
 *
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
#include <SDL.h>
#include <vector>
#include <string>
#include "widget.hpp"
#include "tokenbox.hpp"

class Label: public Widget
{
    public:
        Label(const FONTINFO &, const SDL_Color &, const char *);
        ~Label();

    public:
        const char *Content();
        void SetContent(const char *);

    public:
        void Draw();
        void Update(Uint32);
        bool ProcessEvent(SDL_Event &);

    private:
        void Compile();
        void PushBack(TOKENBOX &);
        void SetTokenBoxStartX();
        void LoadUTF8CharBoxCache(TOKENBOX &);

    private:
        // but only UTF8CharBox here
        std::vector<TOKENBOX>       m_Line;
        std::string                 m_Content;
        UTF8CHARTEXTUREINDICATOR    m_CharBoxCache;
};
