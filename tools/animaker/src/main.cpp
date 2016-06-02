#include <string>
#include <cstdint>
#include "filesys.hpp"
#include "sidwindow.hpp"
#include "mainwindow.hpp"
#include "aboutwindow.hpp"
#include "animationset.hpp"
#include "wilimagepackage.hpp"
#include "progressbarwindow.hpp"
#include "validwilfilewindow.hpp"
#include "validanimationwindow.hpp"
#include "animationpreviewwindow.hpp"

SIDWindow              *g_SIDWindow;
WilImagePackage         g_WilImagePackage[2];
MainWindow             *g_MainWindow;
ValidWilFileWindow     *g_ValidWilFileWindow;
ValidAnimationWindow   *g_ValidAnimationWindow;
AboutWindow            *g_AboutWindow;
ProgressBarWindow      *g_ProgressBarWindow;
AnimationPreviewWindow *g_AnimationPreviewWindow;
AnimationSet            g_AnimationSet;
int                     g_AnimationSetPositionX;
int                     g_AnimationSetPositionY;
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
    g_AnimationSetPositionX  = 200;
    g_AnimationSetPositionY  = 200;
    g_WorkingPathName        = ".";
    g_SIDWindow              = new SIDWindow;
    g_MainWindow             = new MainWindow;
    g_ValidWilFileWindow     = new ValidWilFileWindow;
    g_ValidAnimationWindow   = new ValidAnimationWindow;
    g_AboutWindow            = new AboutWindow;
    g_ProgressBarWindow      = new ProgressBarWindow;
    g_AnimationPreviewWindow = nullptr;

    g_MainWindow->ShowAll();

    return Fl::run();
}
