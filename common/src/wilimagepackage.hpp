#pragma once
#include <array>
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

    private:
        std::vector<uint32_t> m_decodeBuf0;
        std::vector<uint32_t> m_decodeBuf1;
        std::vector<uint32_t> m_decodeBuf2;

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
        // decode image
        // when mergeLayer is true:
        //      layer_0XC1: layer_0XC2 and layer_0XC3 merges to layer_0XC1
        //      layer_0XC2: nullptr
        //      layer_0XC3: nullptr
        //
        // when mergeLayer is false:
        //      layer_0XC1:
        //      layer_0XC2:
        //      layer_0XC3:
        //
        // when there is no pixels, layer_0XC1 returns nullptr
        //
        // when removeShadow is true, alphaf::removeShadowMosaic() applied to layer_0XC1 if it's not nullptr
        // when autoAlpha    is true, alphaf::autoAlpha()          applied to layer_0XC1 if it's not nullptr
        //
        std::array<const uint32_t *, 3> decode(bool /* mergeLayer */, bool /* removeShadow */, bool /* autoAlpha */);

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
                default  : throw fflreach();
            }
        }

        static int wilOff(int version)
        {
            switch(version){
                case   17: return 17;
                case 5000: return 21;
                case 6000: return 21;
                default  : throw fflreach();
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
            throw fflreach();
        }
};
