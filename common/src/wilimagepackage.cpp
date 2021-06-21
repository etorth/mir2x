/*
 * =====================================================================================
 *
 *       Filename: wilimagepackage.cpp
 *        Created: 02/14/2016 16:33:12
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

#include <cmath>
#include <string>
#include <cstdio>
#include <cstring>

#include "colorf.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "wilimagepackage.hpp"

static uint32_t color_u16_2_u32(uint16_t srcColor, uint32_t modColor)
{
    auto r = to_u8((srcColor & 0XF800) >> 8);
    auto g = to_u8((srcColor & 0X07E0) >> 3);
    auto b = to_u8((srcColor & 0X001F) << 3);

    if(colorf::maskRGB(modColor) != colorf::RGB(255, 255, 255)){
        r = colorf::round255(to_df(r) * colorf::R(modColor) / 255.0);
        g = colorf::round255(to_df(g) * colorf::G(modColor) / 255.0);
        b = colorf::round255(to_df(b) * colorf::B(modColor) / 255.0);
    }
    return colorf::RGBA(r, g, b, colorf::A(modColor));
}

static void memcpy_u16_2_u32(uint32_t *dst, const uint16_t *src, size_t n, uint32_t modColor)
{
    fflassert(src);
    fflassert(dst);

    for(size_t i = 0; i < n; i++){
        dst[i] = color_u16_2_u32(src[i], modColor);
    }
}

static void memset_u32(uint32_t *dst, size_t n, uint32_t src)
{
    fflassert(dst);
    for(size_t i = 0; i < n; i++){
        dst[i] = src;
    }
}

WilImagePackage::WilImagePackage(const char* wilFilePath, const char *wilFileName)
    : m_wilFile(make_fileptr(hasWilFile(wilFilePath, wilFileName, {"wil", "Wil", "WIL"}).c_str(), "rb"))
{
    read_fileptr(m_wilFile, &m_wilFileHeader, sizeof(m_wilFileHeader));

    wilOff(m_wilFileHeader.version); // check if version supported
    fflassert(m_wilFileHeader.imageCount >= 0);

    auto wixFile = make_fileptr(hasWilFile(wilFilePath, wilFileName, {"wix", "Wix", "WIX"}).c_str(), "rb");

    read_fileptr(wixFile, &m_wixImageInfo, sizeof(m_wixImageInfo));
    fflassert(m_wixImageInfo.indexCount >= 0);

    seek_fileptr(wixFile, wixOff(m_wilFileHeader.version), SEEK_SET);
    read_fileptr(wixFile, m_wilPositionList, m_wixImageInfo.indexCount);
}

const WILIMAGEINFO *WilImagePackage::setIndex(uint32_t imageIndex)
{
    if(m_currImageIndex.has_value() && m_currImageIndex.value() == imageIndex){
        return &m_currImageInfo;
    }

    m_currImageIndex.reset();
    if(false
            || imageIndex >= to_u32(m_wilPositionList.size())
            || imageIndex >= to_u32(m_wixImageInfo.indexCount)
            || m_wilPositionList.at(imageIndex) <= 0){
        return nullptr;
    }

    seek_fileptr(m_wilFile, m_wilPositionList[imageIndex], SEEK_SET);
    read_fileptr(m_wilFile, &m_currImageInfo, sizeof(m_currImageInfo));

    if(false
            || m_currImageInfo.width  <= 0
            || m_currImageInfo.height <= 0
            || m_currImageInfo.imageLength <= 0){
        return nullptr;
    }

    seek_fileptr(m_wilFile, m_wilPositionList[imageIndex] + wilOff(m_wilFileHeader.version), SEEK_SET);
    read_fileptr(m_wilFile, m_currImageBuffer, m_currImageInfo.imageLength);

    m_currImageIndex = imageIndex;
    return &m_currImageInfo;
}

void WilImagePackage::decode(uint32_t *imageBuffer, uint32_t color0, uint32_t color1, uint32_t color2)
{
    fflassert(imageBuffer);
    fflassert(currImageValid());

    // check decode in suprcode/mir2: LibraryEditor/Graphics/WeMadeLibrary.cs

    size_t srcBeginPos    = 0;
    size_t srcEndPos      = 0;
    size_t srcNowPos      = 0;
    size_t dstNowPosInRow = 0;

    for(int row = 0; row < m_currImageInfo.height; ++row){
        srcEndPos     += m_currImageBuffer[srcBeginPos++];
        srcNowPos      = srcBeginPos;
        dstNowPosInRow = 0;

        while(srcNowPos < srcEndPos){
            const uint16_t hdCode      = m_currImageBuffer[srcNowPos++];
            const uint16_t hdCopyCount = m_currImageBuffer[srcNowPos++];

            // TODO some images have row can overflow to next row, i.e.: W-Hum.wil index 09010
            // this can causes issue if currently is last row, if not last row then next row decoding can override it
            const int rowCopyCount = std::max<int>(0, m_currImageInfo.width - to_d(dstNowPosInRow));

            switch(hdCode){
                case 0XC0: // transparent
                    memset_u32(imageBuffer + row * m_currImageInfo.width + dstNowPosInRow, rowCopyCount, 0X00000000);
                    break;
                case 0XC1:
                    memcpy_u16_2_u32(imageBuffer + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount, color0);
                    srcNowPos += hdCopyCount;
                    break;
                case 0XC2:
                    memcpy_u16_2_u32(imageBuffer + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount, color1);
                    srcNowPos += hdCopyCount;
                    break;
                case 0XC3:
                    memcpy_u16_2_u32(imageBuffer + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount, color2);
                    srcNowPos += hdCopyCount;
                    break;
                default:
                    throw bad_reach();
            }
            dstNowPosInRow += hdCopyCount;
        }

        // TODO wired code detected
        // found image with dstNowPosInRow > m_currImageInfo.width, i.e.: W-Hum.wil index 09010

        // this means the row data can overflow to next row, suprcode/mir2 checks each u16 to avoid this
        // if not at last row it's fine since decoding for next row override it, but the last row may have issue

        if(to_d(dstNowPosInRow) < m_currImageInfo.width){
            memset_u32(imageBuffer + row * m_currImageInfo.width + dstNowPosInRow, m_currImageInfo.width - dstNowPosInRow, 0X00000000);
        }

        srcEndPos++;
        srcBeginPos = srcEndPos;
    }
}
