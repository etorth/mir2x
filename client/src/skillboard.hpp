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
#include "widget.hpp"
#include "labelboard.hpp"
#include "texvslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class SkillBoard: public Widget
{
    private:
        struct magicIcon
        {
            // icon width : 40
            // icon height: 40

            uint32_t magicID;

            int x;
            int y;
            char key;
            bool active;
        };

    private:
        std::vector<magicIcon> m_magicIconList;

    private:
        // no need to introduce a new type
        // use two tritex button to micmic the tab button

        int m_tabIndex = 0;
        std::vector<std::array<std::unique_ptr<TritexButton>, 2>> m_tabButtonList;

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
        void drawPage();

    private:
        static std::array<int, 4> getPageRectange()
        {
            return {18, 44, 304, 329};
        }
};
