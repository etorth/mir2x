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
#include "filesys.hpp"
#include "fileptr.hpp"
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
        fileptr_t m_wilFile;

    private:
        std::optional<uint32_t> m_currImageIndex;

    private:
        WILFILEHEADER m_wilFileHeader;

    private:
        WIXIMAGEINFO m_wixImageInfo;
        WILIMAGEINFO m_currImageInfo;

    private:
        std::vector< int32_t> m_wilPositionList;
        std::vector<uint16_t> m_currImageBuffer;

    public:
        WilImagePackage(const char *, const char *);

    public:
        size_t imageCount() const
        {
            return m_wilFileHeader.imageCount;
        }

        size_t indexCount() const
        {
            return m_wixImageInfo.indexCount;
        }

    public:
        uint16_t version() const
        {
            return m_wilFileHeader.version;
        }

    public:
        const WILIMAGEINFO *setIndex(uint32_t);

    public:
        void decode(uint32_t *, uint32_t, uint32_t, uint32_t);

    public:
        const WILFILEHEADER &header() const
        {
            return m_wilFileHeader;
        }

    public:
        bool currImageValid() const
        {
            return m_currImageIndex.has_value();
        }

    public:
        const WILIMAGEINFO *currImageInfo() const
        {
            return m_currImageIndex.has_value() ? &m_currImageInfo : nullptr;
        }

    public:
        const uint16_t *currImageBuffer() const
        {
            return m_currImageIndex.has_value() ? m_currImageBuffer.data() : nullptr;
        }

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

    private:
        static std::string hasWilFile(const char *path, const char *file, std::initializer_list<const char *> extList)
        {
            fflassert(str_haschar(path));
            fflassert(str_haschar(file));

            for(const auto ext: extList){
                fflassert(str_haschar(ext));
                if(const auto fileName = str_printf("%s/%s.%s", path, file, ext); filesys::hasFile(fileName.c_str())){
                    return fileName;
                }
            }
            throw bad_reach();
        }
};
