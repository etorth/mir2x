/*
 * =====================================================================================
 *
 *       Filename: animationpreviewwindow.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 08/07/2016 11:30:05
 *
 *    Description: to preview animation to test
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
#include <cstdint>
#include <FL/Fl_Double_Window.H>

class AnimationPreviewWindow
{
    private:
        class PreviewWindow: public Fl_Double_Window
        {
            public:
                PreviewWindow(int, int, int, int);
                ~PreviewWindow();

            private:
                int handle(int);

            protected:
                virtual void draw();
        };

    private:
        PreviewWindow   *m_Window;
        uint32_t         m_MonsterID;

    public:
        AnimationPreviewWindow();
        ~AnimationPreviewWindow();

    public:
        void ShowAll();
        void HideAll();
        void RedrawAll();
        void UpdateFrame();

    public:
        uint32_t MonsterID()
        {
            return m_MonsterID;
        }

        void ResetMonsterID(uint32_t);

    public:
        static void TimeoutCallback(void *);
};
