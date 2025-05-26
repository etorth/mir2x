#pragma once
#include "mathf.hpp"
#include "widget.hpp"

class GfxDupBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    public:
        GfxDupBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSize argW,
                Widget::VarSize argH,

                const Widget *argWidget,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  std::move(argW),
                  std::move(argH),

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_gfxWidget([argWidget]{ fflassert(argWidget); return argWidget; }())
        {}

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            const auto fnCropDraw = [dstX, dstY, srcW, srcH, this](int onCropBrdX, int onCropBrdY, int onCropBrdW, int onCropBrdH, int onScreenX, int onScreenY)
            {
                if(mathf::cropROI(
                            &onCropBrdX, &onCropBrdY,
                            &onCropBrdW, &onCropBrdH,

                            &onScreenX,
                            &onScreenY,

                            m_gfxWidget->w(),
                            m_gfxWidget->h(),

                            0, 0, -1, -1, dstX, dstY, srcW, srcH)){
                    m_gfxWidget->drawEx(onScreenX, onScreenY, onCropBrdX, onCropBrdY, onCropBrdW, onCropBrdH);
                }
            };

            const auto extendedDstX = dstX - srcX;
            const auto extendedDstY = dstY - srcY;

            int doneDrawWidth = 0;
            while(doneDrawWidth < w()){
                const int drawWidth = std::min<int>(m_gfxWidget->w(), w() - doneDrawWidth);
                int doneDrawHeight = 0;
                while(doneDrawHeight < h()){
                    const int drawHeight = std::min<int>(m_gfxWidget->h(), h() - doneDrawHeight);
                    fnCropDraw(0, 0, drawWidth, drawHeight, extendedDstX + doneDrawWidth, extendedDstY + doneDrawHeight);
                    doneDrawHeight += drawHeight;
                }
                doneDrawWidth += drawWidth;
            }
        }
};
