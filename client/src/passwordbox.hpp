#pragma once
#include <string>
#include <functional>
#include "inputline.hpp"

class PasswordBox: public InputLine
{
    private:
        bool m_security;
        std::string m_passwordString;

    public:
        PasswordBox(
                dir8_t dir,
                int  x,
                int  y,
                int  w,
                int  h,
                bool security = true,

                uint8_t  font      =  0,
                uint8_t  fontSize  = 10,
                uint8_t  fontStyle =  0,
                uint32_t fontColor =  colorf::WHITE + colorf::A_SHF(255),

                int      cursorWidth = 2,
                uint32_t cursorColor = colorf::WHITE + colorf::A_SHF(255),

                std::function<void()> fnOnTab    = nullptr,
                std::function<void()> fnOnReturn = nullptr,

                Widget *parent     = nullptr,
                bool    autoDelete = false)
            : InputLine
              {
                  dir,
                  x,
                  y,
                  w,
                  h,

                  false,

                  font,
                  fontSize,
                  fontStyle,
                  fontColor,

                  cursorWidth,
                  cursorColor,

                  fnOnTab,
                  fnOnReturn,
                  nullptr,

                  parent,
                  autoDelete,
              }
            , m_security(security)
        {}

    public:
        bool processEventDefault(const SDL_Event &event, bool valid) override
        {
            const auto result = InputLine::processEvent(event, valid);
            if(m_security){
                const auto inputString = getRawString();
                if(inputString.size() + 1 == m_passwordString.size()){
                    // delete one char
                    m_passwordString.erase(m_cursor, 1);
                }

                else if(inputString.size() == m_passwordString.size() + 1){
                    // insert one char
                    m_passwordString.insert(m_cursor - 1, 1, inputString[m_cursor - 1]);
                    deleteChar();
                    insertChar('*');
                }

                else if(inputString.size() != m_passwordString.size()){
                    throw fflerror("password box input error");
                }
            }
            return result;
        }

    public:
        std::string getPasswordString() const
        {
            return m_security ? m_passwordString : getRawString();
        }

        void clear() override
        {
            InputLine::clear();
            m_passwordString.clear();
        }

    public:
        void setSecurity(bool security)
        {
            m_security = security;
        }
};
