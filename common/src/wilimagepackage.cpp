/*
 * =====================================================================================
 *
 *       Filename: wilimagepackage.cpp
 *        Created: 02/14/2016 16:33:12
 *  Last Modified: 09/04/2017 02:35:29
 *
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

#include "condcheck.hpp"
#include "wilimagepackage.hpp"

static uint32_t Color16To32Mapping(uint16_t srcColor, uint32_t chColor)
{
    uint8_t r  = (uint8_t)((srcColor & 0XF800) >> 8);
    uint8_t g  = (uint8_t)((srcColor & 0X07E0) >> 3);
    uint8_t b  = (uint8_t)((srcColor & 0X001F) << 3);

    if((chColor & 0X00FFFFFF) != 0X00FFFFFF){
        uint8_t br = (uint8_t)((chColor & 0X00FF0000) >> 16);
        uint8_t bg = (uint8_t)((chColor & 0X0000FF00) >>  8);
        uint8_t bb = (uint8_t)((chColor & 0X000000FF) >>  0);

        r = (br <= r) ? br : (uint8_t)std::lround(255.0 * r / br);
        g = (bg <= g) ? bg : (uint8_t)std::lround(255.0 * g / bg);
        b = (bb <= b) ? bb : (uint8_t)std::lround(255.0 * b / bb);
    }

    return (chColor & 0XFF000000) + (((uint32_t)(b)) << 16) + (((uint32_t)(g)) << 8) + (uint32_t)(r);
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
    : m_CurrentImageIndex(-1)
    , m_CurrentImageValid(false)
    , m_CurrentImageBuffer(2048)
    , m_WilPosition(2048)
    , m_WilFile(nullptr)
{
    std::memset(&m_WixImageInfo,        0, sizeof(WIXIMAGEINFO ));
    std::memset(&m_CurrentWilImageInfo, 0, sizeof(WILIMAGEINFO ));
    std::memset(&m_WilFileHeader,       0, sizeof(WILFILEHEADER));
}

WilImagePackage::~WilImagePackage()
{
    if(m_WilFile){ std::fclose(m_WilFile); }
}

bool WilImagePackage::SetIndex(uint32_t dwIndex)
{
    if(false
            || m_WilFile == nullptr
            || dwIndex   >= (uint32_t)(m_WilPosition.size())
            || dwIndex   >= (uint32_t)(m_WixImageInfo.nIndexCount)){

        m_CurrentImageValid = false;
        m_CurrentImageIndex = -1;
        return false;
    }

    if((dwIndex == (uint32_t)(m_CurrentImageIndex)) && m_CurrentImageValid){
        return true;
    }

    m_CurrentImageIndex = (int32_t)(dwIndex);
    if(m_WilPosition[dwIndex] <= 0){
        // invalid image but set index operation succeeds
        m_CurrentImageValid = false;
        return true;
    }

    if(std::fseek(m_WilFile, m_WilPosition[dwIndex], SEEK_SET)){
        m_CurrentImageValid = false;
        return false;
    }

    if(std::fread(&m_CurrentWilImageInfo, sizeof(WILIMAGEINFO), 1, m_WilFile) != 1){
        m_CurrentImageValid = false;
        return false;
    }

    m_CurrentImageBuffer.resize(0);
    m_CurrentImageBuffer.resize(m_CurrentWilImageInfo.dwImageLength);

    auto nWilOffset = WilOffset(m_WilFileHeader.shVer);
    if(nWilOffset < 0){
        m_CurrentImageValid = false;
        return false;
    }

    std::fseek(m_WilFile, m_WilPosition[dwIndex], SEEK_SET);
    std::fseek(m_WilFile, nWilOffset, SEEK_CUR);

    auto nCurrDataLen = m_CurrentWilImageInfo.dwImageLength;
    if(std::fread(&(m_CurrentImageBuffer[0]), sizeof(uint16_t), nCurrDataLen, m_WilFile) != nCurrDataLen){
        m_CurrentImageValid = false;
        return false;
    }

    m_CurrentImageValid = true;
    return true;
}

bool WilImagePackage::Load(const char* wilFilePath, const char *wilFileName, const char *)
{
    // 1. set current image invalid if load or reload
    m_CurrentImageValid = false;

    auto fnReleaseFile = [this]()
    {
        if(m_WilFile){ std::fclose(m_WilFile); m_WilFile = nullptr; }
    };

    // 2. load wil image library
    fnReleaseFile();

    if(!m_WilFile){ m_WilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".wil").c_str(), "rb"); }
    if(!m_WilFile){ m_WilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".Wil").c_str(), "rb"); }
    if(!m_WilFile){ m_WilFile = std::fopen((std::string(wilFilePath) + "/" + wilFileName + ".WIL").c_str(), "rb"); }

    if(m_WilFile == nullptr){ return false; }

    if(std::fread(&m_WilFileHeader, sizeof(WILFILEHEADER), 1, m_WilFile) != 1){
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

    if(std::fread(&m_WixImageInfo, sizeof(WIXIMAGEINFO), 1, hWixFile) != 1){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    m_WilPosition.resize(0);
    m_WilPosition.resize(m_WixImageInfo.nIndexCount);

    auto nWixOffset = WixOffset(m_WilFileHeader.shVer);
    if(nWixOffset < 0){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    std::fseek(hWixFile, nWixOffset, SEEK_SET);

    auto nCurrDataLen = m_WixImageInfo.nIndexCount;
    if(std::fread(&(m_WilPosition[0]), sizeof(int32_t), nCurrDataLen, hWixFile) != (size_t)(nCurrDataLen)){
        std::fclose(hWixFile); fnReleaseFile(); return false;
    }

    // set current index to an invalid index
    m_CurrentImageIndex = -1;
    return true;
}

const uint16_t *WilImagePackage::CurrentImageBuffer()
{
    if(m_CurrentImageValid){
        return &(m_CurrentImageBuffer[0]);
    }else{
        return nullptr;
    }
}

const WILIMAGEINFO &WilImagePackage::CurrentImageInfo()
{
    if(m_CurrentImageValid){
        return m_CurrentWilImageInfo;
    }else{
        const static WILIMAGEINFO rstEmptyImageInfo = []()
        {
            WILIMAGEINFO stWilImageInfo;
            std::memset(&stWilImageInfo, 0, sizeof(stWilImageInfo));
            return stWilImageInfo;
        }();

        return rstEmptyImageInfo;
    }
}

void WilImagePackage::Decode(uint32_t *rectImageBuffer, uint32_t dwColor0, uint32_t dwColor1, uint32_t dwColor2)
{
    auto pwSrc   = &(m_CurrentImageBuffer[0]);
    int  nWidth  = m_CurrentWilImageInfo.shWidth;
    int  nHeight = m_CurrentWilImageInfo.shHeight;

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
    return m_WilFile ? m_WilFileHeader.nImageCount : 0;
}

int32_t WilImagePackage::IndexCount()
{
    return m_WilFile ? m_WixImageInfo.nIndexCount: 0;
}

bool WilImagePackage::CurrentImageValid()
{
    return m_CurrentImageValid;
}

int16_t WilImagePackage::Version()
{
    return m_WilFileHeader.shVer;
}

const WILFILEHEADER &WilImagePackage::HeaderInfo() const
{
    return m_WilFileHeader;
}

int WilImagePackage::WixOffset(int nVersion)
{
    switch(nVersion){
        case 17:
            {
                return 24;
            }
        case 5000:
            {
                return 28;
            }
        case 6000:
            {
                return 32;
            }
        default:
            {
                return -1;
            }
    }
}

int WilImagePackage::WilOffset(int nVersion)
{
    switch(nVersion){
        case 17:
            {
                return 17;
            }
        case 5000:
            {
                return 21;
            }
        case 6000:
            {
                return 21;
            }
        default:
            {
                return -1;
            }
    }
}
