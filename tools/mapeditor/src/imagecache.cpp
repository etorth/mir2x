/*
 * =====================================================================================
 *
 *       Filename: imagecache.cpp
 *        Created: 02/14/2016 15:54:58
 *    Description: This class won't handle WilImagePackage directly
 *                 Actually it only deal with all PNG files
 *                 So, don't create Fl_Image and put inside
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

#include "savepng.hpp"
#include "filesys.hpp"
#include "hexstring.hpp"
#include "imagecache.hpp"

ImageCache::ImageCache()
    : m_Path("")
    , m_Cache()
{}

ImageCache::~ImageCache()
{
    for(auto stItor: m_Cache){
        stItor.second->release();
    }
}

void ImageCache::SetPath(const char *szPath)
{
    std::string szTmpPath = szPath;
    while(!szTmpPath.empty() && szTmpPath.back() == '/'){
        szTmpPath.pop_back();
    }

    m_Path = szTmpPath;
    FileSys::MakeDir(szTmpPath.c_str());
    szTmpPath += "/CACHE";
    FileSys::MakeDir(szTmpPath.c_str());
}

Fl_Shared_Image *ImageCache::Retrieve(uint8_t nFileIndex, uint16_t nImageIndex)
{
    // setup cache file folder first
    if(m_Path.empty()){ return nullptr; }

    uint32_t nKey = (((uint32_t)nFileIndex) << 16) + nImageIndex;

    // printf("0X%08X\n", nKey);

    // retrieve in memory
    auto stItor = m_Cache.find(nKey);
    if(stItor != m_Cache.end()){
        return stItor->second;
    }

    // retrieve in file system cache
    char szHexStr[32];
    std::memset(szHexStr, 0, sizeof(szHexStr));
    HexString::ToString<uint32_t, 4>(nKey, szHexStr, true);
    std::string szPNGFullName = m_Path + "/CACHE/" + szHexStr + ".PNG";

    if(FileSys::FileExist(szPNGFullName.c_str())){
        auto pImage = Fl_Shared_Image::get(szPNGFullName.c_str());
        if(pImage){
            m_Cache[nKey] = pImage;
            return pImage;
        }
    }

    return nullptr;
}

bool ImageCache::Register(uint8_t nFileIndex, uint16_t nImageIndex, const uint32_t *pBuff, int nW, int nH)
{
    if(m_Path.empty() || !pBuff || nW <= 0 || nH <= 0){ return false; }

    uint32_t nKey = (((uint32_t)nFileIndex) << 16) + nImageIndex;

    char szHexStr[32];
    std::memset(szHexStr, 0, sizeof(szHexStr));
    HexString::ToString<uint32_t, 4>(nKey, szHexStr, true);
    std::string szPNGFullName = m_Path + "/CACHE/" + szHexStr + ".PNG";
    return SaveRGBABufferToPNG((uint8_t *)pBuff, nW, nH, szPNGFullName.c_str())
        && Retrieve(nFileIndex, nImageIndex) != nullptr;
}
