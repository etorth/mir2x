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

#include "totype.hpp"
#include "condcheck.hpp"
#include "wilimagepackage.hpp"

static uint32_t Color16To32Mapping(uint16_t srcColor, uint32_t chColor)
{
    uint8_t r  = to_u8((srcColor & 0XF800) >> 8);
    uint8_t g  = to_u8((srcColor & 0X07E0) >> 3);
    uint8_t b  = to_u8((srcColor & 0X001F) << 3);

    if((chColor & 0X00FFFFFF) != 0X00FFFFFF){
        uint8_t br = to_u8((chColor & 0X00FF0000) >> 16);
        uint8_t bg = to_u8((chColor & 0X0000FF00) >>  8);
        uint8_t bb = to_u8((chColor & 0X000000FF) >>  0);

        r = (br <= r) ? br : (uint8_t)std::lround(255.0 * r / br);
        g = (bg <= g) ? bg : (uint8_t)std::lround(255.0 * g / bg);
        b = (bb <= b) ? bb : (uint8_t)std::lround(255.0 * b / bb);
    }

    return (chColor & 0XFF000000) + ((to_u32(b)) << 16) + ((to_u32(g)) << 8) + to_u32(r);
}

static void Memcpy16To32(uint32_t *dst, const uint16_t *src, int32_t n, uint32_t dwColor)
{
    if(src && dst){
        for(int32_t ptr = 0; ptr < n; ptr++){
            dst[ptr] = Color16To32Mapping(src[ptr], dwColor);
        }
    }
}

static void MemSet32(uint32_t *dst, int n, uint32_t src)
{
    if(dst){
        for(int32_t ptr = 0; ptr < n; ptr++){
            dst[ptr] = src;
        }
    }
}

WilImagePackage::WilImagePackage()
    : m_currentImageIndex(-1)
    , m_currentImageValid(false)
    , m_currentImageBuffer(2048)
    , m_wilPosition(2048)
    , m_wilFile(nullptr)
{
    std::memset(&m_wixImageInfo,        0, sizeof(WIXIMAGEINFO ));
    std::memset(&m_currentWilImageInfo, 0, sizeof(WILIMAGEINFO ));
    std::memset(&m_wilFileHeader,       0, sizeof(WILFILEHEADER));
}

WilImagePackage::~WilImagePackage()
{
    if(m_wilFile){ std::fclose(m_wilFile); }
}

bool WilImagePackage::SetIndex(uint32_t dwIndex)
{
    if(false
            || m_wilFile == nullptr
            || dwIndex   >= to_u32(m_wilPosition.size())
            || dwIndex   >= to_u32(m_wixImageInfo.nIndexCount)){

        m_currentImageValid = false;
        m_currentImageIndex = -1;
        return false;
    }

    if((dwIndex == to_u32(m_currentImageIndex)) && m_currentImageValid){
        return true;
    }

    m_currentImageIndex = (int32_t)(dwIndex);
    if(m_wilPosition[dwIndex] <= 0){
        // invalid image but set index operation succeeds
        m_currentImageValid = false;
        return true;
    }

    if(std::fseek(m_wilFile, m_wilPosition[dwIndex], SEEK_SET)){
        m_currentImageValid = false;
        return false;
    }

    if(std::fread(&m_currentWilImageInfo, sizeof(WILIMAGEINFO), 1, m_wilFile) != 1){
        m_currentImageValid = false;
        return false;
    }

    m_currentImageBuffer.resize(0);
    m_currentImageBuffer.resize(m_currentWilImageInfo.dwImageLength);

    auto nWilOffset = wilOff(m_wilFileHeader.shVer);
    if(nWilOffset < 0){
        m_currentImageValid = false;
        return false;
    }

    std::fseek(m_wilFile, m_wilPosition[dwIndex], SEEK_SET);
    std::fseek(m_wilFile, nWilOffset, SEEK_CUR);

    auto nCurrDataLen = m_currentWilImageInfo.dwImageLength;
    if(std::fread(&(m_currentImageBuffer[0]), sizeof(uint16_t), nCurrDataLen, m_wilFile) != nCurrDataLen){
        m_currentImageValid = false;
        return false;
    }

    m_currentImageValid = true;
    return true;
}

bool WilImagePackage::Load(const char* wilFilePath, const char *wilFileName, const char *)
{
    // 1. set current image invalid if load or reload
    m_currentImageValid = false;

    auto fnReleaseFile = [this]()
    {
        if(m_wilFile){ std::fclose(m_wilFile); m_wilFile = nullptr; }
    };

    // 2. load wil image library
    fnReleaseFile();

    if(!m_wilFile){ m_wilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".wil").c_str(), "rb"); }
    if(!m_wilFile){ m_wilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".Wil").c_str(), "rb"); }
    if(!m_wilFile){ m_wilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".WIL").c_str(), "rb"); }

    if(m_wilFile == nullptr){ return false; }

    if(std::fread(&m_wilFileHeader, sizeof(WILFILEHEADER), 1, m_wilFile) != 1){
        fnReleaseFile(); return false;
    }

    // 3. load wix index library
    FILE *hWixFile = nullptr;
    if(!hWixFile){ hWixFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".wix").c_str(), "rb"); }
    if(!hWixFile){ hWixFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".Wix").c_str(), "rb"); }
    if(!hWixFile){ hWixFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".WIX").c_str(), "rb"); }

    if(hWixFile == nullptr){
        fnReleaseFile(); return false;
    }

    if(std::fread(&m_wixImageInfo, sizeof(WIXIMAGEINFO), 1, hWixFile) != 1){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    m_wilPosition.resize(0);
    m_wilPosition.resize(m_wixImageInfo.nIndexCount);

    auto nWixOffset = wixOff(m_wilFileHeader.shVer);
    if(nWixOffset < 0){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    std::fseek(hWixFile, nWixOffset, SEEK_SET);

    auto nCurrDataLen = m_wixImageInfo.nIndexCount;
    if(std::fread(&(m_wilPosition[0]), sizeof(int32_t), nCurrDataLen, hWixFile) != (size_t)(nCurrDataLen)){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    // set current index to an invalid index
    m_currentImageIndex = -1;
    return true;
}

const uint16_t *WilImagePackage::CurrentImageBuffer()
{
    if(m_currentImageValid){
        return &(m_currentImageBuffer[0]);
    }else{
        return nullptr;
    }
}

const WILIMAGEINFO &WilImagePackage::CurrentImageInfo()
{
    if(m_currentImageValid){
        return m_currentWilImageInfo;
    }

    const static WILIMAGEINFO s_emptyImageInfo{};
    return s_emptyImageInfo;
}

void WilImagePackage::Decode(uint32_t *rectImageBuffer, uint32_t dwColor0, uint32_t dwColor1, uint32_t dwColor2)
{
    auto pwSrc   = &(m_currentImageBuffer[0]);
    int  nWidth  = m_currentWilImageInfo.shWidth;
    int  nHeight = m_currentWilImageInfo.shHeight;

    size_t srcBeginPos    = 0;
    size_t srcEndPos      = 0;
    size_t srcNowPos      = 0;
    size_t dstNowPosInRow = 0;

    for(int nRow = 0; nRow < nHeight; ++nRow){
        srcEndPos     += pwSrc[srcBeginPos++];
        srcNowPos      = srcBeginPos;
        dstNowPosInRow = 0;

        while(srcNowPos < srcEndPos){
            uint16_t  hdCode  = pwSrc[srcNowPos++];
            uint16_t  cntCopy = pwSrc[srcNowPos++];

            switch(hdCode){
                case 0XC0: // jump code
                    MemSet32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, cntCopy, 0X00000000);
                    break;
                case 0XC1:
                    Memcpy16To32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, pwSrc + srcNowPos, cntCopy, dwColor0);
                    srcNowPos += cntCopy;
                    break;
                case 0XC2:
                    Memcpy16To32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, pwSrc + srcNowPos, cntCopy, dwColor1);
                    srcNowPos += cntCopy;
                    break;
                case 0XC3:
                    Memcpy16To32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, pwSrc + srcNowPos, cntCopy, dwColor2);
                    srcNowPos += cntCopy;
                    break;
                default:
                    // printf("Warning: unexpected hard code in WilImagePackage::Decode()\n");
                    break;
            }
            dstNowPosInRow += cntCopy;
        }

        // actually I don't think it's needed here, but put it here
        MemSet32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, nWidth - dstNowPosInRow, 0X00000000);
        srcEndPos++;
        srcBeginPos = srcEndPos;
    }
}

int32_t WilImagePackage::ImageCount()
{
    return m_wilFile ? m_wilFileHeader.nImageCount : 0;
}

int32_t WilImagePackage::IndexCount()
{
    return m_wilFile ? m_wixImageInfo.nIndexCount: 0;
}

bool WilImagePackage::CurrentImageValid()
{
    return m_currentImageValid;
}

int16_t WilImagePackage::Version()
{
    return m_wilFileHeader.shVer;
}

const WILFILEHEADER &WilImagePackage::HeaderInfo() const
{
    return m_wilFileHeader;
}
