/*
 * =====================================================================================
 *
 *       Filename: pinyinengine.hpp
 *        Created: 03/13/2016 19:18:58
 *  Last Modified: 03/13/2016 19:38:40
 *
 *    Description: pinyin method
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

#include "inputmethodengine.hpp"

class PinyinEngine: public InputMethodEngine
{
    public:
        PinyinEngine();
        virtual ~PinyinEngine();

    public:
        // ctrl/alt/char(aA, 7&)
        void Input(bool, bool, char);

    public:
        std::string Translate();

};
