/*
 * =====================================================================================
 *
 *       Filename: previewwindow.hpp
 *        Created: 07/22/2015 03:16:57 AM
 *  Last Modified: 08/10/2017 18:16:33
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

#pragma once
#include <vector>
#include <cstdint>
#include <FL/Fl_Image.H>
#include <FL/Fl_Double_Window.H>

class PreviewWindow: public Fl_Double_Window
{
    public:
        PreviewWindow(int, int);
        ~PreviewWindow();

    public:
        void draw();

    private:
        void ExtendBuf(size_t);
        void LoadImage();

    private:
        bool          m_Inited;
        uint32_t      m_ImageIndex;
        Fl_RGB_Image *m_Image;

    private:
        std::vector<uint32_t> m_Buf;
};
