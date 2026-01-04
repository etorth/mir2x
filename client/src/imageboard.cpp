#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;

ImageBoard::ImageBoard(ImageBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = args.parent,
      }}

    , m_varColor(std::move(args.modColor))
    , m_varBlendMode(std::move(args.blendMode))

    , m_loadFunc(std::move(args.texLoadFunc))
    , m_xformPair(getHFlipRotatePair(args.hflip, args.vflip, args.rotate))
{
    const auto varTexW = args.w.value_or([this]{ return SDLDeviceHelper::getTextureWidth (getTexture(), 0); });
    const auto varTexH = args.h.value_or([this]{ return SDLDeviceHelper::getTextureHeight(getTexture(), 0); });

    setSize((m_rotate % 2 == 0) ? varTexW : varTexH,
            (m_rotate % 2 == 0) ? varTexH : varTexW);
}

void ImageBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(!colorf::A(Widget::evalU32(m_varColor, this))){
        return;
    }

    const auto [
        drawDstX, drawDstY,
        drawSrcX, drawSrcY,
        drawSrcW, drawSrcH,

        centerOffX,
        centerOffY,

        rotateDegree] = [
            dstX = m.x,
            dstY = m.y,

            srcX = m.ro->x,
            srcY = m.ro->y,
            srcW = m.ro->w,
            srcH = m.ro->h,

            this]() -> std::array<int, 9>
    {
        // draw rotate and flip
        // all corners are indexed as 0, 1, 2, 3

        // 0      1
        //  +----+
        //  |    |   empty square means dst region before rotation
        //  +----+
        // 3      2

        // 0      1
        //  +----+
        //  |....|   solid square means dst region after rotation, the real dst region
        //  +----+
        // 3      2

        switch(m_rotate % 4){
            case 0:
                {
                    //  0           1
                    //  +-----------+
                    //  |...........|
                    //  |...........|
                    //  +-----------+
                    //  3           2

                    return
                    {
                        dstX,
                        dstY,

                        srcX,
                        srcY,

                        srcW,
                        srcH,

                        0,
                        0,

                        0,
                    };
                }
            case 1:
                {
                    // 0           1
                    // +-----------+
                    // |           |
                    // |           |
                    // o-----+-----+
                    // |.....|     2
                    // |.....|
                    // |.....|
                    // |.....|
                    // +-----+
                    // 2     1

                    return
                    {
                        dstX,
                        dstY - srcW,

                        srcY,
                        w() - srcX - srcW,

                        srcH,
                        srcW,

                        0,
                        srcW,

                        90,
                    };
                }
            case 2:
                {
                    // 0           1
                    // +-----------+
                    // |           |
                    // |           |           3
                    // +-----------o-----------+
                    // 3           |...........|
                    //             |...........|
                    //             +-----------+
                    //             1           0

                    return
                    {
                        dstX - srcW,
                        dstY - srcH,

                        w() - srcW - srcX,
                        h() - srcH - srcY,

                        srcW,
                        srcH,

                        srcW,
                        srcH,

                        180,
                    };
                }
            default:
                {
                    // 1     2
                    // +-----+
                    // |.....|
                    // |.....|
                    // |.....|           1
                    // |.....+-----------+
                    // |.....|           |
                    // |.....|           |
                    // +-----o-----------+
                    // 0     3           2

                    return
                    {
                        dstX        + srcW,
                        dstY + srcH - srcW,

                        h() - srcH - srcY,
                        srcX,

                        srcH,
                        srcW,

                        0,
                        srcW,

                        270,
                    };
                }
        }
    }();

    const auto texPtr = getTexture();
    if(!texPtr){
        return;
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

    const auto  widthRatio = to_df(texW) / ((m_rotate % 2 == 0) ? w() : h());
    const auto heightRatio = to_df(texH) / ((m_rotate % 2 == 0) ? h() : w());

    // imgSrcX
    // size and position cropped from original image, no resize, well defined

    /**/  int imgSrcX = to_dround( widthRatio * drawSrcX);
    const int imgSrcY = to_dround(heightRatio * drawSrcY);
    const int imgSrcW = to_dround( widthRatio * drawSrcW);
    const int imgSrcH = to_dround(heightRatio * drawSrcH);

    if(m_hflip){
        imgSrcX = texW - imgSrcX - imgSrcW;
    }

    const SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, Widget::evalU32(m_varColor, this));
    const SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, Widget::evalBlendMode(m_varBlendMode, this));

    g_sdlDevice->drawTextureEx(
            texPtr,

            imgSrcX, imgSrcY,
            imgSrcW, imgSrcH,

            drawDstX, drawDstY,
            drawSrcW, drawSrcH,

            centerOffX,
            centerOffY,

            rotateDegree,
            m_hflip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
