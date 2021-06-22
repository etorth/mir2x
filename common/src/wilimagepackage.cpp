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

#include "alphaf.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "wilimagepackage.hpp"

static void memcpy_color_u16_2_u32(uint32_t *dst, const uint16_t *src, size_t n)
{
    fflassert(src);
    fflassert(dst);

    for(size_t i = 0; i < n; i++){
        dst[i] = colorf::RGBA(to_u8((src[i] & 0XF800) >> 8), to_u8((src[i] & 0X07E0) >> 3), to_u8((src[i] & 0X001F) << 3), 0XFF);
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

std::array<const uint32_t *, 3> WilImagePackage::decode(bool mergeLayer, bool removeShadow, bool autoAlpha)
{
    fflassert(currImageValid());

    m_decodeBuf0.clear();
    m_decodeBuf1.clear();
    m_decodeBuf2.clear();

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
            const int rowLeftCount = std::max<int>(0, m_currImageInfo.width - to_d(dstNowPosInRow));
            const int rowCopyCount = std::min<int>(hdCopyCount, rowLeftCount);

            switch(hdCode){
                case 0XC0: // transparent
                    {
                        break;
                    }
                case 0XC1:
                    {
                        if(m_decodeBuf0.empty()){
                            m_decodeBuf0.resize(m_currImageInfo.width * m_currImageInfo.height, 0X00000000);
                        }

                        memcpy_color_u16_2_u32(m_decodeBuf0.data() + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount);
                        srcNowPos += hdCopyCount;
                        break;
                    }
                case 0XC2:
                    {
                        if(mergeLayer){
                            if(m_decodeBuf0.empty()){
                                m_decodeBuf0.resize(m_currImageInfo.width * m_currImageInfo.height, 0X00000000);
                            }
                            memcpy_color_u16_2_u32(m_decodeBuf0.data() + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount);
                        }
                        else{
                            if(m_decodeBuf1.empty()){
                                m_decodeBuf1.resize(m_currImageInfo.width * m_currImageInfo.height, 0X00000000);
                            }
                            memcpy_color_u16_2_u32(m_decodeBuf1.data() + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount);
                        }

                        srcNowPos += hdCopyCount;
                        break;
                    }
                case 0XC3:
                    {
                        if(mergeLayer){
                            if(m_decodeBuf0.empty()){
                                m_decodeBuf0.resize(m_currImageInfo.width * m_currImageInfo.height, 0X00000000);
                            }
                            memcpy_color_u16_2_u32(m_decodeBuf0.data() + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount);
                        }
                        else{
                            if(m_decodeBuf2.empty()){
                                m_decodeBuf2.resize(m_currImageInfo.width * m_currImageInfo.height, 0X00000000);
                            }
                            memcpy_color_u16_2_u32(m_decodeBuf2.data() + row * m_currImageInfo.width + dstNowPosInRow, m_currImageBuffer.data() + srcNowPos, rowCopyCount);
                        }

                        srcNowPos += hdCopyCount;
                        break;
                    }
                default:
                    {
                        throw bad_reach();
                    }
            }
            dstNowPosInRow += hdCopyCount;
        }

        // TODO wired code detected
        // found image with dstNowPosInRow > m_currImageInfo.width, i.e.: W-Hum.wil index 09010

        // this means the row data can overflow to next row, suprcode/mir2 checks each u16 to avoid this
        // if not at last row it's fine since decoding for next row override it, but the last row may have issue

        srcEndPos++;
        srcBeginPos = srcEndPos;
    }

    if(removeShadow && !m_decodeBuf0.empty()){
        alphaf::removeShadowMosaic(m_decodeBuf0.data(), m_currImageInfo.width, m_currImageInfo.height, colorf::BLACK + colorf::A_SHF(0X80));
    }

    if(autoAlpha && !m_decodeBuf0.empty()){
        alphaf::autoAlpha(m_decodeBuf0.data(), m_currImageInfo.width * m_currImageInfo.height);
    }

    return
    {
        m_decodeBuf0.empty() ? nullptr : m_decodeBuf0.data(),
        m_decodeBuf1.empty() ? nullptr : m_decodeBuf1.data(),
        m_decodeBuf2.empty() ? nullptr : m_decodeBuf2.data(),
    };
}
