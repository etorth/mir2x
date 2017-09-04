/*
 * =====================================================================================
 *
 *       Filename: wilimagepackage.hpp
 *        Created: 02/14/2016 16:33:12
 *  Last Modified: 09/04/2017 02:19:01
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

#pragma once

#include <vector>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#pragma pack(push, 1)

// .wil file header
struct WILFILEHEADER
{
    int16_t     shComp;
    char        szTitle[20];
    int16_t     shVer;
    int32_t     nImageCount;
};

// .wix file header
struct WIXIMAGEINFO
{
    char        szTitle[20];
    int32_t     nIndexCount;
};

struct WILIMAGEINFO
{
    int16_t     shWidth;
    int16_t     shHeight;
    int16_t     shPX;
    int16_t     shPY;
    char        bShadow;                    
    int16_t     shShadowPX;
    int16_t     shShadowPY;
    uint32_t    dwImageLength;
};

#pragma pack(pop)

class WilImagePackage
{
    private:
        WIXIMAGEINFO    m_WixImageInfo;
        WILIMAGEINFO    m_CurrentWilImageInfo;
        WILFILEHEADER   m_WilFileHeader;

    private:
        int32_t m_CurrentImageIndex;
        bool    m_CurrentImageValid;

    private:
        std::vector<uint16_t> m_CurrentImageBuffer;
        std::vector< int32_t> m_WilPosition;

    private:
        FILE *m_WilFile;

    public:
        WilImagePackage();
       ~WilImagePackage();

    public:
        int32_t ImageCount();
        int32_t IndexCount();

    public:
        int16_t Version();

    public:
        bool Load(const char *, const char *, const char *);
        bool SetIndex(uint32_t);
        void Decode(uint32_t *, uint32_t, uint32_t, uint32_t);

    public:
        const WILFILEHEADER &HeaderInfo() const;

    public:
        const uint16_t      *CurrentImageBuffer();
        const WILIMAGEINFO  &CurrentImageInfo();
        bool                 CurrentImageValid();

    public:
        static int WixOffset(int);
        static int WilOffset(int);

    public:
        bool LocateImage(uint16_t);
};
