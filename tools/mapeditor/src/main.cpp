/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 08/23/2017 21:37:27
 *
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

#include <string>
#include <cstdint>
#include <FL/Fl.H>

#include "imagedb.hpp"
#include "editormap.hpp"
#include "mainwindow.hpp"
#include "imagecache.hpp"
#include "animationdb.hpp"
#include "aboutwindow.hpp"
#include "animationdraw.hpp"
#include "progressbarwindow.hpp"
#include "layereditorwindow.hpp"
#include "layerbrowserwindow.hpp"
#include "cropconfigurewindow.hpp"
#include "selectsettingwindow.hpp"
#include "animationselectwindow.hpp"
#include "attributeselectwindow.hpp"

MainWindow                      *g_MainWindow;
SelectSettingWindow             *g_SelectSettingWindow;
AnimationSelectWindow           *g_AnimationSelectWindow;
AboutWindow                     *g_AboutWindow;
AttributeSelectWindow           *g_AttributeGridWindow;
AttributeSelectWindow           *g_AttributeSelectWindow;
ProgressBarWindow               *g_ProgressBarWindow;
LayerEditorWindow               *g_LayerEditorWindow;
LayerBrowserWindow              *g_LayerBrowserWindow;
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

    g_MainWindow            = new MainWindow();
    g_SelectSettingWindow   = new SelectSettingWindow();
    g_AnimationSelectWindow = new AnimationSelectWindow();
    g_AboutWindow           = new AboutWindow();
    g_AttributeSelectWindow = new AttributeSelectWindow();
    g_AttributeGridWindow   = new AttributeSelectWindow();
    g_ProgressBarWindow     = new ProgressBarWindow();
    g_LayerEditorWindow     = new LayerEditorWindow();
    g_LayerBrowserWindow    = new LayerBrowserWindow();
    g_CropConfigureWindow   = new CropConfigureWindow();

    g_ImageCache.SetPath(".");

    g_MainWindow->ShowAll();
    return Fl::run();
}
