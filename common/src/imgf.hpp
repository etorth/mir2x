/*
 * =====================================================================================
 *
 *       Filename: imgf.hpp
 *        Created: 02/06/2016 04:25:06
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
#include <cstdint>
#include <cstddef>

namespace imgf
{
    bool roiCrop(
            int &,              // dstX, the default parameters we used in Widget::drawEx
            int &,              // dstY, ...

            int &,              // srcX, ...
            int &,              // srcY, ...

            int &,              // srcW, ...
            int &,              // srcH, ...

            int,                // origSrcW, original width  of src image/widget/etc.
            int,                // origSrcH, original height of src image/widget/etc.

            int =  0,           // roiSrcX, ROI on src image/widget/etc., by default use fully
            int =  0,           // roiSrcY, ...
            int = -1,           // roiSrcW, ...
            int = -1,           // roiSrcH, ...

            int =  0,           // roiDstX, ROI on dst image/widget/etc., by default use fully
            int =  0,           // roiDstY, ...
            int = -1,           // roiDstW, ...
            int = -1);          // roiDstH, ...

    void blendImageBuffer(
            uint32_t *,         // RGBA dst buffer
            size_t,             // RGBA dst buffer width
            size_t,             // RGBA dst buffer height

            const uint32_t *,   // RGBA src buffer
            size_t,             // RGBA src buffer width
            size_t,             // RGBA src buffer height

            int,                // dst x
            int,                // dst y

            int,                // src x
            int,                // src y
            int,                // src w
            int);               // src h

    bool saveImageBuffer(
            const void *,       // RGBA src buffer
            size_t,             // RGBA src buffer width
            size_t,             // RGBA src buffer height
            const char *);      // save PNG file name
}
