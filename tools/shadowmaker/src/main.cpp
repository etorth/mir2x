#include <string>
#include <cstdint>
#include "misc.hpp"
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

const char *g_MonsterWilFileNameList[] = {"M-Hum", "WM-Hum", ""};

// const char *g_MonsterWilFileNameList[] = {
//     "Mon-1",  "Mon-2",  "Mon-3",  "Mon-4",  "Mon-5",  "Mon-6",  "Mon-7",  "Mon-8",  "Mon-9",  "Mon-10",
//     "Mon-11", "Mon-12", "Mon-13", "Mon-14", "Mon-15", "Mon-16", "Mon-17", "Mon-18", "Mon-19", "Mon-20",
//     ""
// };

int g_MonsterWilFileStartIndex[] = {1, 1};

// int g_MonsterWilFileStartIndex[] = {
//     1,    1, 0, 0, 0, 0, 1, 0, 0, 1,
//     1001, 0, 1, 0, 0, 1, 1, 1, 1, 1
// };

// int g_MonsterShadowWilFileStartIndex[] = {
//     1,    1,  880, 0, 7000, 2000, 0, 0, 0,    1,
//     2001, 0,    1, 0,    0,    1, 1, 1, 1, 1881
// };

// list for 100 possible status
const char *g_StatusNameList[] = {
    "Stand",             //  0 
    "Archery",           //  1
    "CastMagic",         //  2
    "PrayerMagic",       //  3
    "Defense",           //  4
    "BowBody",           //  5, this maybe a intermedial state
    "UnderKicked",       //  6
    "StandBeforeAttack", //  7
    "Cut",               //  8
    "SingleHandAttack0", //  9
    "DoubleHandAttack0", // 10
    "SingleHandAttack1", // 11
    "DoubleHandAttack1", // 12
    "DoubleHandAttack2", // 13
    "DoubleHandAttack3", // 14
    "Struck",            // 15
    "SingleHandAttack2", // 16
    "SingleHandAttack3", // 17
    "Kick",              // 18
    "Dead",              // 19
    "DeadOnHorse",       // 20
    "Walk",              // 21
    "Run",               // 22
    "Push",              // 23
    "Roll",              // 24
    "Fish",              // 25
    "EndFish0",          // 26
    "StartFish",         // 27
    "EndFish1",          // 28
    "StandOnHorse",      // 29
    "WalkOnHorse",       // 30
    "RunOnHorse",        // 31
    "StructOnHorse",     // 32
    ""
};

// const int g_StatusStartIndexList[] = {
//       1, //  0 Stand
//      81, //  1 Archery(Lost)
//     161, //  2 CastMagic
//     241, //  3 PrayerMagic
//     321, //  4 Defense
//     401, //  5 BowBody, I think this is a intermedial state
//     481, //  6 UnderKick
//     561, //  7 StandBeforeAttack
//     641, //  8 Cut
//     721, //  9 SingleHandAttack0
//     801, // 10 DoubleHandAttack0
//     881, // 11 SingleHandAttack1
//     961, // 12 DoubleHandAttack1
//    1041, // 13 DoubleHandAttack2
//    1121,
//    1201,
//    1281, // 10 frames
//    1361,
//    1441,
//    1521,
//    1601, // lost
//    1681,
//    1761,
//    1841,
//    1921, // RollBack Lost
//    2001, // Fish(lost)
//    2081, // FishEnd(lost)
//    2161, // FishStart(lost)
//    2241, // FishEnd1(lost)
//    2321, // StandOnHorse
//    2401, // WalkOnHorse
//    2481, // RunOnHorse
//    2561, // StruckOnHorse
//    // 3001, next animation set
// };

// const char *g_StatusNameList[] = {
//     "Stand",             // 0 
//     "Walk",              // 1
//     "Attack",            // 2
//     "Under Attack",      // 3
//     "Dead",              // 4
//     "Undefined",         // 5
//     "Undefined",         // 6
//     "Undefined",         // 7
//     "Undefined",         // 8
//     "Undefined",         // 9
//     ""
// };

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
