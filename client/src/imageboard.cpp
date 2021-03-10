/*
 * =====================================================================================
 *
 *       Filename: imageboard.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/09/21 20:10:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;
void ImageBoard::setSize(std::optional<int> newWidth, std::optional<int> newHeight)
{
    m_w = newWidth .value_or(m_w);
    m_h = newHeight.value_or(m_h);
}

void ImageBoard::setSizeRatio(std::optional<float> widthRatio, std::optional<float> heightRatio)
{
    if(widthRatio.value_or(0.0f) < 0.0f){
        throw fflerror("invalid width ratio: %f", widthRatio.value_or(0.0f));
    }

    if(heightRatio.value_or(0.0f) < 0.0f){
        throw fflerror("invalid height ratio: %f", heightRatio.value_or(0.0f));
    }

    m_w *=  widthRatio.value_or(1.0f);
    m_h *= heightRatio.value_or(1.0f);
}

void ImageBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!colorf::A(m_color)){
        return;
    }

    const auto texPtr = m_loadFunc(this);
    if(!texPtr){
        return;
    }

    if(!(w() > 0 && h() > 0)){
        return;
    }

    if(!mathf::ROICrop(&srcX, &srcY, &srcW, &srcH, &dstX, &dstY, w(), h())){
        return;
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
    const auto rw = to_f(texW) / w();
    const auto rh = to_f(texH) / h();

    SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, m_color);
    g_sdlDevice->drawTexture(texPtr, dstX, dstY, srcW, srcH, srcX * rw, srcY * rh, srcW * rw, srcH * rh);
}
