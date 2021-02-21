/*
 * =====================================================================================
 *
 *       Filename: colorf.hpp
 *        Created: 03/31/2016 19:46:27
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
#include <array>
#include <cstdint>
#include <SDL2/SDL.h>

namespace colorf
{
    enum ColorMask: uint32_t
    {
        MASK_R  = 0XFF000000,
        MASK_G  = 0X00FF0000,
        MASK_B  = 0X0000FF00,
        MASK_A  = 0X000000FF,
    };

    enum ColorShift: int
    {
        SHIFT_R = 24,
        SHIFT_G = 16,
        SHIFT_B =  8,
        SHIFT_A =  0,
    };

    constexpr uint8_t R(uint32_t nRGBA)
    {
        return (nRGBA & MASK_R) >> SHIFT_R;
    }

    constexpr uint8_t G(uint32_t nRGBA)
    {
        return (nRGBA & MASK_G) >> SHIFT_G;
    }

    constexpr uint8_t B(uint32_t nRGBA)
    {
        return (nRGBA & MASK_B) >> SHIFT_B;
    }

    constexpr uint8_t A(uint32_t nRGBA)
    {
        return (nRGBA & MASK_A) >> SHIFT_A;
    }

    constexpr uint32_t RGBMask(uint32_t color)
    {
        return color & 0XFFFFFF00;
    }

    constexpr uint32_t RGBA(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
    {
        return ((uint32_t)(nR) << SHIFT_R) | ((uint32_t)(nG) << SHIFT_G) | ((uint32_t)(nB) << SHIFT_B)  | ((uint32_t)(nA) << SHIFT_A);
    }

    constexpr uint32_t ARGB(uint8_t nA, uint8_t nR, uint8_t nG, uint8_t nB)
    {
        return ((uint32_t)(nA) << 24) | (RGBA(nR, nG, nB, nA) >> 8);
    }

    constexpr uint32_t RGBA2ARGB(uint32_t nRGBA)
    {
        return ARGB(R(nRGBA), G(nRGBA), B(nRGBA), A(nRGBA));
    }

    constexpr uint32_t ARGB2RGBA(uint32_t nARGB)
    {
        return (nARGB << 8) | (nARGB >> 24);
    }

    constexpr uint32_t ABGR2RGBA(uint32_t colorABGR)
    {
        return RGBA((colorABGR & 0X000000FF), (colorABGR & 0X0000FF00) >> 8, (colorABGR & 0X00FF0000) >> 16, (colorABGR & 0XFF000000) >> 24);
    }

    constexpr uint32_t RGBA2ABGR(uint32_t colorRGBA)
    {
        return ((uint32_t)(R(colorRGBA)) << 0) | ((uint32_t)(G(colorRGBA)) << 8) | ((uint32_t)(B(colorRGBA)) << 16) | ((uint32_t)(A(colorRGBA)) << 24);
    }

    template<typename T> constexpr uint8_t round255(T nValue)
    {
        if(nValue < T(0)){
            return 0;
        }

        if(nValue > T(255)){
            return 255;
        }

        return (uint8_t)(nValue);
    }

    constexpr uint32_t RGBA_F(double fR, double fG, double fB, double fA)
    {
        return RGBA(round255(fR * 255.0), round255(fG * 255.0), round255(fB * 255.0), round255(fA * 255.0));
    }

    constexpr uint32_t RED     = RGBA(0XFF, 0X00, 0X00, 0X00);
    constexpr uint32_t GREEN   = RGBA(0X00, 0XFF, 0X00, 0X00);
    constexpr uint32_t BLUE    = RGBA(0X00, 0X00, 0XFF, 0X00);

    constexpr uint32_t BLACK   = RGBA(0X00, 0X00, 0X00, 0X00);
    constexpr uint32_t WHITE   = RGBA(0XFF, 0XFF, 0XFF, 0X00);
    constexpr uint32_t GREY    = RGBA(0X80, 0X80, 0X80, 0X00);
    constexpr uint32_t YELLOW  = RGBA(0XFF, 0XFF, 0X00, 0X00);
    constexpr uint32_t PURPLE  = RGBA(0XAB, 0X27, 0X4F, 0X00);


    constexpr uint32_t renderRGBA(uint32_t dstColor, uint32_t srcColor)
    {
        auto dstR = R(dstColor);
        auto dstG = G(dstColor);
        auto dstB = B(dstColor);
        auto dstA = A(dstColor);

        auto srcR = R(srcColor);
        auto srcG = G(srcColor);
        auto srcB = B(srcColor);
        auto srcA = A(srcColor);

        double fAlpha = srcA / 255.0;

        dstR = round255(fAlpha * srcR + (1.0 - fAlpha) * dstR);
        dstG = round255(fAlpha * srcG + (1.0 - fAlpha) * dstG);
        dstB = round255(fAlpha * srcB + (1.0 - fAlpha) * dstB);
        dstA = round255(         srcA + (1.0 - fAlpha) * dstA);

        return RGBA(dstR, dstG, dstB, dstA);
    }

    constexpr uint32_t renderABGR(uint32_t dstColor, uint32_t srcColor)
    {
        uint8_t dstR = (dstColor & 0X000000FF) >>  0;
        uint8_t dstG = (dstColor & 0X0000FF00) >>  8;
        uint8_t dstB = (dstColor & 0X00FF0000) >> 16;
        uint8_t dstA = (dstColor & 0XFF000000) >> 24;

        uint8_t srcR = (srcColor & 0X000000FF) >>  0;
        uint8_t srcG = (srcColor & 0X0000FF00) >>  8;
        uint8_t srcB = (srcColor & 0X00FF0000) >> 16;
        uint8_t srcA = (srcColor & 0XFF000000) >> 24;

        const double alpha = srcA / 255.0;

        dstR = round255(alpha * srcR + (1.0 - alpha) * dstR);
        dstG = round255(alpha * srcG + (1.0 - alpha) * dstG);
        dstB = round255(alpha * srcB + (1.0 - alpha) * dstB);
        dstA = round255(        srcA + (1.0 - alpha) * dstA);

        return ((uint32_t)(dstA) << 24) | ((uint32_t)(dstB) << 16) | ((uint32_t)(dstG) <<  8) | ((uint32_t)(dstR) <<  0);
    }

    template<size_t GradientCount> std::array<uint32_t, GradientCount> GradientColor(uint32_t nColor0, uint32_t nColor1)
    {
        static_assert(GradientCount >= 2);
        std::array<uint32_t, GradientCount> stvGradientColor;

        auto fDR = ((float)(R(nColor1)) - (float)(R(nColor0))) / (GradientCount - 1.0);
        auto fDG = ((float)(G(nColor1)) - (float)(G(nColor0))) / (GradientCount - 1.0);
        auto fDB = ((float)(B(nColor1)) - (float)(B(nColor0))) / (GradientCount - 1.0);
        auto fDA = ((float)(A(nColor1)) - (float)(A(nColor0))) / (GradientCount - 1.0);

        for(size_t nIndex = 0; nIndex < GradientCount; ++nIndex){
            uint8_t nR = round255(R(nColor0) + nIndex * fDR);
            uint8_t nG = round255(G(nColor0) + nIndex * fDG);
            uint8_t nB = round255(B(nColor0) + nIndex * fDB);
            uint8_t nA = round255(A(nColor0) + nIndex * fDA);
            stvGradientColor[nIndex] = RGBA(nR, nG, nB, nA);
        }
        return stvGradientColor;
    }

    inline SDL_Color RGBA2Color(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
    {
        SDL_Color stColor;
        stColor.r = nR;
        stColor.g = nG;
        stColor.b = nB;
        stColor.a = nA;
        return stColor;
    }

    inline SDL_Color RGBA2Color(uint32_t nColor)
    {
        return RGBA2Color(colorf::R(nColor), colorf::G(nColor), colorf::B(nColor), colorf::A(nColor));
    }

    inline uint32_t Color2RGBA(const SDL_Color &rstColor)
    {
        return RGBA(rstColor.r, rstColor.g, rstColor.b, rstColor.a);
    }

    inline uint32_t CompColor(uint32_t nColor)
    {
        return RGBA(255 - R(nColor), 255 - G(nColor), 255 - B(nColor), 255 - A(nColor));
    }

    uint32_t string2RGBA(const char *);
}
