#pragma once
#include <iterator>
#include <string>
#include <functional>
#include <utf8.h>
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
            std::function<bool(const std::string &, const std::string &)> validate = nullptr;

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
                  .validate = std::move(args.validate),

                  .parent = std::move(args.parent),
              }}

            , m_security(std::move(args.security))
        {}

    protected:
        bool insertInput(const std::string &input) override
        {
            if(!security()){
                return InputLine::insertInput(input);
            }

            if(input.empty() || !validateInput(m_passwordString, input)){
                return false;
            }

            auto insertPos = m_passwordString.begin();
            utf8::advance(insertPos, m_cursor, m_passwordString.end());
            const auto insertOff = std::distance(m_passwordString.begin(), insertPos);
            const auto inputLength = utf8::distance(input.begin(), input.end());

            m_passwordString.insert(to_uz(insertOff), input);
            m_cursor += m_tpset.insertUTF8String(m_cursor, 0, std::string(to_uz(inputLength), '*').c_str());
            return true;
        }

        bool deleteInput() override
        {
            if(!security()){
                return InputLine::deleteInput();
            }

            if(m_cursor <= 0){
                return false;
            }

            auto eraseBegin = m_passwordString.begin();
            utf8::advance(eraseBegin, m_cursor - 1, m_passwordString.end());

            auto eraseEnd = eraseBegin;
            utf8::advance(eraseEnd, 1, m_passwordString.end());

            m_passwordString.erase(
                    to_uz(std::distance(m_passwordString.begin(), eraseBegin)),
                    to_uz(std::distance(eraseBegin, eraseEnd)));
            return InputLine::deleteInput();
        }

    public:
        std::string getPasswordString() const
        {
            return security() ? m_passwordString : getRawString();
        }

        void clear() override
        {
            m_passwordString.clear();
            InputLine::clear();
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
