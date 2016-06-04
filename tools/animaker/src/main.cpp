/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 06/04/2016 03:38:32
 *
 *    Description: set all kinds of align, for animation center (X, Y)
 *                 1. animation align:
 *                      the offset for all actionsets in this animation, like for a
 *                      bird, the graphcal center is always upper to its animation
 *                      center. this is mostly used
 *
 *                 2. direction align:
 *                      all actionset on one direction will get an align, this is
 *                      based on the fact that for one direction, the switch for
 *                      different action is pretty smooth. but when changing the
 *                      direction, the cover between two direction is not good, so
 *                      we add the align, and since changing direction can tolerate
 *                      some ``non-smothness", this helps to make the cover to be
 *                      better
 *
 *                 2. actionset align:
 *                      offset for this actionset only, for one monster stand and
 *                      the start to move, this offset make the state switch to be
 *                      smooth, not like stand->jump->walk
 *
 *                 3. body frame align:
 *                      the additional align for body frame. because mir2 comes
 *                      with an align alreay. this align is merely used since for
 *                      the actionset I checked the frame serial is pretty good! so
 *                      this frame align is disabled
 *
 *                 4. shadow frame align
 *                      the additional align for shadow frame, mir2 alreayd provides
 *                      shadow align. this additional align is used when we need to
 *                      dynamically create shadows for the monster. otherwise it's
 *                      disabled
 *
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
#include <string>
#include <cstdint>

#include "filesys.hpp"
#include "mainwindow.hpp"
#include "aboutwindow.hpp"
#include "animationset.hpp"
#include "wilimagepackage.hpp"
#include "progressbarwindow.hpp"
#include "validwilfilewindow.hpp"
#include "validanimationwindow.hpp"
#include "animationpreviewwindow.hpp"

WilImagePackage         g_WilImagePackage[2];
MainWindow             *g_MainWindow;
ValidWilFileWindow     *g_ValidWilFileWindow;
ValidAnimationWindow   *g_ValidAnimationWindow;
AboutWindow            *g_AboutWindow;
ProgressBarWindow      *g_ProgressBarWindow;
AnimationPreviewWindow *g_AnimationPreviewWindow;
AnimationSet            g_AnimationSet;
int                     g_AnimationSetWinCenterX;   // center X on win
int                     g_AnimationSetWinCenterY;   // center Y on win
uint32_t                g_TestAnimationCode;
std::string             g_WorkingPathName;

const char *g_MonsterWilFileNameList[] = {
    "Mon-1",  "Mon-2",  "Mon-3",  "Mon-4",  "Mon-5",  "Mon-6",  "Mon-7",  "Mon-8",  "Mon-9",  "Mon-10",
    "Mon-11", "Mon-12", "Mon-13", "Mon-14", "Mon-15", "Mon-16", "Mon-17", "Mon-18", "Mon-19", "Mon-20",
    ""
};

int g_MonsterWilFileStartIndex[] = {
    1,    1, 0, 0, 0, 0, 1, 0, 0, 1,
    1001, 0, 1, 0, 0, 1, 1, 1, 1, 1
};

int g_MonsterShadowWilFileStartIndex[] = {
    1,    1,  880, 0, 7000, 2000, 0, 0, 0,    1,
    2001, 0,    1, 0,    0,    1, 1, 1, 1, 1881
};

// list for 100 possible status
const char *g_StatusNameList[] = {
    "Stand",             // 0 
    "Walk",              // 1
    "Attack",            // 2
    "Under Attack",      // 3
    "Dead",              // 4
    "Undefined",         // 5
    "Undefined",         // 6
    "Undefined",         // 7
    "Undefined",         // 8
    "Undefined",         // 9
    ""
};

int main()
{
    MakeDir("./IMG");
    fl_register_images();

    g_TestAnimationCode      = 0;
    g_AnimationSetWinCenterX = 0;
    g_AnimationSetWinCenterY = 0;
    g_WorkingPathName        = ".";
    g_MainWindow             = new MainWindow;
    g_ValidWilFileWindow     = new ValidWilFileWindow;
    g_ValidAnimationWindow   = new ValidAnimationWindow;
    g_AboutWindow            = new AboutWindow;
    g_ProgressBarWindow      = new ProgressBarWindow;
    g_AnimationPreviewWindow = nullptr;

    g_MainWindow->ShowAll();

    return Fl::run();
}
