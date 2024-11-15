#pragma once
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include "widget.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxcropdupboard.hpp"

class ModalStringBoard: public Widget
{
    private:
        mutable std::mutex m_lock;
        mutable std::condition_variable m_cond;

    private:
        const int m_minH = 300;
        const uint32_t m_texID = 0X07000000;

    private:
        LayoutBoard m_board;

    private:
        ImageBoard      m_image;
        GfxCropBoard    m_imageUp;    // top 180 pixels
        GfxCropDupBoard m_imageUpDup; // duplicate top 84 ~ 180 pixels for stretch
        GfxCropBoard    m_imageDown;  // bottom 40 pixels

    private:
        bool m_done = false;
        std::u8string m_xmlString;

    public:
        ModalStringBoard();

    public:
        void loadXML(std::u8string);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void  setDone();
        void waitDone();
};
