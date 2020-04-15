/*
 * =====================================================================================
 *
 *       Filename: passwordbox.hpp
 *        Created: 07/16/2017 19:06:25
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
#include <string>
#include <functional>
#include "inputline.hpp"

class PasswordBox: public InputLine
{
    private:
        bool m_security;

    public:
        PasswordBox(
                int  x,
                int  y,
                int  w,
                int  h,
                bool security = true,

                uint8_t  font      =  0,
                uint8_t  fontSize  = 10,
                uint8_t  fontStyle =  0,
                uint32_t fontColor =  colorf::WHITE,

                int      cursorWidth = 2,
                uint32_t cursorColor = colorf::WHITE,

                std::function<void()>  fnOnTab    = [](){},
                std::function<void()>  fnOnReturn = [](){},
                Widget                *parent     = nullptr,
                bool                   autoDelete = false)
            : InputLine 
              {
                  x,
                  y,
                  w,
                  h,

                  font,
                  fontSize,
                  fontStyle,
                  fontColor,

                  cursorWidth,
                  cursorColor,

                  fnOnTab,
                  fnOnReturn,

                  parent,
                  autoDelete,
              }
            , m_security(security)
        {}
};
