#pragma once
#include <cstdint>
#include "widget.hpp"
#include "itembox.hpp"
#include "gfxcropboard.hpp"
#include "gfxshapeboard.hpp"
#include "texsliderbar.hpp"

class RuntimeConfigBoard;
class DropItemRuleBoard final: public Widget
{
    private:
        constexpr static int m_listX = 0;
        constexpr static int m_listY = 0;
        constexpr static int m_listW = 420;
        constexpr static int m_listH = 300;
        constexpr static int m_sliderX = m_listX + m_listW + 6;

    private:
        RuntimeConfigBoard *m_configBoard = nullptr;

    private:
        GfxShapeBoard m_listBg;

    private:
        ItemBox m_itemBox;
        GfxCropBoard m_viewport;
        TexSliderBar m_slider;

    public:
        explicit DropItemRuleBoard(RuntimeConfigBoard *);

    private:
        int scrollMax() const
        {
            return std::max<int>(0, m_itemBox.h() - m_listH);
        }
};
