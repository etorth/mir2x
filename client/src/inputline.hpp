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
            Widget::VarU32 color = colorf::WHITE_A255;
            Widget::VarDir align = DIR_NONE;

            Widget::VarSize w = 2;
            Widget::VarBool lazy = true;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Widget::VarDirOpt align = std::nullopt; // tpset align
            Widget::VarInt enableIME = IME_DISABLE;

            Widget::FontConfig font {};
            InputLine::CursorArgs cursor {};

            std::function<void()>                         onTab    = nullptr;
            std::function<void()>                         onCR     = nullptr;
            std::function<void(std::string)>              onChange = nullptr;
            std::function<bool(std::string, std::string)> validate = nullptr;

            Widget::WADPair parent {};
        };

    protected:
        Widget::VarDirOpt m_tpsetAlign;

    protected:
        Widget::VarInt m_imeEnabled;

    protected:
        XMLTypeset m_tpset;

    protected:
        int m_cursor = 0;

    protected:
        std::optional<int> m_tpsetXOpt = std::nullopt; // use opt, so no need to initialize in ctor

    protected:
        double m_cursorBlink = 0.0;
        InputLine::CursorArgs m_cursorArgs;

    protected:
        std::function<void()> m_onTab;
        std::function<void()> m_onCR;
        std::function<void(std::string)> m_onChange;
        std::function<bool(std::string, std::string)> m_validate;

    protected:
        bool validateInput(const std::string &, const std::string &) const;
        virtual bool insertInput(const std::string &);
        virtual bool deleteInput();

    protected:
        void notifyInputChange() const;

    protected:
        void adjustCursorPLocX();

    protected:
        int tpsetXFromOpt() const;

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

    public:
        int w() const override
        {
            return varWOpt() ? m_tpset.fw() : Widget::w();
        }

        int h() const override
        {
            if(varHOpt()){
                return m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.fh();
            }
            return Widget::h();
        }

    public:
        virtual void clear();

    public:
        void setValidateFunc(std::function<bool(std::string, std::string)>);

    public:
        void deleteChar();
        void insertChar(char);
        void setInput(const char *);
        void insertUTF8String(const char *);

    public:
        const auto &tpset() const
        {
            return m_tpset;
        }

    public:
        int tpx() const; // XMLTypeset up-left corner location corresponding to InputLine up-left corner
        int tpy() const;

    public:
        int baselineY() const
        {
            return std::get<0>(m_tpset.getDefaultFontHk()) - 1;
        }

    public:
        std::tuple<int, int, int, int> getCursorPLoc() const;
        std::tuple<int, int, int, int> getCursorPLocInXMLTypeset() const;
};
