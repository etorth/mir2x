/*
 * =====================================================================================
 *
 *       Filename: colorfunc.hpp
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

namespace ColorFunc
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

    template<typename T> constexpr uint8_t Round255(T nValue)
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
        return RGBA(Round255(fR * 255.0), Round255(fG * 255.0), Round255(fB * 255.0), Round255(fA * 255.0));
    }

    constexpr uint32_t RED     = RGBA(0XFF, 0X00, 0X00, 0X00);
    constexpr uint32_t GREEN   = RGBA(0X00, 0XFF, 0X00, 0X00);
    constexpr uint32_t BLUE    = RGBA(0X00, 0X00, 0XFF, 0X00);

    constexpr uint32_t BLACK   = RGBA(0X00, 0X00, 0X00, 0X00);
    constexpr uint32_t WHITE   = RGBA(0XFF, 0XFF, 0XFF, 0X00);
    constexpr uint32_t YELLOW  = RGBA(0XFF, 0XFF, 0X00, 0X00);
    constexpr uint32_t PURPLE  = RGBA(0XAB, 0X27, 0X4F, 0X00);


    inline uint32_t RenderRGBA(uint32_t nDstColor, uint32_t nSrcColor)
    {
        auto nDstR = R(nDstColor);
        auto nDstG = G(nDstColor);
        auto nDstB = B(nDstColor);
        auto nDstA = A(nDstColor);

        auto nSrcR = R(nSrcColor);
        auto nSrcG = G(nSrcColor);
        auto nSrcB = B(nSrcColor);
        auto nSrcA = A(nSrcColor);

        double fAlpha = nSrcA / 255.0;

        nDstR = Round255(fAlpha * nSrcR + (1.0 - fAlpha) * nDstR);
        nDstG = Round255(fAlpha * nSrcG + (1.0 - fAlpha) * nDstG);
        nDstB = Round255(fAlpha * nSrcB + (1.0 - fAlpha) * nDstB);
        nDstA = Round255(         nSrcA + (1.0 - fAlpha) * nDstA);

        return RGBA(nDstR, nDstG, nDstB, nDstA);
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
            uint8_t nR = Round255(R(nColor0) + nIndex * fDR);
            uint8_t nG = Round255(G(nColor0) + nIndex * fDG);
            uint8_t nB = Round255(B(nColor0) + nIndex * fDB);
            uint8_t nA = Round255(A(nColor0) + nIndex * fDA);
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
        return RGBA2Color(ColorFunc::R(nColor), ColorFunc::G(nColor), ColorFunc::B(nColor), ColorFunc::A(nColor));
    }

    inline uint32_t Color2RGBA(const SDL_Color &rstColor)
    {
        return RGBA(rstColor.r, rstColor.g, rstColor.b, rstColor.a);
    }

    inline uint32_t CompColor(uint32_t nColor)
    {
        return RGBA(255 - R(nColor), 255 - G(nColor), 255 - B(nColor), 255 - A(nColor));
    }

    uint32_t String2RGBA(const char *);
}
