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
#include <list>
#include <array>
#include <memory>
#include "strf.hpp"
#include "mathf.hpp"
#include "widget.hpp"
#include "raiitimer.hpp"
#include "labelboard.hpp"
#include "texvslider.hpp"
#include "dbcomrecord.hpp"
#include "tritexbutton.hpp"
#include "labelshadowboard.hpp"

class ProcessRun;
class SkillBoard: public Widget
{
    private:
        struct MagicIconData
        {
            // icon width : 40
            // icon height: 40

            const uint32_t magicID = 0;
            const int x = 0;
            const int y = 0;

            int level = 0;      //  0 means magic is not valid for this user
            char key  = '\0';   // \0 means magic is not enabled, if level > 0

            int coolDown = 1000;        // cool down time in milliseconds
            hres_timer lastCastTime{};  // last time cast this magic

            int angle() const
            {
                if(level <= 0){
                    return 0;
                }

                if(coolDown <= 0 || lastCastTime.diff_msec() >= to_u64(coolDown)){
                    return 360;
                }

                return mathf::bound<int>(std::lround(360.0 * to_df(lastCastTime.diff_msec()) / coolDown), 0, 360);
            }
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
                SkillBoard::MagicIconData *m_magicIconDataPtr = nullptr;

            private:
                TritexButton m_icon;

            public:
                MagicIconButton(int, int, SkillBoard::MagicIconData *, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void drawEx(int, int, int, int, int, int) const override;

            public:
                bool processEvent(const SDL_Event &, bool) override;

            public:
                bool cursorOn() const
                {
                    return m_icon.getState() != BEVENT_OFF;
                }

                const auto getMagicIconDataPtr() const
                {
                    return m_magicIconDataPtr;
                }
        };

        class SkillPage: public WidgetGroup
        {
            private:
                const uint32_t m_pageImage;
                std::vector<SkillBoard::MagicIconButton *> m_magicIconButtonList;

            public:
                SkillPage(uint32_t, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void addIcon(SkillBoard::MagicIconData *iconDataPtr)
                {
                    if(!iconDataPtr){
                        throw fflerror("invalid icon data pointer: (null)");
                    }

                    m_magicIconButtonList.push_back(new SkillBoard::MagicIconButton
                    {
                        iconDataPtr->x,
                        iconDataPtr->y,
                        iconDataPtr,
                        this,
                        true,
                    });
                }

            public:
                void drawEx(int, int, int, int, int, int) const override;

            public:
                const auto &getMagicIconButtonList() const
                {
                    return m_magicIconButtonList;
                }

                void setMagicKey(uint32_t magicID, char key)
                {
                    dynamic_cast<SkillBoard *>(m_parent)->setMagicKey(magicID, key);
                }
        };

    private:
        std::list<MagicIconData> m_magicIconDataList;

    private:
        std::vector<SkillPage *> m_skillPageList;

    private:
        // no need to introduce a new type
        // use two tritex button to micmic the tab button

        int m_selectedTabIndex =  0;
        int m_cursorOnTabIndex = -1;
        std::vector<TritexButton *> m_tabButtonList;

    private:
        TexVSlider m_slider;

    private:
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    public:
        SkillBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawTabName() const;
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        static std::array<int, 4> getPageRectange()
        {
            return {18, 44, 304, 329};
        }

    public:
        static int tabElem(int tabIndex)
        {
            if(tabIndex >= 0 && tabIndex <= 6){
                return MET_BEGIN + tabIndex;
            }
            else if(tabIndex == 7){
                return MET_NONE;
            }
            else{
                throw fflerror("invalid tab index: %d", tabIndex);
            }
        }

        int selectedElem() const
        {
            return tabElem(m_selectedTabIndex);
        }

        int cursorOnElem() const
        {
            return tabElem(m_cursorOnTabIndex);
        }

    public:
        std::tuple<uint32_t, int> key2MagicID(char) const;

    public:
        struct MagicKey
        {
            uint32_t magicID = 0;

            char key   = '\0';
            int  angle =   0;
        };

        std::vector<MagicKey> getMagicKeyList() const
        {
            std::vector<MagicKey> result;
            for(const auto &iconData: m_magicIconDataList){
                if(iconData.magicID && iconData.key != '\0'){
                    result.emplace_back(iconData.magicID, iconData.key, iconData.angle());
                }
            }
            return result;
        }

    public:
        void setMagicLevel(uint32_t, int);
        void setMagicKey(uint32_t, char);
        void setMagicCastTime(uint32_t);

    private:
        static int getSkillPageIndex(uint32_t magicID)
        {
            if(!magicID){
                return -1;
            }

            const auto &mr = DBCOM_MAGICRECORD(magicID);
            if(!mr){
                return -1;
            }

            const int elemID = magicElemID(mr.element);
            if(!elemID){
                return -1;
            }

            if(elemID >= MET_BEGIN && elemID < MET_END){
                return elemID - MET_BEGIN;
            }
            else if(elemID == MET_NONE){
                return 7;
            }
            else{
                return -1;
            }
        }
};
