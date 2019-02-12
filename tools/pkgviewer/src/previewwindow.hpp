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
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <FL/Fl_Image.H>
#include <FL/Fl_Double_Window.H>

class PreviewWindow: public Fl_Double_Window
{
    private:
        // for Fl_RGB_Image class
        // caller need to maintain the color buffer
        std::vector<uint32_t> m_Buf;

    private:
        std::optional<uint32_t> m_ImageIndex;

    private:
        std::unique_ptr<Fl_RGB_Image> m_Image;

    public:
        PreviewWindow()
            : Fl_Double_Window(0, 0, 10, 10)
            , m_Buf()
            , m_ImageIndex(0)
            , m_Image()
        {}

    public:
        ~PreviewWindow() = default;

    public:
        void draw() override;

    public:
        bool LoadImage();
};
