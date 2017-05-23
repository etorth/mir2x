/*
 * =====================================================================================
 *
 *       Filename: imagedb.hpp
 *        Created: 02/14/2016 16:33:12
 *  Last Modified: 05/22/2017 17:32:48
 *
 *    Description: Handle operation against wilimagepackage
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

#include "wilimagepackage.hpp"

class ImageDB
{
    public:
        ImageDB();
       ~ImageDB();

    public:
        bool LoadDB(const char *);
        bool Valid(uint8_t, uint16_t);

    public:
        int FastW(uint8_t);
        int FastH(uint8_t);
        int W(uint8_t, uint16_t);
        int H(uint8_t, uint16_t);

    public:
        const uint32_t *FastDecode(uint8_t, uint32_t, uint32_t, uint32_t);
        const uint32_t *Decode(uint8_t, uint16_t, uint32_t, uint32_t, uint32_t);

    private:
        void ExtendBuf(int);
        bool Load(uint8_t, const char *, const char *, const char *);

    private:
        std::vector<uint8_t> m_Buf;
        std::array<WilImagePackage, 256> m_ImagePackage;
};
