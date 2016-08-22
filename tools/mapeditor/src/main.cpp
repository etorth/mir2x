#include <string>
#include <cstdint>
#include <FL/Fl.H>
#include "mainwindow.hpp"
#include "aboutwindow.hpp"
#include "attributeselectwindow.hpp"
#include "attributegridwindow.hpp"
#include "cropconfigurewindow.hpp"
#include "mapinfowindow.hpp"
#include "selectsettingwindow.hpp"
#include "animationselectwindow.hpp"
#include "editormap.hpp"

#include "imagedb.hpp"
#include "imagecache.hpp"
#include "animationdb.hpp"
#include "animationdraw.hpp"

MainWindow                      *g_MainWindow;
SelectSettingWindow             *g_SelectSettingWindow;
AnimationSelectWindow           *g_AnimationSelectWindow;
AboutWindow                     *g_AboutWindow;
MapInfoWindow                   *g_MapInfoWindow;
AttributeSelectWindow           *g_AttributeSelectWindow;
AttributeGridWindow             *g_AttributeGridWindow;
CropConfigureWindow             *g_CropConfigureWindow;
EditorMap                        g_EditorMap;
ImageDB                          g_ImageDB;
ImageCache                       g_ImageCache;
AnimationDB                      g_AnimationDB;
AnimationDraw                    g_AnimationDraw;

std::string                      g_WilFilePathName;
std::string                      g_WorkingPathName;

int main()
{
    fl_register_images();

    g_WilFilePathName       = "";
    g_WorkingPathName       = "";

    g_MainWindow             = new MainWindow();
    g_SelectSettingWindow    = new SelectSettingWindow();
    g_AnimationSelectWindow  = new AnimationSelectWindow();
    g_MapInfoWindow          = new MapInfoWindow();
    g_AboutWindow            = new AboutWindow();
    g_AttributeSelectWindow  = new AttributeSelectWindow();
    g_AttributeGridWindow    = new AttributeGridWindow();        // this is prettry redundant
    g_CropConfigureWindow    = new CropConfigureWindow();

    g_ImageCache.SetPath(".");

    g_MainWindow->ShowAll();
    return Fl::run();
}
