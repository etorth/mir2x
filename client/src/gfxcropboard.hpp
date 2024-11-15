#pragma once
#include "mathf.hpp"
#include "widget.hpp"

class GfxCropBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    private:
        const Widget::VarOff m_brdCropX;
        const Widget::VarOff m_brdCropY;
        const Widget::VarOff m_brdCropW;
        const Widget::VarOff m_brdCropH;

    private:
        const std::array<int, 4> m_margin;

    public:
        GfxCropBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                const Widget *argWidget,

                Widget::VarOff argBrdCropX, // crop on gfx widget
                Widget::VarOff argBrdCropY, // ...
                Widget::VarOff argBrdCropW, // crop width, don't use Widget::VarSize, support over-cropping
                Widget::VarOff argBrdCropH, // ...

                std::array<int, 4> argMargin = {0, 0, 0, 0},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  0,
                  0,

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_gfxWidget([argWidget]{ fflassert(argWidget); return argWidget; }())

            , m_brdCropX(std::move(argBrdCropX))
            , m_brdCropY(std::move(argBrdCropY))
            , m_brdCropW(std::move(argBrdCropW))
            , m_brdCropH(std::move(argBrdCropH))

            , m_margin(argMargin)
        {
            // respect blank space by over-cropping
            // if cropped size bigger than gfx size, we fill with blank

            setSize([this](const Widget *)
            {
                const auto cropW = Widget::evalOff(m_brdCropW, this);
                fflassert(cropW >= 0);
                return cropW + m_margin[2] + m_margin[3];
            },

            [this](const Widget *)
            {
                const auto cropH = Widget::evalOff(m_brdCropH, this);
                fflassert(cropH >= 0);
                return cropH + m_margin[0] + m_margin[1];
            });
        }

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            const int brdCropXOrig = Widget::evalOff(m_brdCropX, this);
            const int brdCropYOrig = Widget::evalOff(m_brdCropY, this);

            int brdCropX = brdCropXOrig;
            int brdCropY = brdCropYOrig;
            int brdCropW = Widget::evalOff(m_brdCropW, this);
            int brdCropH = Widget::evalOff(m_brdCropH, this);

            fflassert(brdCropW >= 0);
            fflassert(brdCropH >= 0);

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_gfxWidget->w(), m_gfxWidget->h(), brdCropX, brdCropY, brdCropW, brdCropH)){
                return;
            }

            int drawDstX = dstX;
            int drawDstY = dstY;
            int drawSrcX = srcX;
            int drawSrcY = srcY;
            int drawSrcW = srcW;
            int drawSrcH = srcH;

            if(!mathf::cropChildROI(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        w(),
                        h(),

                        m_margin[2] + brdCropX - brdCropXOrig,
                        m_margin[0] + brdCropY - brdCropYOrig,

                        brdCropW,
                        brdCropH)){
                return;
            }

            m_gfxWidget->drawEx(drawDstX, drawDstY, drawSrcX + brdCropX, drawSrcY + brdCropY, drawSrcW, drawSrcH);
        }
};
