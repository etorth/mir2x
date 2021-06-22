/*
 * =====================================================================================
 *
 *       Filename: alphaf.hpp
 *        Created: 11/13/2018 22:31:02
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
#include <tuple>
#include <cmath>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "colorf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

namespace alphaf
{
    inline void removeShadowMosaic(uint32_t *data, size_t width, size_t height, uint32_t shadowColor)
    {
        fflassert(data);
        fflassert(width > 0);
        fflassert(height > 0);

        if(width < 3 || height < 3){
            return;
        }

        // -1: normal
        //  0: transparent
        //  1: has shadow color, but can be noise points
        //  2: shadow points for sure

        std::vector<signed char> dataMarker(width * height);
        const auto fn_marker_ref = [width, &dataMarker](size_t x, size_t y) -> signed char &
        {
            return dataMarker.at(y * width + x);
        };

        for(size_t y = 0; y < height; ++y){
            for(size_t x = 0; x < width; ++x){
                if(const uint32_t pixelColor = data[y * width + x]; colorf::A(pixelColor) > 0){
                    // alpha not 0
                    // can be shadow pixels, noise points or normal points

                    const uint32_t r = colorf::R(pixelColor);
                    const uint32_t g = colorf::G(pixelColor);
                    const uint32_t b = colorf::B(pixelColor);

                    if(true
                            && r + g + b < 0X08 * 3
                            && std::max<uint32_t>({r, g, b}) < 0X0C){
                        fn_marker_ref(x, y) = 1;
                    }
                    else{
                        fn_marker_ref(x, y) = -1;
                    }
                }
                else{
                    // alpha is 0
                    // can be grids between shadow pixels
                    fn_marker_ref(x, y) = 0;
                }
            }
        }

        // mark the 100% sure shadow points, strict step
        // can fail the edge of shadow area, shadow area too thin also fails

        for(size_t y = 1; y < height - 1; ++y){
            for(size_t x = 1; x < width - 1; ++x){
                if(fn_marker_ref(x, y) == 1){
                    if(true
                            && fn_marker_ref(x - 1, y - 1) > 0
                            && fn_marker_ref(x + 1, y - 1) > 0
                            && fn_marker_ref(x - 1, y + 1) > 0
                            && fn_marker_ref(x + 1, y + 1) > 0

                            && fn_marker_ref(x, y - 1) == 0
                            && fn_marker_ref(x, y + 1) == 0
                            && fn_marker_ref(x - 1, y) == 0
                            && fn_marker_ref(x + 1, y) == 0){

                        // checked neighbors around it
                        // mark as shadow points for 100% sure
                        fn_marker_ref(x, y) = 2;
                    }
                }
            }
        }

        // after previous step, if a pixel is 1, then it must be a shadow pixel
        // now add back edge points

        for(size_t y = 1; y < height - 1; ++y){
            for(size_t x = 1; x < width - 1; ++x){
                if(fn_marker_ref(x, y) == 2){
                    if(auto &mark = fn_marker_ref(x - 1, y - 1); mark == 1){ mark = 2; }
                    if(auto &mark = fn_marker_ref(x + 1, y - 1); mark == 1){ mark = 2; }
                    if(auto &mark = fn_marker_ref(x - 1, y + 1); mark == 1){ mark = 2; }
                    if(auto &mark = fn_marker_ref(x + 1, y + 1); mark == 1){ mark = 2; }
                }
            }
        }

        // try add back areas too thin
        // count 1 and 0, black areas can't have 0's around shadow color
        for(size_t y = 1; y < height - 1; ++y){
            for(size_t x = 1; x < width - 1; ++x){
                if(fn_marker_ref(x, y) == 1){
                    const auto n11 = fn_marker_ref(x - 1, y - 1) > 0;
                    const auto n12 = fn_marker_ref(x + 1, y - 1) > 0;
                    const auto n13 = fn_marker_ref(x - 1, y + 1) > 0;
                    const auto n14 = fn_marker_ref(x + 1, y + 1) > 0;

                    const auto n01 = fn_marker_ref(x, y - 1) == 0;
                    const auto n02 = fn_marker_ref(x, y + 1) == 0;
                    const auto n03 = fn_marker_ref(x - 1, y) == 0;
                    const auto n04 = fn_marker_ref(x + 1, y) == 0;

                    if(true
                            && n11 + n12 + n13 + n14 >= 2
                            && n01 + n02 + n03 + n04 >= 2){
                        fn_marker_ref(x, y) = 2;
                    }
                }
            }
        }

        // fill those 0's inside 1's
        // edges can be filled, also area too thin can be filled

        for(size_t y = 0; y < height; ++y){
            for(size_t x = 0; x < width; ++x){
                if(fn_marker_ref(x, y) == 0){
                    if((x > 0) && (x + 1 < width) && (fn_marker_ref(x - 1, y) == 2) && (fn_marker_ref(x + 1, y) == 2)){
                        fn_marker_ref(x, y) = 2;
                        continue;
                    }

                    if((y > 0) && (y + 1 < height) && (fn_marker_ref(x, y - 1) == 2) && (fn_marker_ref(x, y + 1) == 2)){
                        fn_marker_ref(x, y) = 2;
                        continue;
                    }
                }
            }
        }

        // done
        // for pixel marked as 2, assign given color
        // don't assign for pixels marked as 1, because this may change those black areas

        for(size_t y = 0; y < height; ++y){
            for(size_t x = 0; x < width; ++x){
                if(fn_marker_ref(x, y) == 2){
                    data[y * width + x] = shadowColor;
                }
            }
        }
    }

    inline void autoAlpha(uint32_t *data, size_t size)
    {
        fflassert(data);
        fflassert(size > 0);

        for(size_t i = 0; i < size; ++i){
            uint8_t a = colorf::A(data[i]);
            if(a == 0){
                data[i] = 0;
                continue;
            }

            uint8_t r = colorf::R(data[i]);
            uint8_t g = colorf::G(data[i]);
            uint8_t b = colorf::B(data[i]);

            a = std::max<uint8_t>({r, g, b});
            if(a == 0){
                data[i] = 0;
                continue;
            }

            r = colorf::round255(1.0 * r * 255.0 / a);
            g = colorf::round255(1.0 * g * 255.0 / a);
            b = colorf::round255(1.0 * b * 255.0 / a);

            data[i] = colorf::RGBA(r, g, b, a);
        }
    }

    inline std::tuple<const uint32_t *, size_t, size_t> createShadow(std::vector<uint32_t> &dst, bool proj, const uint32_t *src, size_t srcW, size_t srcH, uint32_t shadowColor)
    {
        fflassert(src);
        fflassert(srcW > 0);
        fflassert(srcH > 0);

        // shadow size depends on project or not
        //
        //      project :  (srcW + srcH / 2) x (srcH / 2 + 1)
        //              :  (srcW x srcH)
        //

        const size_t dstW = proj ? (srcW + srcH / 2) : srcW;
        const size_t dstH = proj ? (   1 + srcH / 2) : srcH;

        dst.clear();
        dst.resize(dstW * dstH, 0X00000000);

        if(proj){
            for(size_t y = 0; y < srcH; ++y){
                const size_t yproj = y - y / 2;
                for(size_t x = 0; x < srcW; ++x){
                    const size_t xproj = x + (srcH - y) / 2;
                    if(colorf::A(src[y * srcW + x])){
                        dst[yproj * dstW + xproj] = shadowColor;
                    }
                }
            }
        }
        else{
            for(size_t y = 0; y < srcH; ++y){
                for(size_t x = 0; x < srcW; ++x){
                    if(colorf::A(src[y * srcW + x])){
                        dst[y * srcW + x] = shadowColor;
                    }
                }
            }
        }

        return {dst.data(), dstW, dstH};
    }
}
