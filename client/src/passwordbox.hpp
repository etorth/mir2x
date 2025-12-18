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
                Widget::VarDir argDir,
                Widget::VarInt argX,
                Widget::VarInt argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                bool argSecurity = true,

                uint8_t argFont      =  0,
                uint8_t argFontSize  = 10,
                uint8_t argFontStyle =  0,

                Widget::VarU32 argFontColor = colorf::WHITE_A255,

                int            argCursorWidth = 2,
                Widget::VarU32 argCursorColor = colorf::WHITE_A255,

                std::function<void()> argOnTab    = nullptr,
                std::function<void()> argOnReturn = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : InputLine
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  std::move(argW),
                  std::move(argH),

                  false,

                  argFont,
                  argFontSize,
                  argFontStyle,

                  std::move(argFontColor),

                  argCursorWidth,
                  std::move(argCursorColor),

                  std::move(argOnTab),
                  std::move(argOnReturn),
                  nullptr,

                  argParent,
                  argAutoDelete,
              }

            , m_security(argSecurity)
        {}

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            const auto result = InputLine::processEventDefault(event, valid, m);
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
