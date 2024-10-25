#pragma once
#include <initializer_list>
#include "mathf.hpp"
#include "widget.hpp"
#include "gfxdupboard.hpp"
#include "gfxcropboard.hpp"

class GfxCropDupBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    private:
        const WidgetVarOffset m_cropX;
        const WidgetVarOffset m_cropY;
        const WidgetVarOffset m_cropW;
        const WidgetVarOffset m_cropH;

    public:
        GfxCropDupBoard(
                WidgetVarDir argDir,

                WidgetVarOffset argX,
                WidgetVarOffset argY,

                WidgetVarSize argW,
                WidgetVarSize argH,

                const Widget *argWidget,

                WidgetVarOffset argCropX,
                WidgetVarOffset argCropY,
                WidgetVarOffset argCropW, // don't use VarSize because empty VarSize is not well-defined
                WidgetVarOffset argCropH, // ...

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

            , m_cropX(std::move(argCropX))
            , m_cropY(std::move(argCropY))
            , m_cropW(std::move(argCropW))
            , m_cropH(std::move(argCropH))
        {
            if(Widget::hasFuncOffset(m_cropX)){ fflassert(Widget::asFuncOffset(m_cropX), m_cropX); }
            if(Widget::hasFuncOffset(m_cropY)){ fflassert(Widget::asFuncOffset(m_cropY), m_cropY); }
            if(Widget::hasFuncOffset(m_cropW)){ fflassert(Widget::asFuncOffset(m_cropW), m_cropW); }
            if(Widget::hasFuncOffset(m_cropH)){ fflassert(Widget::asFuncOffset(m_cropH), m_cropH); }

            if(Widget::hasIntOffset(m_cropW)){ fflassert(Widget::asIntOffset(m_cropW) >= 0, m_cropW); }
            if(Widget::hasIntOffset(m_cropH)){ fflassert(Widget::asIntOffset(m_cropH) >= 0, m_cropH); }
        }

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            int cropSrcX = Widget::evalOffset(m_cropX, this);
            int cropSrcY = Widget::evalOffset(m_cropY, this);
            int cropSrcW = Widget::evalOffset(m_cropW, this);
            int cropSrcH = Widget::evalOffset(m_cropH, this);

            fflassert(cropSrcW >= 0, cropSrcW);
            fflassert(cropSrcH >= 0, cropSrcH);

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_gfxWidget->w(), m_gfxWidget->h(), cropSrcX, cropSrcY, cropSrcW, cropSrcH)){
                return;
            }

            //       w0  w1  w2
            //     +---+---+---+
            //  h0 |   |   |   |
            //     +---+---+---+
            //  h1 |   |   |   |
            //     +---+---+---+
            //  h2 |   |   |   |
            //     +---+---+---+

            const int x0 = 0;
            const int y0 = 0;
            const int w0 = cropSrcX;
            const int h0 = cropSrcY;

            const int x1 = x0 + w0;
            const int y1 = y0 + h0;
            const int w1 = cropSrcW;
            const int h1 = cropSrcH;

            const int x2 = x1 + w1;
            const int y2 = y1 + h1;
            const int w2 = m_gfxWidget->w() - w0 - w1;
            const int h2 = m_gfxWidget->h() - h0 - h1;

            const GfxCropBoard topLeft     {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y0, w0, h0};
            const GfxCropBoard top         {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y0, w1, h0};
            const GfxCropBoard topRight    {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y0, w2, h0};
            const GfxCropBoard left        {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y1, w0, h1};
            const GfxCropBoard middle      {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y1, w1, h1};
            const GfxCropBoard right       {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y1, w2, h1};
            const GfxCropBoard bottomLeft  {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y2, w0, h2};
            const GfxCropBoard bottom      {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y2, w1, h2};
            const GfxCropBoard bottomRight {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y2, w2, h2};

            const GfxDupBoard    topDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h0, &top    };
            const GfxDupBoard   leftDup{DIR_UPLEFT, 0, 0,            w0, h() - h0 - h2, &left   };
            const GfxDupBoard middleDup{DIR_UPLEFT, 0, 0, w() - w0 - w2, h() - h0 - h2, &middle };
            const GfxDupBoard  rightDup{DIR_UPLEFT, 0, 0,            w2, h() - h0 - h2, &right  };
            const GfxDupBoard bottomDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h2, &bottom };

            for(const auto &[widgetPtr, offX, offY]: std::initializer_list<std::tuple<const Widget *, int, int>>
            {
                {static_cast<const Widget *>(&topLeft    ),        0,        0},
                {static_cast<const Widget *>(&topDup     ),       w0,        0},
                {static_cast<const Widget *>(&topRight   ), w() - w2,        0},

                {static_cast<const Widget *>(&  leftDup  ),        0,       h0},
                {static_cast<const Widget *>(&middleDup  ),       w0,       h0},
                {static_cast<const Widget *>(& rightDup  ), w() - w2,       h0},

                {static_cast<const Widget *>(&bottomLeft ),        0, h() - h2},
                {static_cast<const Widget *>(&bottomDup  ),       w0, h() - h2},
                {static_cast<const Widget *>(&bottomRight), w() - w2, h() - h2},
            }){
                int drawSrcX = srcX;
                int drawSrcY = srcY;
                int drawSrcW = srcW;
                int drawSrcH = srcH;
                int drawDstX = dstX;
                int drawDstY = dstY;

                if(mathf::cropChildROI(
                            &drawSrcX, &drawSrcY,
                            &drawSrcW, &drawSrcH,
                            &drawDstX, &drawDstY,

                            w(),
                            h(),

                            offX,
                            offY,
                            widgetPtr->w(),
                            widgetPtr->h())){
                    widgetPtr->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
                }
            }
        }
};
