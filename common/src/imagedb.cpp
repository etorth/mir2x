/*
 * =====================================================================================
 *
 *       Filename: imagedb.cpp
 *        Created: 02/14/2016 16:35:49
 *  Last Modified: 05/22/2017 17:31:34
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

#include <cstring>
#include "imagedb.hpp"

ImageDB::ImageDB()
    : m_BufLen(0)
    , m_Buf(nullptr)
    , m_ImagePackage()
{}

ImageDB::~ImageDB()
{
    delete m_Buf; m_Buf = nullptr;
}

bool ImageDB::LoadDB(const char *szPathName)
{
    const char *szFileName[] = {
        "Tilesc",
        "Tiles30c",
        "Tiles5c",
        "Smtilesc",
        "Housesc",
        "Cliffsc",
        "Dungeonsc",
        "Innersc",
        "Furnituresc",
        "Wallsc",
        "SmObjectsc",
        "Animationsc",
        "Object1c",
        "Object2c",
        "Custom",
        "Wood/Tilesc",
        "Wood/Tiles30c",
        "Wood/Tiles5c",
        "Wood/Smtilesc",
        "Wood/Housesc",
        "Wood/Cliffsc",
        "Wood/Dungeonsc",
        "Wood/Innersc",
        "Wood/Furnituresc",
        "Wood/Wallsc",
        "Wood/SmObjectsc",
        "Wood/Animationsc",
        "Wood/Object1c",
        "Wood/Object2c",
        "Wood/Custom",
        "Sand/Tilesc",
        "Sand/Tiles30c",
        "Sand/Tiles5c",
        "Sand/Smtilesc",
        "Sand/Housesc",
        "Sand/Cliffsc",
        "Sand/Dungeonsc",
        "Sand/Innersc",
        "Sand/Furnituresc",
        "Sand/Wallsc",
        "Sand/SmObjectsc",
        "Sand/Animationsc",
        "Sand/Object1c",
        "Sand/Object2c",
        "Sand/Custom",
        "Snow/Tilesc",
        "Snow/Tiles30c",
        "Snow/Tiles5c",
        "Snow/Smtilesc",
        "Snow/Housesc",
        "Snow/Cliffsc",
        "Snow/Dungeonsc",
        "Snow/Innersc",
        "Snow/Furnituresc",
        "Snow/Wallsc",
        "Snow/SmObjectsc",
        "Snow/Animationsc",
        "Snow/Object1c",
        "Snow/Object2c",
        "Snow/Custom",
        "Forest/Tilesc",
        "Forest/Tiles30c",
        "Forest/Tiles5c",
        "Forest/Smtilesc",
        "Forest/Housesc",
        "Forest/Cliffsc",
        "Forest/Dungeonsc",
        "Forest/Innersc",
        "Forest/Furnituresc",
        "Forest/Wallsc",
        "Forest/SmObjectsc",
        "Forest/Animationsc",
        "Forest/Object1c",
        "Forest/Object2c",
        "Forest/Custom",
        ""
    };

    for(int i = 0; std::strlen(szFileName[i]) > 0; ++i){
        Load((uint8_t)i, szPathName, szFileName[i], ".wil");
    }

    return true;
}

bool ImageDB::Load(uint8_t nFileIndex, const char *szPathName, const char *szFileName, const char *szNamePrefix)
{
    return m_ImagePackage[nFileIndex].Load(szPathName, szFileName, szNamePrefix);
}

bool ImageDB::Valid(uint8_t nFileIndex, uint16_t nImageIndex)
{
    if(nFileIndex == 255 || nImageIndex == 65535){ return false; }

    if(m_ImagePackage[nFileIndex].SetIndex(nImageIndex) &&
            m_ImagePackage[nFileIndex].CurrentImageValid()){
        int nW = m_ImagePackage[nFileIndex].CurrentImageInfo().shWidth;
        int nH = m_ImagePackage[nFileIndex].CurrentImageInfo().shHeight;
        if(nW * nH > 0){
            return true;
        }
    }
    return false;
}

int ImageDB::FastW(uint8_t nFileIndex)
{
    return m_ImagePackage[nFileIndex].CurrentImageInfo().shWidth;
}

int ImageDB::W(uint8_t nFileIndex, uint16_t nImageIndex)
{
    return Valid(nFileIndex, nImageIndex) ? FastW(nFileIndex) : 0;
}

int ImageDB::FastH(uint8_t nFileIndex)
{
    return m_ImagePackage[nFileIndex].CurrentImageInfo().shHeight;
}

int ImageDB::H(uint8_t nFileIndex, uint16_t nImageIndex)
{
    return Valid(nFileIndex, nImageIndex) ? FastH(nFileIndex) : 0;
}

const uint32_t *ImageDB::FastDecode(uint8_t nFileIndex, uint32_t nC0, uint32_t nC1, uint32_t nC2)
{
    int nW = FastW(nFileIndex);
    int nH = FastH(nFileIndex);

    ExtendBuf(nW * nH);
    m_ImagePackage[nFileIndex].Decode(m_Buf, nC0, nC1, nC2);

    return m_Buf;
}

const uint32_t *ImageDB::Decode(uint8_t nFileIndex, uint16_t nImageIndex, uint32_t nC0, uint32_t nC1, uint32_t nC2)
{
    if(Valid(nFileIndex, nImageIndex)){
        return FastDecode(nFileIndex, nC0, nC1, nC2);
    }
    return nullptr;
}

void ImageDB::ExtendBuf(int nBufLen)
{
    if(m_BufLen < nBufLen){
        delete m_Buf;
        m_Buf = new uint32_t[nBufLen];
        m_BufLen = nBufLen;
    }
}
