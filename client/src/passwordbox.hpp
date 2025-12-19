#pragma once
#include <string>
#include <functional>
#include "inputline.hpp"

class PasswordBox: public InputLine
{
    private:
        using CursorArgs = InputLine::CursorArgs;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Widget::VarBool security = true;

            Widget::FontConfig font {};
            PasswordBox::CursorArgs cursor {};

            std::function<void()>            onTab    = nullptr;
            std::function<void()>            onCR     = nullptr;
            std::function<void(std::string)> onChange = nullptr;

            Widget::WADPair parent {};
        };

    private:
        Widget::VarBool m_security;

    private:
        std::string m_passwordString;

    public:
        PasswordBox(PasswordBox::InitArgs args)
            : InputLine
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w = std::move(args.w),
                  .h = std::move(args.h),

                  .font = std::move(args.font),
                  .cursor = std::move(args.cursor),

                  .onTab    = std::move(args.onTab),
                  .onCR     = std::move(args.onCR),
                  .onChange = std::move(args.onChange),

                  .parent = std::move(args.parent),
              }}

            , m_security(std::move(args.security))
        {}

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            const auto result = InputLine::processEventDefault(event, valid, m);
            if(security()){
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
            return security() ? m_passwordString : getRawString();
        }

        void clear() override
        {
            InputLine::clear();
            m_passwordString.clear();
        }

    public:
        void setSecurity(Widget::VarBool argSecurity)
        {
            m_security = std::move(argSecurity);
        }

        bool security() const
        {
            return Widget::evalBool(m_security, this);
        }
};
