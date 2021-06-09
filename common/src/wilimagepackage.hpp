/*
 * =====================================================================================
 *
 *       Filename: wilimagepackage.hpp
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

#pragma once

#include <vector>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "fflerror.hpp"

#pragma pack(push, 1)

struct WILFILEHEADER
{
    int16_t     comp;
    char        title[20];
    int16_t     version;
    int32_t     imageCount;
};

struct WIXIMAGEINFO
{
    char        title[20];
    int32_t     indexCount;
};

struct WILIMAGEINFO
{
    int16_t     width;
    int16_t     height;
    int16_t     px;
    int16_t     py;
    char        shadow;                    
    int16_t     shadowPX;
    int16_t     shadowPY;
    uint32_t    imageLength;
};

#pragma pack(pop)

class WilImagePackage
{
    private:
        WIXIMAGEINFO  m_wixImageInfo;
        WILIMAGEINFO  m_currentWilImageInfo;
        WILFILEHEADER m_wilFileHeader;

    private:
        int32_t m_currentImageIndex;
        bool    m_currentImageValid;

    private:
        std::vector<uint16_t> m_currentImageBuffer;
        std::vector< int32_t> m_wilPosition;

    private:
        FILE *m_wilFile;

    public:
        WilImagePackage();
        ~WilImagePackage();

    public:
        int32_t ImageCount();
        int32_t IndexCount();

    public:
        uint16_t version() const
        {
            return m_wilFileHeader.version;
        }

    public:
        bool SetIndex(uint32_t);

    public:
        bool Load(const char *, const char *, const char *);
        void Decode(uint32_t *, uint32_t, uint32_t, uint32_t);

    public:
        const WILFILEHEADER &HeaderInfo() const;

    public:
        const WILIMAGEINFO  &CurrentImageInfo();
        bool                 CurrentImageValid();
        const uint16_t      *CurrentImageBuffer();

    private:
        static int wixOff(int version)
        {
            switch(version){
                case   17: return 24;
                case 5000: return 28;
                case 6000: return 32;
                default  : throw bad_reach();
            }
        }

        static int wilOff(int version)
        {
            switch(version){
                case   17: return 17;
                case 5000: return 21;
                case 6000: return 21;
                default  : throw bad_reach();
            }
        }
};
