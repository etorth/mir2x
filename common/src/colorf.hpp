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
    constexpr uint32_t R_MASK = 0X000000FF;
    constexpr uint32_t G_MASK = 0X0000FF00;
    constexpr uint32_t B_MASK = 0X00FF0000;
    constexpr uint32_t A_MASK = 0XFF000000;

    constexpr int R_SHIFT =  0;
    constexpr int G_SHIFT =  8;
    constexpr int B_SHIFT = 16;
    constexpr int A_SHIFT = 24;

    constexpr uint8_t R(uint32_t color) { return (color & R_MASK) >> R_SHIFT; }
    constexpr uint8_t G(uint32_t color) { return (color & G_MASK) >> G_SHIFT; }
    constexpr uint8_t B(uint32_t color) { return (color & B_MASK) >> B_SHIFT; }
    constexpr uint8_t A(uint32_t color) { return (color & A_MASK) >> A_SHIFT; }

    constexpr uint32_t R_SHF(uint8_t r) { return (uint32_t)(r) << R_SHIFT; }
    constexpr uint32_t G_SHF(uint8_t g) { return (uint32_t)(g) << G_SHIFT; }
    constexpr uint32_t B_SHF(uint8_t b) { return (uint32_t)(b) << B_SHIFT; }
    constexpr uint32_t A_SHF(uint8_t a) { return (uint32_t)(a) << A_SHIFT; }

    constexpr uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        return R_SHF(r) | G_SHF(g) | B_SHF(b) | A_SHF(a);
    }

    constexpr uint32_t RGB(uint8_t r, uint8_t g, uint8_t b)
    {
        return R_SHF(r) | G_SHF(g) | B_SHF(b);
    }

    constexpr uint32_t RGBMask(uint32_t color)
    {
        return color & (R_MASK | G_MASK | B_MASK);
    }

    template<typename T> constexpr uint8_t round255(T val)
    {
        if(val <= T(0)){
            return 0;
        }

        if(val >= T(255)){
            return 255;
        }

        return (uint8_t)(val);
    }

    constexpr uint32_t RGBA_F(double fr, double fg, double fb, double fa)
    {
        return RGBA(round255(fr * 255.0), round255(fg * 255.0), round255(fb * 255.0), round255(fa * 255.0));
    }

    constexpr uint32_t RED     = RGB(0XFF, 0X00, 0X00);
    constexpr uint32_t GREEN   = RGB(0X00, 0XFF, 0X00);
    constexpr uint32_t BLUE    = RGB(0X00, 0X00, 0XFF);

    constexpr uint32_t YELLOW  = RGB(0XFF, 0XFF, 0X00);
    constexpr uint32_t CYAN    = RGB(0X00, 0XFF, 0XFF);
    constexpr uint32_t MAGENTA = RGB(0XFF, 0X00, 0XFF);

    constexpr uint32_t BLACK   = RGB(0X00, 0X00, 0X00);
    constexpr uint32_t GREY    = RGB(0X80, 0X80, 0X80);
    constexpr uint32_t WHITE   = RGB(0XFF, 0XFF, 0XFF);

    constexpr uint32_t modRGBA(uint32_t origColor, uint32_t modColor)
    {
        const float r_R = 1.0f * R(modColor) / 255.0;
        const float r_G = 1.0f * G(modColor) / 255.0;
        const float r_B = 1.0f * B(modColor) / 255.0;
        const float r_A = 1.0f * A(modColor) / 255.0;

        const auto newR = round255(r_R * R(origColor));
        const auto newG = round255(r_G * G(origColor));
        const auto newB = round255(r_B * B(origColor));
        const auto newA = round255(r_A * A(origColor));

        return RGBA(newR, newG, newB, newA);
    }

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

    constexpr uint32_t fadeRGBA(uint32_t fromColor, uint32_t toColor, float r)
    {
        const uint32_t fromR = colorf::R(fromColor);
        const uint32_t fromG = colorf::G(fromColor);
        const uint32_t fromB = colorf::B(fromColor);
        const uint32_t fromA = colorf::A(fromColor);

        const uint32_t toR = colorf::R(toColor);
        const uint32_t toG = colorf::G(toColor);
        const uint32_t toB = colorf::B(toColor);
        const uint32_t toA = colorf::A(toColor);

        const uint32_t dstR = round255(fromR * (1.0 - r) + toR * r);
        const uint32_t dstG = round255(fromG * (1.0 - r) + toG * r);
        const uint32_t dstB = round255(fromB * (1.0 - r) + toB * r);
        const uint32_t dstA = round255(fromA * (1.0 - r) + toA * r);

        return colorf::RGBA(dstR, dstG, dstB, dstA);
    }

    template<size_t N> std::array<uint32_t, N> gradColor(uint32_t beginColor, uint32_t endColor)
    {
        static_assert(N >= 2);
        std::array<uint32_t, N> result;

        const auto beginR = (float)(R(beginColor));
        const auto beginG = (float)(G(beginColor));
        const auto beginB = (float)(B(beginColor));
        const auto beginA = (float)(A(beginColor));

        const auto fdR = ((float)(R(endColor)) - beginR) / (N - 1.0);
        const auto fdG = ((float)(G(endColor)) - beginG) / (N - 1.0);
        const auto fdB = ((float)(B(endColor)) - beginB) / (N - 1.0);
        const auto fdA = ((float)(A(endColor)) - beginA) / (N - 1.0);

        for(size_t i = 0; i < N; ++i){
            result[i] = RGBA(round255(beginR + i * fdR), round255(beginG + i * fdG), round255(beginB + i * fdB), round255(beginA + i * fdA));
        }
        return result;
    }

    inline SDL_Color RGBA2SDLColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        SDL_Color color;
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
        return color;
    }

    inline SDL_Color RGBA2SDLColor(uint32_t color)
    {
        return RGBA2SDLColor(colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color));
    }

    inline uint32_t SDLColor2RGBA(const SDL_Color &color)
    {
        return RGBA(color.r, color.g, color.b, color.a);
    }

    inline uint32_t compColor(uint32_t color)
    {
        return RGBA(255 - R(color), 255 - G(color), 255 - B(color), 255 - A(color));
    }

    uint32_t string2RGBA(const char *);
}
