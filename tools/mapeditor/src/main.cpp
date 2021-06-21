/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
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
#include "layerviewwindow.hpp"
#include "progressbarwindow.hpp"
#include "layerbrowserwindow.hpp"
#include "cropconfigurewindow.hpp"
#include "selectsettingwindow.hpp"
#include "attributeselectwindow.hpp"

MainWindow                      *g_mainWindow               = nullptr;
AboutWindow                     *g_aboutWindow              = nullptr;
AttributeSelectWindow           *g_attributeGridWindow      = nullptr;
AttributeSelectWindow           *g_attributeSelectWindow    = nullptr;
ProgressBarWindow               *g_progressBarWindow        = nullptr;
LayerViewWindow                 *g_layerViewWindow          = nullptr;
LayerBrowserWindow              *g_layerBrowserWindow       = nullptr;
ImageDB                         *g_imageDB                  = nullptr;
EditorMap                        g_editorMap;
ImageCache                       g_imageCache;
AnimationDB                      g_animationDB;
std::string                      g_wilFilePathName;
std::string                      g_workingPathName;

int main()
{
    fl_register_images();

    g_wilFilePathName       = "";
    g_workingPathName       = "";

    g_mainWindow            = new MainWindow();
    g_aboutWindow           = new AboutWindow();
    g_attributeSelectWindow = new AttributeSelectWindow();
    g_attributeGridWindow   = new AttributeSelectWindow();
    g_progressBarWindow     = new ProgressBarWindow();
    g_layerViewWindow       = new LayerViewWindow();
    g_layerBrowserWindow    = new LayerBrowserWindow();

    g_mainWindow->showAll();
    return Fl::run();
}
