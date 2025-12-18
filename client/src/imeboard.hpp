#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "colorf.hpp"
#include "ime.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"

class IMEBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x   = 0;
            Widget::VarInt y   = 0;

            Widget::FontConfig font
            {
                .id = 1,
                .size = 12,
                .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
            };

            Widget::VarU32 fontColorHover   = colorf::RGBA(0XFF, 0X00, 0X00, 0XFF);
            Widget::VarU32 fontColorPressed = colorf::RGBA(0X00, 0X00, 0XFF, 0XFF);
            Widget::VarU32 separatorColor   = colorf::RGBA(0XFF, 0XFF, 0X00, 0X30);

            Widget::WADPair parent {};
        };

    private:
        IME m_ime;
        bool m_active = true; // needs this because IME is used not only in ProcessRun

    private:
        const Widget::FontConfig m_font;

    private:
        const Widget::VarU32 m_fontColorHover;
        const Widget::VarU32 m_fontColorPressed;
        const Widget::VarU32 m_separatorColor;

    private:
        const size_t m_fontTokenHeight;

    private:
        size_t m_startIndex = 0;

    private:
        const size_t m_startX = 12;
        const size_t m_startY = 10;

        const size_t m_separatorSpace = 4;
        const size_t m_candidateSpace = 5;

    private:
        Widget *m_inputWidget = nullptr;
        std::function<void(std::string)> m_onCommit;

    private:
        std::vector<std::string> m_candidateList;
        std::vector<std::unique_ptr<LabelBoard>> m_boardList;

    private:
        ImageBoard m_upLeftCorner;
        ImageBoard m_downRightCorner;

        ImageBoard m_bgImg;
        GfxResizeBoard m_bg;

    public:
        IMEBoard(IMEBoard::InitArgs);

    public:
        void updateDefault(double) override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void gainFocus(std::string, std::string, Widget *, std::function<void(std::string)>);
        void dropFocus();

    private:
        void prepareLabelBoardList();

    private:
        size_t totalLabelWidth() const;

    public:
        bool active() const
        {
            return m_active;
        }

        void setActive(bool active)
        {
            m_active = active;
        }

        bool switchActive()
        {
            return m_active = !m_active;
        }
};
