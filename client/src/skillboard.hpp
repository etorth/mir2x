/*
 * =====================================================================================
 *
 *       Filename: skillboard.hpp
 *        Created: 10/08/2017 19:06:52
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
#include <array>
#include <memory>
#include "strf.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "texvslider.hpp"
#include "dbcomrecord.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class SkillBoard: public Widget
{
    private:
        struct MagicIconData
        {
            // icon width : 40
            // icon height: 40

            uint32_t magicID;
            int level;

            int x;
            int y;

            char key;
            SkillBoard *board;
        };

        class MagicIconButton: public WidgetGroup
        {
            // +-+-----+
            // |A|     |
            // +-+     |
            // |       |
            // +-------+-+
            //         |1|
            //         +-+

            private:
                LabelBoard m_key;
                LabelBoard m_keyShadow;

            private:
                LabelBoard m_level;
                TritexButton m_icon;

            public:
                MagicIconButton(int, int, SkillBoard::MagicIconData *, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void drawEx(int, int, int, int, int, int) override;

            public:
                void setKey(char key)
                {
                    if(key == '\0'){
                        m_key      .setText(u8"");
                        m_keyShadow.setText(u8"");
                    }
                    else{
                        m_key      .setText(u8"%c", key);
                        m_keyShadow.setText(u8"%c", key);
                    }
                }

                void setLevel(int level)
                {
                    switch(level){
                        case 0: m_level.setText(u8""); break;
                        case 1:
                        case 2:
                        case 3: m_level.setText(u8"%d", level); break;
                        default: throw fflerror("invalid skill level: %d", level);
                    }
                }
        };

        class SkillPage: public WidgetGroup
        {
            private:
                const uint32_t m_pageImage;

            public:
                SkillPage(uint32_t, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void addIcon(SkillBoard::MagicIconData *iconDataPtr)
                {
                    if(!iconDataPtr){
                        throw fflerror("invalid icon data pointer: (null)");
                    }

                    new SkillBoard::MagicIconButton
                    {
                        iconDataPtr->x,
                        iconDataPtr->y,
                        iconDataPtr,
                        this,
                        true,
                    };
                }

            public:
                void drawEx(int, int, int, int, int, int) override;
        };

    private:
        int m_magicIconIndex = -1;
        std::vector<MagicIconData> m_magicIconDataList;

    private:
        std::vector<SkillPage *> m_skillPageList;

    private:
        // no need to introduce a new type
        // use two tritex button to micmic the tab button

        int m_tabIndex = 0;
        std::vector<TritexButton *> m_tabButtonList;

    private:
        TexVSlider m_slider;

    private:
        LabelBoard m_textBoard;

    private:
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    public:
        SkillBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;
        void drawEx(int, int, int, int, int, int) override;
        bool processEvent(const SDL_Event &, bool) override;

    private:
        static std::array<int, 4> getPageRectange()
        {
            return {18, 44, 304, 329};
        }

    public:
        void setText(const std::u8string &s)
        {
            m_textBoard.setText(s.c_str());
        }

        int selectedElem() const
        {
            if(m_tabIndex >= 0 && m_tabIndex <= 6){
                return MET_BEGIN + m_tabIndex;
            }
            else if(m_tabIndex == 7){
                return MET_NONE;
            }
            else{
                throw fflerror("invalid tab index: %d", m_tabIndex);
            }
        }

    public:
        uint32_t key2MagicID(char);

    public:
        struct MagicKey
        {
            uint32_t magicID;
            char     key;
        };

        std::vector<MagicKey> getMagicKeyList() const
        {
            std::vector<MagicKey> result;
            for(const auto &iconData: m_magicIconDataList){
                if(iconData.magicID && iconData.key != '\0'){
                    result.emplace_back(iconData.magicID, iconData.key);
                }
            }
            return result;
        }
};
