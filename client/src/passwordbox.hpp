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

class passwordBox: public inputLine
{
    private:
        bool m_security;

    public:
        passwordBox(
                int  x,
                int  y,
                int  w,
                int  h,
                bool security = true,

                uint8_t  font      =  0,
                uint8_t  fontSize  = 10,
                uint8_t  fontStyle =  0,
                uint32_t fontColor =  ColorFunc::WHITE,

                int      cursorWidth = 2,
                uint32_t cursorColor = ColorFunc::WHITE,

                std::function<void()>  fnOnTab    = [](){},
                std::function<void()>  fnOnReturn = [](){},
                widget                *parent     = nullptr,
                bool                   autoDelete = false)
            : inputLine 
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
