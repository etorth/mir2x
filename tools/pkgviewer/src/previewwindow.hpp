/*
 * =====================================================================================
 *
 *       Filename: previewwindow.hpp
 *        Created: 07/22/2015 03:16:57 AM
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
#include <cmath>
#include <memory>
#include <vector>
#include <cstdint>
#include <FL/Fl_Image.H>
#include <FL/Fl_Double_Window.H>

class PreviewWindow: public Fl_Double_Window
{
    public:
        PreviewWindow(int, int);

    public:
        ~PreviewWindow() = default;

    public:
        void draw();

    private:
        void LoadImage();

    private:
        bool     m_Inited;
        uint32_t m_ImageIndex;

    private:
        std::unique_ptr<Fl_RGB_Image> m_Image;

    private:
        std::vector<uint32_t> m_Buf;
};
