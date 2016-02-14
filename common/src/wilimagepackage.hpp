#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstdio>

#pragma pack(push, 1)

// .wil file header
typedef struct{
    int16_t     shComp;
    char        szTitle[20];
    int16_t     shVer;
    int32_t     nImageCount;
}WILFILEHEADER;

// .wix file header
typedef struct{
    char        szTitle[20];
    int32_t     nIndexCount;
    int32_t*    pnPosition;
}WIXIMAGEINFO;

typedef struct{
    int16_t     shWidth;
    int16_t     shHeight;
    int16_t     shPX;
    int16_t     shPY;
    char        bShadow;                    
    int16_t     shShadowPX;
    int16_t     shShadowPY;
    uint32_t    dwImageLength;
}WILIMAGEINFO;

#pragma pack(pop)

class WilImagePackage
{
    private:
        WIXIMAGEINFO    m_WixImageInfo;
        WILIMAGEINFO    m_DumbWilImageInfo;
        WILIMAGEINFO    m_CurrentWilImageInfo;

    private:
        uint32_t     m_CurrentImageIndex;
        int32_t      m_ImageCount;
        bool         m_CurrentImageValid;
        uint16_t    *m_CurrentImageBuffer;
        uint32_t     m_CurrentImageBufferLength;
        int16_t      m_Version;
        FILE        *m_FP;

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
        void ShadowDecode(uint32_t *, bool, uint32_t, uint32_t);

    public:
        const uint16_t      *CurrentImageBuffer();
        const WILIMAGEINFO  &CurrentImageInfo();
        bool                 CurrentImageValid();
};
