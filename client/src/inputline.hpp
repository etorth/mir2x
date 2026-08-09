#pragma once
#include <functional>
#include <string>
#include "colorf.hpp"
#include "widget.hpp"
#include "xmltypeset.hpp"
#include "ime.hpp"

class InputLine: public Widget
{
    public:
        struct CursorArgs final
        {
            Widget::VarSize w = 2;
            Widget::VarU32 color = colorf::WHITE_A255;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Widget::VarInt enableIME = IME_DISABLE;

            Widget::FontConfig font {};
            InputLine::CursorArgs cursor {};

            std::function<void()>            onTab    = nullptr;
            std::function<void()>            onCR     = nullptr;
            std::function<void(std::string)> onChange = nullptr;
            std::function<bool(const std::string &, const std::string &)> validate = nullptr;

            Widget::WADPair parent {};
        };

    protected:
        Widget::VarInt m_imeEnabled;

    protected:
        XMLTypeset m_tpset;

    protected:
        int m_cursor = 0;

    protected:
        double m_cursorBlink = 0.0;
        InputLine::CursorArgs m_cursorArgs;

    protected:
        std::function<void()>            m_onTab;
        std::function<void()>            m_onCR;
        std::function<void(std::string)> m_onChange;
        std::function<bool(const std::string &, const std::string &)> m_validate;

    protected:
        bool validateInput(const std::string &, const std::string &) const;
        virtual bool insertInput(const std::string &);
        virtual bool deleteInput();

        void notifyInputChange() const;

    public:
        InputLine(InputLine::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setFocus(bool) override;

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void updateDefault(double ms) override
        {
            m_cursorBlink += ms;
        }

    public:
        std::string getRawString() const
        {
            return m_tpset.getRawString();
        }

    // public:
    //     int w() const override
    //     {
    //         return varWOpt() ? m_tpset.pw() : Widget::w();
    //     }
    //
    //     int h() const override
    //     {
    //         if(varHOpt()){
    //             if(m_tpset.empty()){
    //                 return m_tpset.getDefaultFontHeight();
    //             }
    //             return m_tpset.ph();
    //         }
    //         return Widget::h();
    //     }

    public:
        virtual void clear();

        void setValidateFunc(std::function<bool(const std::string &, const std::string &)>);

    public:
        void deleteChar();
        void insertChar(char);
        void setInput(const char *);
        void insertUTF8String(const char *);
};
