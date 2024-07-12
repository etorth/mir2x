#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "colorf.hpp"
#include "ime.hpp"
#include "labelboard.hpp"

class IMEBoard: public Widget
{
    private:
        IME m_ime;
        bool m_active = true; // needs this because IME is used not only in ProcessRun

    private:
        const uint8_t m_font;
        const uint8_t m_fontSize;
        const uint8_t m_fontStyle;

        const uint32_t m_fontColor;
        const uint32_t m_fontColorHover;
        const uint32_t m_fontColorPressed;

        const uint32_t m_separatorColor;

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
        std::vector<std::unique_ptr<LabelBoard>> m_labelBoardList;

    public:
        IMEBoard(
                dir8_t, // dir
                int,    // x
                int,    // y

                uint8_t =  1, // font
                uint8_t = 12, // fontSize
                uint8_t =  0, // fontStyle

                uint32_t = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF), // fontColor
                uint32_t = colorf::RGBA(0XFF, 0X00, 0X00, 0XFF), // fontColorHover
                uint32_t = colorf::RGBA(0X00, 0X00, 0XFF, 0XFF), // fontColorPressed

                uint32_t = colorf::RGBA(0XFF, 0XFF, 0X00, 0X30), // separatorColor

                Widget * = nullptr, // parent
                bool     = false);  // autoDelete

    public:
        void update(double) override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void gainFocus(std::string, std::string, Widget *, std::function<void(std::string)>);
        void dropFocus();

    private:
        void updateSize();
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
