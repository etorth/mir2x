#include "wilimagepackage.hpp"
#include <cstring>
#include <string>
#include <cmath>
#include <cstdio>

static uint32_t Color16To32Mapping(uint16_t srcColor, uint32_t chColor)
{
    // ARGB
    uint8_t r, g, b, br, bg, bb;

    r  = (uint8_t)((srcColor & 0XF800) >> 8);
    g  = (uint8_t)((srcColor & 0X07E0) >> 3);
    b  = (uint8_t)((srcColor & 0X001F) << 3);

    if((chColor & 0X00FFFFFF) != 0X00FFFFFF){
        br = (uint8_t)((chColor & 0X00FF0000) >> 16);
        bg = (uint8_t)((chColor & 0X0000FF00) >>  8);
        bb = (uint8_t)((chColor & 0X000000FF) >>  0);

        r = (br <= r) ? br : (uint8_t)std::lround(255.0 * r / br);
        g = (bg <= g) ? bg : (uint8_t)std::lround(255.0 * g / bg);
        b = (bb <= b) ? bb : (uint8_t)std::lround(255.0 * b / bb);
    }

    // return (chColor & 0XFF000000) + (((uint32_t)r) << 16) + (((uint32_t)g) << 8) + (uint32_t)b;
    return (chColor & 0XFF000000) + (((uint32_t)b) << 16) + (((uint32_t)g) << 8) + (uint32_t)r;
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
    : m_CurrentImageIndex(0)
    , m_ImageCount(0)
    , m_CurrentImageValid(false)
    , m_CurrentImageBuffer(nullptr)
    , m_CurrentImageBufferLength(0)
    , m_Version(0)
    , m_FP(nullptr)
{
    std::memset(&m_WixImageInfo, 0, sizeof(WIXIMAGEINFO));
    std::memset(&m_CurrentWilImageInfo, 0, sizeof(WILIMAGEINFO));
    std::memset(&m_DumbWilImageInfo, 0, sizeof(WILIMAGEINFO));
}

WilImagePackage::~WilImagePackage()
{
    if(m_FP){
        fclose(m_FP); m_FP = nullptr;
    }

    delete []m_CurrentImageBuffer;
    delete [](m_WixImageInfo.pnPosition);
}

bool WilImagePackage::SetIndex(uint32_t dwIndex)
{
    if(m_FP == nullptr || dwIndex >= (uint32_t)(m_WixImageInfo.nIndexCount)){
        m_CurrentImageValid = false;
        return false;
    }

    if(dwIndex == m_CurrentImageIndex && m_CurrentImageValid){
        return true;
    }

    m_CurrentImageIndex = dwIndex;
    if(m_WixImageInfo.pnPosition[dwIndex] <= 0){
        // invalid image but set index operation succeeds
        m_CurrentImageValid = false;
        return true;
    }

    if(fseek(m_FP, m_WixImageInfo.pnPosition[dwIndex], SEEK_SET)){
        m_CurrentImageValid = false;
        return false;
    }

    if(fread(&m_CurrentWilImageInfo, sizeof(WILIMAGEINFO), 1, m_FP) != 1){
        m_CurrentImageValid = false;
        return false;
    }

    if(m_CurrentWilImageInfo.dwImageLength > m_CurrentImageBufferLength){
        delete []m_CurrentImageBuffer;
        m_CurrentImageBuffer       = new uint16_t[m_CurrentWilImageInfo.dwImageLength];
        m_CurrentImageBufferLength = m_CurrentWilImageInfo.dwImageLength;
    }

    if(m_Version == 5000){
        // we need to skip two zeros, no idea why
        uint16_t wSkip[2];
        if(fread(wSkip, sizeof(uint16_t), 2, m_FP) != 2){
            m_CurrentImageValid = false;
            return false;
        }
        if(wSkip[0] != 0 || wSkip[1] != 0){
            printf("warning: wSkip is not zero\n");
        }
    }

    if(fread(m_CurrentImageBuffer, sizeof(uint16_t),
                m_CurrentWilImageInfo.dwImageLength, m_FP) != m_CurrentWilImageInfo.dwImageLength){
        m_CurrentImageValid = false;
        return false;
    }

    m_CurrentImageValid = true;
    return true;
}

bool WilImagePackage::Load(const char* wilFilePath, const char *wilFileName, const char *)
{
    m_CurrentImageValid = false;
    if(m_FP){
        fclose(m_FP); m_FP = nullptr;
    }

    auto hWixFile = fopen((std::string(wilFilePath) + "/" + wilFileName + ".wix").c_str(), "rb");
    if(hWixFile == nullptr){
        return false;
    }

    // when first time to load, m_WixImageInfo.nIndexCount was set as 0
    auto oldWixIndexCount = m_WixImageInfo.nIndexCount;

    if(fread(&m_WixImageInfo, sizeof(WIXIMAGEINFO) - sizeof(int32_t*), 1, hWixFile) != 1){
        fclose(hWixFile);
        return false;
    }

    if(oldWixIndexCount < m_WixImageInfo.nIndexCount){
        delete []m_WixImageInfo.pnPosition;
        m_WixImageInfo.pnPosition = new int32_t[m_WixImageInfo.nIndexCount];
    }

    if(fread(m_WixImageInfo.pnPosition, sizeof(int32_t), 
                m_WixImageInfo.nIndexCount, hWixFile) != (size_t)m_WixImageInfo.nIndexCount){
        fclose(hWixFile);
        return false;
    }

    // set current index to an invalid index
    m_CurrentImageIndex = (uint32_t)m_WixImageInfo.nIndexCount;

    if(!m_FP){ m_FP = fopen((std::string(wilFilePath) + "/" + wilFileName + ".wil").c_str(), "rb"); }
    if(!m_FP){ m_FP = fopen((std::string(wilFilePath) + "/" + wilFileName + ".Wil").c_str(), "rb"); }
    if(!m_FP){ m_FP = fopen((std::string(wilFilePath) + "/" + wilFileName + ".WIL").c_str(), "rb"); }

    if(m_FP == nullptr){
        return false;
    }

    WILFILEHEADER tmpWilHdr;
    if(fread(&tmpWilHdr, sizeof(WILFILEHEADER), 1, m_FP) != 1){
        fclose(m_FP); m_FP = nullptr;
        return false;
    }

    m_ImageCount = tmpWilHdr.nImageCount;
    m_Version    = tmpWilHdr.shVer;
    return true;
}

const uint16_t *WilImagePackage::CurrentImageBuffer()
{
    if(m_CurrentImageValid){
        return m_CurrentImageBuffer;
    }else{
        return nullptr;
    }
}

const WILIMAGEINFO &WilImagePackage::CurrentImageInfo()
{
    if(m_CurrentImageValid){
        return m_CurrentWilImageInfo;
    }else{
        return m_DumbWilImageInfo;
    }
}

void WilImagePackage::Decode(uint32_t *rectImageBuffer, uint32_t dwColor1, uint32_t dwColor2)
{
    auto    pwSrc   = m_CurrentImageBuffer;
    int16_t nWidth  = m_CurrentWilImageInfo.shWidth;
    int16_t nHeight = m_CurrentWilImageInfo.shHeight;

    uint16_t srcBeginPos    = 0;
    uint16_t srcEndPos      = 0;
    uint16_t srcNowPos      = 0;
    uint16_t dstNowPosInRow = 0;

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
                    Memcpy16To32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, pwSrc + srcNowPos, cntCopy, 0XFFFFFFFF);
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

    // remove the shadow

    // for(int y = 1; y < nHeight - 2; ++y){
    //     for(int x = 1; x < nWidth - 2; ++x){
    //         auto p0 = *(rectImageBuffer + (y + 0) * nWidth + (x + 0)) & 0XFFFFFF00;
    //         auto p1 = *(rectImageBuffer + (y - 1) * nWidth + (x - 1)) & 0XFFFFFF00;
    //         auto p2 = *(rectImageBuffer + (y - 1) * nWidth + (x + 1)) & 0XFFFFFF00;
    //         auto p3 = *(rectImageBuffer + (y + 1) * nWidth + (x - 1)) & 0XFFFFFF00;
    //         auto p4 = *(rectImageBuffer + (y + 1) * nWidth + (x + 1)) & 0XFFFFFF00;

    //         if(true
    //                 && p0 == p1
    //                 && p0 == p2
    //                 && p0 == p3
    //                 && p0 == p4
    //           ){
    //             for(int i = -1; i <= 2; ++i){
    //                 for(int j = -1; j <= 2; ++j){
    //                     *(rectImageBuffer + (y + i) * nWidth + (x + j)) = 0X80000000;
    //                 }
    //             }
    //         }
    //     }
    // }
}

int32_t WilImagePackage::ImageCount()
{
    return m_FP ? m_ImageCount : 0;
}

int32_t WilImagePackage::IndexCount()
{
    return m_FP ? m_WixImageInfo.nIndexCount: 0;
}

bool WilImagePackage::CurrentImageValid()
{
    return m_CurrentImageValid;
}

int16_t WilImagePackage::Version()
{
    return m_Version;
}

void WilImagePackage::ShadowDecode(uint32_t *rectImageBuffer, bool, uint32_t dwColor1, uint32_t dwColor2)
{
    auto    pwSrc   = m_CurrentImageBuffer;
    int16_t nWidth  = m_CurrentWilImageInfo.shWidth;
    int16_t nHeight = m_CurrentWilImageInfo.shHeight;

    uint16_t srcBeginPos    = 0;
    uint16_t srcEndPos      = 0;
    uint16_t srcNowPos      = 0;
    uint16_t dstNowPosInRow = 0;

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
                    Memcpy16To32(rectImageBuffer + nRow * nWidth + dstNowPosInRow, pwSrc + srcNowPos, cntCopy, 0XFFFFFFFF);
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

    // remove the shadow

    // for(int y = 1; y < nHeight - 2; ++y){
    //     for(int x = 1; x < nWidth - 2; ++x){
    //         auto p0 = *(rectImageBuffer + (y + 0) * nWidth + (x + 0)) & 0XFFFFFF00;
    //         auto p1 = *(rectImageBuffer + (y - 1) * nWidth + (x - 1)) & 0XFFFFFF00;
    //         auto p2 = *(rectImageBuffer + (y - 1) * nWidth + (x + 1)) & 0XFFFFFF00;
    //         auto p3 = *(rectImageBuffer + (y + 1) * nWidth + (x - 1)) & 0XFFFFFF00;
    //         auto p4 = *(rectImageBuffer + (y + 1) * nWidth + (x + 1)) & 0XFFFFFF00;

    //         if(true
    //                 && p0 == p1
    //                 && p0 == p2
    //                 && p0 == p3
    //                 && p0 == p4
    //           ){
    //             for(int i = -1; i <= 2; ++i){
    //                 for(int j = -1; j <= 2; ++j){
    //                     *(rectImageBuffer + (y + i) * nWidth + (x + j)) = 0X80000000;
    //                 }
    //             }
    //         }
    //     }
    // }
}
