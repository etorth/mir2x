#include <string>
#include <FL/Fl.H>
#include "mainwindow.hpp"
#include "aboutwindow.hpp"
#include "attributeselectwindow.hpp"
#include "attributegridwindow.hpp"
#include "cropconfigurewindow.hpp"
#include "mapinfowindow.hpp"
#include "selectsettingwindow.hpp"
#include "editormap.hpp"

#include "imagedb.hpp"
#include "imagecache.hpp"

MainWindow                      *g_MainWindow;
SelectSettingWindow             *g_SelectSettingWindow;
AboutWindow                     *g_AboutWindow;
MapInfoWindow                   *g_MapInfoWindow;
AttributeSelectWindow           *g_AttributeSelectWindow;
AttributeGridWindow             *g_AttributeGridWindow;
CropConfigureWindow             *g_CropConfigureWindow;
EditorMap                        g_EditorMap;
ImageDB                          g_ImageDB;
ImageCache                       g_ImageCache;

std::string                      g_WilFilePathName;
std::string                      g_WorkingPathName;
std::string                      g_TestAnimationPathName;
std::vector<Animation>           g_AnimationV;
int                      

int main()
{
    fl_register_images();

    g_WilFilePathName       = "";
    g_WorkingPathName       = "";
    g_TestAnimationPathName = "";

    g_MainWindow            = new MainWindow();
    g_SelectSettingWindow   = new SelectSettingWindow();
    g_MapInfoWindow         = new MapInfoWindow();
    g_AboutWindow           = new AboutWindow();
    g_AttributeSelectWindow = new AttributeSelectWindow();
    g_AttributeGridWindow   = new AttributeGridWindow();        // this is prettry redundant
    g_CropConfigureWindow   = new CropConfigureWindow();

    g_ImageCache.SetPath(".");

    g_MainWindow->ShowAll();
    return Fl::run();
}
