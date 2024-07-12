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
        std::vector<uint32_t> m_imageBuf;

    private:
        std::optional<uint32_t> m_imageIndex;

    private:
        int m_imageOffX = 0;
        int m_imageOffY = 0;
        std::unique_ptr<Fl_Image> m_image;

    public:
        PreviewWindow()
            : Fl_Double_Window(0, 0, 10, 10)
            , m_imageIndex(0)
        {}

    public:
        ~PreviewWindow() = default;

    public:
        void draw() override;

    public:
        bool loadImage();
        void autoResize();
};
